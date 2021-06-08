#include ":/lib/gen/batch.hpp"
#include ":/util/ansi.hpp"

#include <thread>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <limits>

namespace solvent::lib::gen::batch {

	template<Order O>
	const unsigned ThreadFunc<O>::DEFAULT_NUM_THREADS = [](){
		const unsigned hwc = std::thread::hardware_concurrency();
		// NOTE: hardware_concurency is specified to be zero if unknown.
		return (hwc != 0) ? std::min(TRY_DEFAULT_NUM_EXTRA_THREADS_ + 1, hwc) : 1;
	}();


	template<Order O>
	Params Params::clean(void) noexcept {
		gen_params.template clean<O>();
		if (num_threads == 0) {
			num_threads = ThreadFunc<O>::DEFAULT_NUM_THREADS;
		}
		if (max_backtrack_sample_granularity == 0) {
			max_backtrack_sample_granularity = SharedData::SAMPLE_GRANULARITY_DEFAULT;
		}
		return *this;
	}


	template<Order O>
	void ThreadFunc<O>::operator()() {
		shared_data_mutex_.lock();
		while (get_progress() < params_.stop_after) [[likely]] {
			shared_data_mutex_.unlock(); //____________________
			const typename Generator<O>::GenResult gen_result
				= generator_.generate(params_.gen_params);
			shared_data_mutex_.lock(); //‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾

			shared_data_.total_anys++;
			if (gen_result.status == ExitStatus::Ok) [[likely]] {
				shared_data_.total_oks++;

				auto& dist_summary_row = shared_data_.max_backtrack_samples[
					params_.max_backtrack_sample_granularity
					* (gen_result.most_backtracks_seen - 1)
					/ gen_result.params.max_backtracks
				];
				dist_summary_row.marginal_oks++;
				dist_summary_row.marginal_ops += gen_result.op_count;
			}
			gen_result_consumer_(gen_result);
		}
		shared_data_mutex_.unlock();
	}


	template<Order O>
	const BatchReport batch(Params& params, callback_t<O> gen_result_consumer) {
		params.clean<O>();
		std::mutex shared_data_mutex;
		SharedData shared_data;
		shared_data.max_backtrack_samples.resize(params.max_backtrack_sample_granularity);

		std::vector<std::thread> threads;
		for (unsigned i = 0; i < params.num_threads; i++) {
			threads.push_back(std::thread(ThreadFunc<O>(
				params, shared_data, shared_data_mutex, gen_result_consumer)
			));
		}
		for (auto& thread : threads) {
			thread.join();
		}
		shared_data.time_elapsed = shared_data.timer.read_elapsed();
		{
			double net_ops = 0.0;
			trials_t net_oks = 0;
			for (unsigned i = 0; i < shared_data.max_backtrack_samples.size(); i++) {
				auto& sample = shared_data.max_backtrack_samples[i];
				sample.max_backtracks = params.gen_params.max_backtracks * (i+1) / params.max_backtrack_sample_granularity;
				net_ops += sample.marginal_ops;
				net_oks += sample.marginal_oks;
				sample.marginal_average_ops = sample.marginal_oks ? std::optional(sample.marginal_ops / sample.marginal_oks) : std::nullopt;
				sample.net_average_ops = net_oks ? std::optional(static_cast<double>(net_ops) / net_oks) : std::nullopt;
			}
		}{
			// Get the index of the sample representing the optimal max_backtracks setting:
			shared_data.max_backtrack_samples_best_i = 0;
			double best_net_average_ops = std::numeric_limits<double>::max();
			for (unsigned i = 0; i < shared_data.max_backtrack_samples.size(); i++) {
				const auto& sample = shared_data.max_backtrack_samples[i];
				if (sample.net_average_ops.has_value() && (sample.net_average_ops.value() < best_net_average_ops)) {
					best_net_average_ops = sample.net_average_ops.value();
					shared_data.max_backtrack_samples_best_i = i;
				}
			}
		}
		return shared_data;
	}


	void SharedData::print(std::ostream& os, const Order O) const {
		static const std::string THROUGHPUT_BAR_STRING("--------------------------------");
		static const std::string TABLE_SEPARATOR = "\n+------------------+------------------+--------------------------+-------------------+";
		static const std::string TABLE_HEADER    = "\n|  max backtracks  |   marginal oks   |   marginal average ops   |  net average ops  |";

		std::cout << TABLE_SEPARATOR;
		std::cout << TABLE_HEADER;
		std::cout << TABLE_SEPARATOR;
		const auto& best_sample = max_backtrack_samples[max_backtrack_samples_best_i];
		for (const auto& sample : max_backtrack_samples) {

			// max_backtracks:
			if (O <= 4) {
				std::cout << "\n|" << std::setw(9) << sample.max_backtracks;
			} else {
				std::cout << "\n|" << std::setw(8) << (sample.max_backtracks / 1'000.0) << 'K';
			}

			// marginal_oks:
			std::cout << "  |";
			if (sample.marginal_oks == 0) { std::cout << util::ansi::DIM.ON; }
			std::cout << std::setw(8) << sample.marginal_oks;
			if (sample.marginal_oks == 0) { std::cout << util::ansi::DIM.OFF; }

			// marginal_average_ops:
			std::cout << "  |";
			if (sample.marginal_oks == 0) { std::cout << util::ansi::DIM.ON; }
			std::cout << std::setw(13);
			if (sample.marginal_average_ops.has_value()) {
				std::cout << (sample.marginal_average_ops.value() / ((O < 5) ? 1 : 1000));
			} else {
				std::cout << "-";
			}
			std::cout << ((O < 5) ? ' ' : 'K');
			if (sample.marginal_oks == 0) { std::cout << util::ansi::DIM.OFF; }

			// net_average_ops:
			std::cout << "  |";
			std::cout << std::setw(9);
			if (sample.net_average_ops.has_value()) {
				std::cout << (100.0 * sample.net_average_ops.value());
			} else {
				std::cout << "-";
			}

			std::cout << "  |";

			// Print a bar to visualize throughput relative to that
			// of the best. Note visual exaggeration via exponents
			// (the exponent value was chosen by taste / visual feel)
			const unsigned bar_length = (best_sample.net_average_ops.has_value()) ? (THROUGHPUT_BAR_STRING.length() * std::pow(
				1.0 / (sample.net_average_ops.value_or(0) / best_sample.net_average_ops.value()),
				static_cast<int>(20.0 / O)
			)) : 0;
			if (&sample != &best_sample) std::cout << util::ansi::DIM.ON;
			std::cout << ' ' << THROUGHPUT_BAR_STRING.substr(0, bar_length);
			if (&sample != &best_sample) std::cout << util::ansi::DIM.OFF;
		}
		std::cout << TABLE_SEPARATOR;
		if (total_oks < max_backtrack_samples.size() * gen::batch::SharedData::RECOMMENDED_OKS_PER_SAMPLE) {
			std::cout << util::ansi::DIM.ON << "\nexercise caution against small datasets!" << util::ansi::DIM.OFF << std::endl;
		}
	}


	#define SOLVENT_TEMPL_TEMPL(O_) \
		template Params Params::clean<O_>(void) noexcept; \
		template class ThreadFunc<O_>; \
		template const BatchReport batch<O_>(Params&, callback_t<O_>);
	SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef SOLVENT_TEMPL_TEMPL
}
namespace std {
	#define SOLVENT_TEMPL_TEMPL(O_) \
		template class function<void(typename solvent::lib::gen::Generator<O_>::GenResult const&)>;
	SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef SOLVENT_TEMPL_TEMPL
}
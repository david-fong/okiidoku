#include <solvent_lib/gen/batch.hpp>

#include <thread>
#include <mutex>
#include <algorithm>
#include <limits>
#include <cassert>

namespace solvent::lib::gen::batch {

	unsigned DEFAULT_NUM_THREADS(const Order O) {
		const unsigned hwc = std::thread::hardware_concurrency();
		if (hwc == 0) {
			// Note: hardware_concurency is specified to be zero if unknown.
			return 1;
		}
		else return std::min(
			TRY_DEFAULT_NUM_EXTRA_THREADS_(O) + 1,
			hwc == 1 ? 1 : hwc - 1 // leave at least one spare thread
		);
	}


	Params Params::clean(const Order O) noexcept {
		gen_params.clean(O);
		if (num_threads == 0) {
			num_threads = DEFAULT_NUM_THREADS(O);
		}
		if (max_dead_end_sample_granularity == 0) {
			max_dead_end_sample_granularity = BatchReport::SAMPLE_GRANULARITY_DEFAULT;
		}
		return *this;
	}


	//
	template<Order O>
	struct ThreadFunc final {
	 static_assert(O > 0);
		void operator()();

		trials_t get_progress(void) const noexcept {
			if (params_.only_count_oks) {
				return shared_data_.total_oks;
			} else {
				return shared_data_.total_anys;
			}
		}

		const Params params_;
		BatchReport& shared_data_;
		std::mutex& shared_data_mutex_;
		callback_t<O> callback_;
		Generator<O> generator_ {};
	};


	template<Order O>
	void ThreadFunc<O>::operator()() {
		shared_data_mutex_.lock();
		while (get_progress() < params_.stop_after) [[likely]] {
			shared_data_mutex_.unlock(); //____________________
			const typename Generator<O>::ResultView result = generator_(params_.gen_params);
			shared_data_mutex_.lock(); //‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾

			shared_data_.total_anys++;
			if (result.exit_status() == ExitStatus::Ok) [[likely]] {
				shared_data_.total_oks++;

				auto& dist_summary_row = shared_data_.max_dead_end_samples[
					params_.max_dead_end_sample_granularity
					* static_cast<unsigned long>(result.most_dead_ends_seen())
					/ (params_.gen_params.max_dead_ends + 1)
				];
				dist_summary_row.marginal_oks++;
				dist_summary_row.marginal_ops += static_cast<double>(result.op_count());
			}
			callback_(result);
		}
		shared_data_mutex_.unlock();
	}


	BatchReport batch_(const Order O, Params& params, std::function<std::thread(BatchReport&, std::mutex&)> mk_thread) {
		params.clean(O);
		std::mutex shared_data_mutex;
		BatchReport shared_data;
		shared_data.max_dead_end_samples.resize(params.max_dead_end_sample_granularity);

		std::vector<std::thread> threads;
		for (unsigned i = 0; i < params.num_threads; i++) {
			threads.push_back(mk_thread(shared_data, shared_data_mutex));
		}
		for (auto& thread : threads) {
			thread.join();
		}
		shared_data.time_elapsed = shared_data.timer.read_elapsed();

		shared_data.fraction_aborted = (shared_data.total_anys == 0) ? 1.0 :
			(static_cast<double>(shared_data.total_anys - shared_data.total_oks)
			/ static_cast<double>(shared_data.total_anys));
		{
			double net_ops = 0.0;
			trials_t net_oks = 0;
			for (unsigned i = 0; i < params.max_dead_end_sample_granularity; i++) {
				auto& sample = shared_data.max_dead_end_samples[i];
				sample.max_dead_ends = params.gen_params.max_dead_ends * (i+1) / params.max_dead_end_sample_granularity;
				net_ops += sample.marginal_ops;
				net_oks += sample.marginal_oks;
				sample.marginal_average_ops = sample.marginal_oks
					? std::optional(sample.marginal_ops / static_cast<double>(sample.marginal_oks))
					: std::nullopt;
				sample.net_average_ops = net_oks
					? std::optional(static_cast<double>(net_ops) / static_cast<double>(net_oks))
					: std::nullopt;
			}
		}{
			// Get the index of the sample representing the optimal max_dead_ends setting:
			shared_data.max_dead_end_samples_best_i = 0;
			double best_net_average_ops = std::numeric_limits<double>::max();
			for (unsigned i = 0; i < shared_data.max_dead_end_samples.size(); i++) {
				const auto& sample = shared_data.max_dead_end_samples[i];
				if (sample.net_average_ops.has_value() && (sample.net_average_ops.value() < best_net_average_ops)) {
					best_net_average_ops = sample.net_average_ops.value();
					shared_data.max_dead_end_samples_best_i = i;
				}
			}
		}
		return shared_data;
	}


	template<Order O>
	BatchReport batch(Params& params, callback_t<O> callback) {
		return batch_(O, params, [&](auto& sd, auto& sd_mutex) {
			return std::thread(ThreadFunc<O>{
				params, sd, sd_mutex, callback
			});
		});
	}


	BatchReport batch_O(Order O, Params& params, callback_o_t callback) {
		assert(is_order_compiled(O));
		return batch_(O, params, [&](auto& sd, auto& sd_mutex) {
			switch (O) {
			#define M_SOLVENT_TEMPL_TEMPL(O_) \
				case O_: { return std::thread(ThreadFunc<O_>{ \
					params, sd, sd_mutex, [&](const auto result){ \
						return callback(result.to_generic()); \
					} \
				}); \
				break; }
			M_SOLVENT_INSTANTIATE_ORDER_TEMPLATES
			#undef M_SOLVENT_TEMPL_TEMPL
			default: return std::thread{}; // never reaches here
			}
		});
	}


	#define M_SOLVENT_TEMPL_TEMPL(O_) \
		template BatchReport batch<O_>(Params&, std::function<void(typename Generator<O_>::ResultView)>);
	M_SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef M_SOLVENT_TEMPL_TEMPL
}
namespace std {
	template class function<void(const solvent::lib::gen::ResultView&)>;
}
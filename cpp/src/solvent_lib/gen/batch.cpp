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


	struct ThreadSharedData {
		const Params params;
		BatchReport report {};
	};


	//
	template<Order O>
	struct ThreadFunc final {
		static_assert(O > 0);
		void operator()();

		trials_t get_progress(void) const noexcept {
			if (sd_.params.only_count_oks) {
				return sd_.report.total_oks;
			} else {
				return sd_.report.total_anys;
			}
		}

		ThreadSharedData& sd_;
		std::mutex& sd_mutex_;
		callback_O_t<O> callback_;
		GeneratorO<O> generator_ {};
	};


	template<Order O>
	void ThreadFunc<O>::operator()() {
		sd_mutex_.lock();
		while (get_progress() < sd_.params.stop_after) [[likely]] {
			sd_mutex_.unlock(); //___
				generator_(sd_.params.gen_params);
			sd_mutex_.lock(); //‾‾‾‾

			sd_.report.total_anys++;
			if (generator_.status() == ExitStatus::Ok) [[likely]] {
				sd_.report.total_oks++;

				auto& dist_summary_row = sd_.report.max_dead_end_samples[
					sd_.params.max_dead_end_sample_granularity
					* static_cast<unsigned long>(generator_.get_most_dead_ends_seen())
					/ (sd_.params.gen_params.max_dead_ends + 1)
				];
				dist_summary_row.marginal_oks++;
				dist_summary_row.marginal_ops += static_cast<double>(generator_.get_op_count());
			}
			callback_(generator_);
		}
		sd_mutex_.unlock();
	}


	BatchReport batch_(const Order O, Params& params, std::function<std::thread(ThreadSharedData&, std::mutex&)> mk_thread) {
		params.clean(O);
		std::mutex sd_mutex;
		ThreadSharedData sd {.params {params}};
		sd.report.max_dead_end_samples.resize(params.max_dead_end_sample_granularity);

		std::vector<std::thread> threads;
		for (unsigned i = 0; i < params.num_threads; i++) {
			threads.push_back(mk_thread(sd, sd_mutex));
		}
		for (auto& thread : threads) {
			thread.join();
		}
		sd.report.time_elapsed = sd.report.timer.read_elapsed();

		sd.report.fraction_aborted = (sd.report.total_anys == 0) ? 1.0 :
			(static_cast<double>(sd.report.total_anys - sd.report.total_oks)
			/ static_cast<double>(sd.report.total_anys));
		{
			double net_ops = 0.0;
			trials_t net_oks = 0;
			for (unsigned i = 0; i < params.max_dead_end_sample_granularity; i++) {
				auto& sample = sd.report.max_dead_end_samples[i];
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
			sd.report.max_dead_end_samples_best_i = 0;
			double best_net_average_ops = std::numeric_limits<double>::max();
			for (unsigned i = 0; i < sd.report.max_dead_end_samples.size(); i++) {
				const auto& sample = sd.report.max_dead_end_samples[i];
				if (sample.net_average_ops.has_value() && (sample.net_average_ops.value() < best_net_average_ops)) {
					best_net_average_ops = sample.net_average_ops.value();
					sd.report.max_dead_end_samples_best_i = i;
				}
			}
		}
		return sd.report;
	}


	template<Order O>
	BatchReport batch_O(Params& params, callback_O_t<O> callback) {
		return batch_(O, params, [&](auto& sd, auto& sd_mutex) {
			return std::thread(ThreadFunc<O>{
				sd, sd_mutex, callback
			});
		});
	}


	BatchReport batch(Order O, Params& params, callback_t callback) {
		assert(is_order_compiled(O));
		return batch_(O, params, [&](auto& sd, auto& sd_mutex) {
			switch (O) {
			#define M_SOLVENT_TEMPL_TEMPL(O_) \
				case O_: { return std::thread(ThreadFunc<O_>{ \
					sd, sd_mutex, [&](const auto result){ \
						return callback(static_cast<const Generator&>(result)); \
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
		template BatchReport batch_O<O_>(Params&, callback_O_t<O_>);
	M_SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef M_SOLVENT_TEMPL_TEMPL
}
#ifndef HPP_SOLVENT_LIB_BATCH
#define HPP_SOLVENT_LIB_BATCH

#include <solvent_lib/gen/mod.hpp>
#include <solvent_util/timer.hpp>

#include <iosfwd>
#include <vector>
#include <string>
#include <mutex>
#include <functional>
#include <optional>

namespace solvent::lib::gen::batch {

	using trials_t = unsigned long;

	//
	struct Params {
		gen::Params gen_params;
		unsigned num_threads = 0; // Defaulted if zero.
		unsigned max_dead_end_sample_granularity = 0; // Defaulted if zero.
		bool only_count_oks;
		trials_t stop_after;

		// Cleans self and returns a copy of self.
		Params clean(Order O) noexcept;
	};

	//
	struct SharedData {
		trials_t total_anys = 0;
		trials_t total_oks = 0;
		double fraction_aborted;

		util::Timer timer;
		util::Timer::Elapsed time_elapsed;

		struct MaxDeadEndSample {
			unsigned long max_dead_ends;
			trials_t marginal_oks;
			double marginal_ops;
			std::optional<double> marginal_average_ops; // marginal_ops / marginal_oks. null if no oks.
			std::optional<double> net_average_ops; // (accumulated marginal_ops) / (accumulated marginal_oks). null if no oks.
		};
		static constexpr unsigned SAMPLE_GRANULARITY_DEFAULT = 20u;
		static constexpr unsigned SAMPLE_GRANULARITY_MAX = 50u;
		static constexpr unsigned RECOMMENDED_OKS_PER_SAMPLE = 20u;

		// Data sampled. Each entry showing the outcome if its max_dead_ends
		// value was used.
		std::vector<MaxDeadEndSample> max_dead_end_samples;
		unsigned max_dead_end_samples_best_i = 0u;

		void print(std::ostream&, Order O) const;
	};

	//
	using callback_t = std::function<void(GenResult const&)>;

	constexpr unsigned TRY_DEFAULT_NUM_EXTRA_THREADS_(const Order O) {
		if (O < 4) { return 0; }
		else if (O == 4) { return 1; }
		else { return 2; }
	};
	unsigned DEFAULT_NUM_THREADS(Order O);

	//
	template<Order O>
	class ThreadFunc final {
	 static_assert(O > 0);
	 public:
		void operator()();

		trials_t get_progress(void) const noexcept {
			if (params_.only_count_oks) {
				return shared_data_.total_oks;
			} else {
				return shared_data_.total_anys;
			}
		}

		const Params params_;
		SharedData& shared_data_;
		std::mutex& shared_data_mutex_;
		callback_t gen_result_consumer_;
		Generator<O> generator_ = Generator<O>();
	};


	//
	using BatchReport = SharedData;

	//
	BatchReport batch(Order, Params&, callback_t);


	#define SOLVENT_TEMPL_TEMPL(O_) \
		extern template class ThreadFunc<O_>;
	SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef SOLVENT_TEMPL_TEMPL
}

namespace std {
	extern template class function<void(solvent::lib::gen::GenResult const&)>;
}
#endif
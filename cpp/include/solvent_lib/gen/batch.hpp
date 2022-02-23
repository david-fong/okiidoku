#ifndef HPP_SOLVENT_LIB__GEN__BATCH
#define HPP_SOLVENT_LIB__GEN__BATCH

#include <solvent_lib/gen/mod.hpp>
#include <solvent_util/timer.hpp>

#include <iosfwd>
#include <functional>
#include <vector>
#include <optional>

namespace solvent::lib::gen::batch {

	using trials_t = unsigned long;

	//
	struct Params final {
		gen::Params gen_params;
		unsigned num_threads = 0; // Defaulted if zero.
		unsigned max_dead_end_sample_granularity = 0; // Defaulted if zero.
		bool only_count_oks;
		trials_t stop_after;
		unsigned callback_buffering = 0; // Defaulted if zero.

		// Cleans self and returns a copy of self.
		Params clean(Order O) noexcept;

		static constexpr unsigned DEFAULT_BUFFERING[]{ 0, 1,
			/*2*/1000,
			/*3*/1000,
			/*4*/1000,
			/*5*/1,
			1, 1, 1, 1, 1
		};
	};

	//
	struct BatchReport final {
		trials_t total_anys = 0;
		trials_t total_oks = 0;
		double fraction_aborted;

		util::Timer timer;
		util::Timer::Elapsed time_elapsed;

		struct MaxDeadEndSample final {
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

	constexpr unsigned TRY_DEFAULT_NUM_EXTRA_THREADS_(const Order O) {
		if (O < 4) { return 0; }
		else if (O == 4) { return 1; }
		else { return 2; }
	};
	unsigned DEFAULT_NUM_THREADS(Order O);

	//
	using callback_t = std::function<void (const GenResult&)>;

	// callback is guarded by mutex.
	[[nodiscard]] BatchReport batch(Order, Params&, callback_t);
}

namespace std {
	extern template class function<void (const solvent::lib::gen::GenResult&)>;
}
#endif
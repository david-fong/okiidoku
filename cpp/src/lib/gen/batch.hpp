#ifndef HPP_SOLVENT_LIB_BATCH
#define HPP_SOLVENT_LIB_BATCH

#include "./mod.hpp"
#include ":/util/timer.hpp"

#include <iosfwd>
#include <vector>
#include <array>
#include <string>
#include <mutex>
#include <functional>
#include <optional>

namespace solvent::lib::gen::batch {

	using trials_t = unsigned long;

	//
	struct Params {
		gen::Params gen_params;
		unsigned num_threads = 0; // If zero, a default value will be used.
		unsigned max_backtrack_sample_granularity = 0;
		bool only_count_oks;
		trials_t stop_after;

		template<Order O> Params clean(void) noexcept;
	};

	//
	struct SharedData {
		trials_t total_anys = 0;
		trials_t total_oks = 0;

		util::Timer timer;
		util::Timer::Elapsed time_elapsed;

		struct MaxBacktrackSample {
			unsigned long max_backtracks;
			trials_t marginal_oks;
			double marginal_ops;
			std::optional<double> marginal_average_ops; // marginal_ops / marginal_oks. null if no oks.
			std::optional<double> net_average_ops; // (accumulated marginal_ops) / (accumulated marginal_oks). null if no oks.
		};
		static constexpr unsigned SAMPLE_GRANULARITY_DEFAULT = 20u;
		static constexpr unsigned SAMPLE_GRANULARITY_MAX = 50u;
		static constexpr unsigned RECOMMENDED_OKS_PER_SAMPLE = 20u;

		// Data sampled. Each entry showing the outcome if its max_backtracks
		// value was used.
		std::vector<MaxBacktrackSample> max_backtrack_samples;
		unsigned max_backtrack_samples_best_i = 0u;

		void print(std::ostream&, Order O) const;
	};

	//
	template<Order O>
	using callback_t = std::function<void(typename Generator<O>::GenResult const&)>;

	//
	template<Order O>
	class ThreadFunc final {
	 static_assert(O > 0);
	 public:
		static constexpr unsigned TRY_DEFAULT_NUM_EXTRA_THREADS_ = [](){
			if (O < 4) { return 0; }
			else if (O == 4) { return 1; }
			else { return 2; }
		}();
		static const unsigned DEFAULT_NUM_THREADS;

	 public:
		ThreadFunc(void) = delete;
		explicit ThreadFunc(
			const Params p, SharedData& sd, std::mutex& sdm, callback_t<O> grc
		):
			params_(p), shared_data_(sd), shared_data_mutex_(sdm), gen_result_consumer_(grc)
		{};

		void operator()();

		trials_t get_progress(void) const noexcept {
			if (params_.only_count_oks) {
				return shared_data_.total_oks;
			} else {
				return shared_data_.total_anys;
			}
		}

	 private:
		const Params params_;
		SharedData& shared_data_;
		std::mutex& shared_data_mutex_;
		callback_t<O> gen_result_consumer_;
		Generator<O> generator_;
	};


	//
	using BatchReport = SharedData;

	//
	template<Order O>
	const BatchReport batch(Params&, callback_t<O>);


	#define SOLVENT_TEMPL_TEMPL(O_) \
	extern template Params Params::clean<O_>(void) noexcept;
	SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef SOLVENT_TEMPL_TEMPL

	#define SOLVENT_TEMPL_TEMPL(O_) \
	extern template class ThreadFunc<O_>;
	SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef SOLVENT_TEMPL_TEMPL

	#define SOLVENT_TEMPL_TEMPL(O_) \
	extern template const BatchReport batch<6>(Params&, callback_t<6>);
	SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef SOLVENT_TEMPL_TEMPL
}

namespace std {
	#define SOLVENT_TEMPL_TEMPL(O_) \
	extern template class function<void(typename solvent::lib::gen::Generator<O_>::GenResult const&)>;
	SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef SOLVENT_TEMPL_TEMPL
}
#endif
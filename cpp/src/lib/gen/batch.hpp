#ifndef HPP_SOLVENT_LIB_BATCH
#define HPP_SOLVENT_LIB_BATCH

#include "./mod.hpp"

#include <mutex>
#include <string>
#include <array>

namespace solvent::lib::gen::batch {

	using trials_t = unsigned long;

	constexpr unsigned NUM_BINS = 20u;

	struct Params {
		const bool only_count_oks;
		const trials_t stop_after;
	};

	struct SharedData {
		trials_t total_anys;
		trials_t total_oks;

		struct DistSummaryRow {
			trials_t hit_count;
			double total_ops;
		};
		std::array<DistSummaryRow, NUM_BINS> dist_summary;
	};

	//
	template<Order O>
	class ThreadFunc final {
	 static_assert(O > 0);
	 public:
		static constexpr unsigned NUM_EXTRA_THREADS = [](){
			if (O < 4) { return 0; }
			else if (O == 4) { return 1; }
			else { return 2; }
		}();
		static const unsigned NUM_THREADS;

	 public:
		ThreadFunc(void) = delete;
		explicit ThreadFunc(
			Params p, SharedData& sd, std::mutex& sdm,
			void(& grc)(const typename Generator<O>::GenResult)
		) : params_(p), shared_data_mutex_(sdm), shared_data_(sd), gen_result_consumer_(grc) {};

		void operator()();

		trials_t get_progress(void) const {
			if (params_.only_count_oks) {
				return shared_data_.total_oks;
			} else {
				return shared_data_.total_anys;
			}
		}

	 private:
		const Params params_;
		std::mutex& shared_data_mutex_;
		SharedData& shared_data_;
		void (& gen_result_consumer_)(const Generator<O>::GenResult);
		Generator<O> generator_;
	};

	//
	struct BatchReport : public SharedData {
		BatchReport() = delete;
		explicit BatchReport(SharedData sd) : SharedData(sd) {}

		double processor_time_elapsed;
		double wall_clock_time_elapsed;
	};

	//
	template<Order O>
	SharedData batch(Params, void(&)(const typename Generator<O>::GenResult));
}
#endif
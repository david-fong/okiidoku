#include "./batch.hpp"

#include <thread>
#include <algorithm>

namespace solvent::lib::gen::batch {

	template<Order O>
	const unsigned ThreadFunc<O>::DEFAULT_NUM_THREADS = [](){
		const unsigned hwc = std::thread::hardware_concurrency();
		// NOTE: hardware_concurency is specified to be zero if unknown.
		return (hwc != 0) ? std::min(NUM_EXTRA_THREADS + 1, hwc) : 1;
	}();

	template<Order O>
	void ThreadFunc<O>::operator()() {
		shared_data_mutex_.lock();
		while (true) {
			if (get_progress() >= params_.stop_after) {
				break;
			}
			shared_data_mutex_.unlock(); //____________________
			const typename Generator<O>::GenResult gen_result
				= generator_.generate(params_.gen_params);
			shared_data_mutex_.lock(); //‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾

			shared_data_.total_anys++;
			if (gen_result.exit_status == ExitStatus::Ok) {
				shared_data_.total_oks++;
			}
			auto& dist_summary_row = shared_data_.max_backtrack_samples[
				params_.max_backtrack_sample_granularity
				* gen_result.most_backtracks_seen
				/ generator_.max_backtracks_
			];
			dist_summary_row.marginal_oks++;
			dist_summary_row.marginal_ops += gen_result.op_count;

			gen_result_consumer_(gen_result);
		}
		shared_data_mutex_.unlock();
	}

	template<Order O>
	const BatchReport batch(Params& params, callback_t<O> gen_result_consumer) {
		if (params.num_threads == 0) {
			params.num_threads = ThreadFunc<O>::DEFAULT_NUM_THREADS;
		}
		const util::Timer timer;
		std::mutex shared_data_mutex;
		SharedData shared_data;

		std::vector<std::thread> threads(params.num_threads,
			ThreadFunc<O>(params, shared_data, shared_data_mutex, gen_result_consumer)
		);
		for (auto& thread : threads) {
			thread.join();
		}
		const auto time_elapsed = timer.read_elapsed();
		{
			double net_ops = 0.0;
			trials_t net_oks = 0;
			for (auto& sample : shared_data.max_backtrack_samples) {
				sample.max_backtracks = params.gen_params.max_backtracks;
				net_ops += sample.marginal_ops;
				net_oks += sample.marginal_oks;
				sample.marginal_average_ops = sample.marginal_ops / sample.marginal_oks;
				sample.net_average_ops = static_cast<double>(net_ops) / net_oks;
			}
		}
		return BatchReport(shared_data, time_elapsed);
	}
}
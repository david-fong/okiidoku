#include "./batch.hpp"

#include <thread>
#include <algorithm>
#include <vector>

namespace solvent::lib::gen::batch {

	template<Order O>
	const unsigned ThreadFunc<O>::NUM_THREADS = [](){
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
			shared_data_mutex_.unlock(); //_______________
			const auto gen_result = generator_.generate();
			shared_data_mutex_.lock(); //‾‾‾‾‾‾‾‾‾‾‾‾‾‾

			shared_data_.total_anys++;
			if (gen_result.exit_status == ExitStatus::Ok) {
				shared_data_.total_oks++;
			}
			// Save some stats for later diagnostics-printing:
			const unsigned bin_i = NUM_BINS * (gen_result.most_backtracks) / gen->GIVEUP_THRESHOLD;
			shared_data_.dist_summary.hit_count[bin_i]++;
			shared_data_.dist_summary.total_ops[bin_i] += gen_result.op_count;

			gen_result_consumer_(std::move(gen_result));
		}
		mutex.unlock();
	}

	template<Order O>
	SharedData batch(const Params params, void(& gen_result_consumer)(const typename Generator<O>::GenResult)) {
		auto wall_clock_start = std::chrono::steady_clock::now();
		auto proc_clock_start = std::clock();

		std::mutex shared_data_mutex;
		SharedData shared_data;

		const num_threads = ThreadFunc<O>::NUM_THREADS;
		std::vector<std::thread> extra_threads(num_threads, std::thread(
			ThreadFunc<O>(params, shared_data, shared_data_mutex, gen_result_consumer),
		));
		for (unsigned i = 0; i < num_threads.size(); i++) {
			extra_threads[i].join();
		}

		const double proc_seconds = ((double)(std::clock() - proc_clock_start) / CLOCKS_PER_SEC);
		const double wall_seconds = ((double)[wall_clock_start](){
			using namespace std::chrono;
			return duration_cast<microseconds>(steady_clock::now() - wall_clock_start);
		}().count() / 1'000'000);
		return BatchReport();
	}
}
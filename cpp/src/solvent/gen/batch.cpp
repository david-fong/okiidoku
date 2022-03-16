#include "solvent/gen/batch.hpp"

#include <thread>
#include <mutex>
#include <algorithm> // min
#include <cassert>

namespace solvent::gen::ss::batch {

	unsigned default_num_threads(const Order O) {
		const unsigned hwc = std::thread::hardware_concurrency();
		if (hwc == 0) {
			// Note: hardware_concurency is specified to be zero if unknown.
			return 1;
		}
		else return std::min(
			try_default_num_extra_threads_(O) + 1,
			hwc == 1 ? 1 : hwc - 1 // leave at least one spare thread
		);
	}


	Params Params::clean(const Order O) noexcept {
		if (num_threads == 0) {
			num_threads = default_num_threads(O);
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
		void operator()();

		ThreadSharedData& sd_;
		std::mutex& sd_mutex_;
		callback_O_t<O> callback_;
		ss::GeneratorO<O> generator_ {};
	};


	template<Order O>
	void ThreadFunc<O>::operator()() {
		sd_mutex_.lock();
		while (sd_.report.progress < sd_.params.stop_after) [[likely]] {
			sd_mutex_.unlock(); //___
				generator_();
			sd_mutex_.lock(); //‾‾‾‾
			++(sd_.report.progress);
			callback_(generator_);
		}
		sd_mutex_.unlock();
	}


	SOLVENT_NO_EXPORT [[gnu::noinline]] BatchReport batch_(const Order O, Params& params, std::function<std::thread(ThreadSharedData&, std::mutex&)> mk_thread) {
		params.clean(O);
		std::mutex sd_mutex;
		ThreadSharedData sd {.params {params}};

		std::vector<std::thread> threads;
		for (unsigned i {0}; i < params.num_threads; ++i) {
			threads.push_back(mk_thread(sd, sd_mutex));
		}
		for (auto& thread : threads) {
			thread.join();
		}
		sd.report.time_elapsed = sd.report.timer.read_elapsed();
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
						return callback(static_cast<const ss::Generator&>(result)); \
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
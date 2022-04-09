#ifndef HPP_OKIIDOKU__GEN__BATCH
#define HPP_OKIIDOKU__GEN__BATCH

#include <okiidoku/gen/stochastic.hpp>
#include <okiidoku/timer.hpp>
#include <okiidoku_export.h>

#include <functional>

namespace okiidoku::gen::ss::batch {

	using trials_t = unsigned long;

	//
	struct OKIIDOKU_EXPORT Params final {
		unsigned num_threads {0}; // Defaulted if zero.
		trials_t stop_after {1};

		// Cleans self and returns a copy of self.
		Params clean(Order O) noexcept;
	};


	//
	struct OKIIDOKU_EXPORT BatchReport final {
		trials_t progress {0};
		Timer timer;
		Timer::Elapsed time_elapsed;
	};


	inline unsigned try_default_num_extra_threads_(const Order O) {
		if (O < 4) { return 0; }
		else if (O == 4) { return 1; }
		else { return 2; }
	};
	unsigned default_num_threads(Order O);


	template<Order O>
	using callback_O_t = std::function<void(const ss::GeneratorO<O>&)>;
	// calls to the callback will be guarded by a mutex.
	template<Order O>
	OKIIDOKU_EXPORT [[nodiscard]] BatchReport batch_O(Params&, callback_O_t<O>);


	using callback_t = std::function<void(const ss::Generator&)>;
	// calls to the callback will be guarded by a mutex.
	// contract: the order is compiled.
	OKIIDOKU_EXPORT [[nodiscard]] BatchReport batch(Order, Params&, callback_t);

// inline void batch() {} // can we do something like this?
}
#endif
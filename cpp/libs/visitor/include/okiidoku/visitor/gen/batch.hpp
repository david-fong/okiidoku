#ifndef HPP_OKIIDOKU__VISITOR__GEN__BATCH
#define HPP_OKIIDOKU__VISITOR__GEN__BATCH

#include <okiidoku/visitor/gen/stochastic.hpp>
// #include <okiidoku/mono/gen/batch.hpp>
#include <okiidoku_export.h>

#include <functional>

namespace okiidoku::visitor::gen::ss::batch {

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
	};


	inline unsigned try_default_num_extra_threads_(const Order O) {
		if (O < 4) { return 0; }
		else if (O == 4) { return 1; }
		else { return 2; }
	};
	unsigned default_num_threads(Order O);


	using callback_t = std::function<void(const ss::Generator&)>;
	// calls to the callback will be guarded by a mutex.
	OKIIDOKU_EXPORT [[nodiscard]] BatchReport batch(Params&, callback_t);
}
#endif
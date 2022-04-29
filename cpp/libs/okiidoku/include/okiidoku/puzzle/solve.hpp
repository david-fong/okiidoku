#ifndef HPP_OKIIDOKU__PUZZLE__SOLVE
#define HPP_OKIIDOKU__PUZZLE__SOLVE

// #include <okiidoku/shared_rng.hpp>
#include <okiidoku/grid.hpp>
#include <okiidoku/detail/order_templates.hpp>

namespace okiidoku::mono::puzzle {

	template<Order O> requires(is_order_compiled(O))
	struct OKIIDOKU_EXPORT SolvePath {
	};

	template<Order O> requires(is_order_compiled(O))
	class OKIIDOKU_EXPORT SolutionWalker final {
	public:
		// TODO.low either 
		struct Iterator final {
			;
		};
	};
}


namespace okiidoku::visitor::puzzle {
}
#endif
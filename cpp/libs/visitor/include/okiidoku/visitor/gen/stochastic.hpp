#ifndef HPP_OKIIDOKU__VISITOR__GEN__STOCHASTIC
#define HPP_OKIIDOKU__VISITOR__GEN__STOCHASTIC

#include <okiidoku/visitor/grid.hpp>
#include <okiidoku/traits.hpp>
#include <okiidoku/okiidoku_config.hpp>
#include <okiidoku_export.h>

#include <random>  // minstd_rand
#include <array>
#include <span>
#include <cassert>

namespace okiidoku::visitor::gen::ss {


	class OKIIDOKU_EXPORT Generator final {
	public:
		void operator()();
		void write_to(grid_span_t sink) const;

	private:
		auto g_;
	};
}
#endif
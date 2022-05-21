#ifndef HPP_OKIIDOKU__PRINT_2D
#define HPP_OKIIDOKU__PRINT_2D

#include <okiidoku/shared_rng.hpp>
#include <okiidoku/detail/export.h>

#include <iosfwd>
#include <functional>
#include <span>

namespace okiidoku {

	// TODO.asap size_t is platform dependent. don't use it. use something that will always work. input is o4x_t, output is o2i_t.
	using print_2d_grid_view = std::function<size_t (size_t rmi)>;
	// contract: each grid view's domain is [0, O4), and range is [0, O2].
	OKIIDOKU_EXPORT void print_2d(std::ostream&, unsigned O, std::span<const print_2d_grid_view> grid_views, SharedRng&);
}
#endif
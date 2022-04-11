#ifndef HPP_OKIIDOKU__VISITOR__MORPH__TRANSFORM
#define HPP_OKIIDOKU__VISITOR__MORPH__TRANSFORM

#include <okiidoku/visitor/grid.hpp>
#include <okiidoku/traits.hpp>
#include <okiidoku/okiidoku_config.hpp>
#include <okiidoku_export.h>

namespace okiidoku::visitor::morph {

	struct OKIIDOKU_EXPORT Transformation final {
		constexpr bool operator==(const Transformation&) const = default;
		void apply_from_to(grid_const_span_t src, grid_span_t dest) const noexcept;
		void apply_in_place(grid_span_t) const noexcept;
		Transformation inverted() const noexcept;
	};
}
#endif
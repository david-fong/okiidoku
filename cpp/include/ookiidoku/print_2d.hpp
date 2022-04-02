#ifndef HPP_OOKIIDOKU__PRINT_2D
#define HPP_OOKIIDOKU__PRINT_2D

#include <ookiidoku_export.h>

#include <iosfwd>
#include <functional>
#include <span>

namespace ookiidoku {

	// OOKIIDOKU_EXPORT [[gnu::hot]] void val_to_str(std::ostream&, Order order, uint8_t value) noexcept;

	using print_2d_grid_view = std::function<size_t (size_t coord)>;
	// contract: each grid view's domain is [0, O4), and range is [0, O2].
	OOKIIDOKU_EXPORT void print_2d(std::ostream&, unsigned, std::span<const print_2d_grid_view> grid_views);
}
#endif
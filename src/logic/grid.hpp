#ifndef HPP_SUDOKU_GRID
#define HPP_SUDOKU_GRID

#include "../util/ansi.hpp"
#include "./enum.hpp"
#include "./size.hpp"

namespace Sudoku::Grid {
	/**
	 */
	template <Order O>
	class Grid {
		static_assert((1 < O) && (O <= MAX_REASONABLE_ORDER));
	public:
		using occmask_t = typename Size<O>::occmask_t;
		using order_t  = typename Size<O>::order_t;
		using length_t = typename Size<O>::length_t;
		using area_t   = typename Size<O>::area_t;
		using value_t  = typename Size<O>::value_t;

	// ========================
	// PUBLIC MEMBERS
	// ========================
	public:
		static constexpr order_t  order  = O;
		static constexpr length_t length = O*O;
		static constexpr area_t   area   = O*O*O*O;

	protected:
		void clear(void);

	// ========================
	// STATIC UTILITIES
	// ========================
	public:
		// Inline functions:
		[[gnu::const]] static constexpr length_t getRow(const area_t index) noexcept { return index / length; }
		[[gnu::const]] static constexpr length_t getCol(const area_t index) noexcept { return index % length; }
		[[gnu::const]] static constexpr length_t getBlk(const area_t index) noexcept { return getBlk(getRow(index), getCol(index)); }
		[[gnu::const]] static constexpr length_t getBlk(const length_t row, const length_t col) noexcept {
			return ((row / order) * order) + (col / order);
		}
		[[gnu::const]] static length_t occmask_popcount(occmask_t occmask) noexcept {
			if constexpr (O < 6) {
				return __builtin_popcount(occmask);
			} else if constexpr (O < 9) {
				return __builtin_popcountl(occmask);
			} else {
				return __builtin_popcountll(occmask);
			}
		}
		[[gnu::const]] static length_t occmask_ctz(occmask_t occmask) noexcept {
			if constexpr (O < 6) {
				return __builtin_ctz(occmask);
			} else if constexpr (O < 9) {
				return __builtin_ctzl(occmask);
			} else {
				return __builtin_ctzll(occmask);
			}
		}
	}; // class Grid

} // namespace Sudoku::Grid

#endif
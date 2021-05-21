#ifndef HPP_SOLVENT_LIB_GRID
#define HPP_SOLVENT_LIB_GRID

#include "../util/ansi.hpp"
#include "./size.hpp"

namespace solvent::lib {
	/**
	 */
	template <Order O>
	class Grid {
		static_assert((1 < O) && (O <= MAX_REASONABLE_ORDER));
	public:
		using occmask_t = typename size<O>::occmask_t;
		using order_t   = typename size<O>::order_t;
		using length_t  = typename size<O>::length_t;
		using area_t    = typename size<O>::area_t;
		using value_t   = typename size<O>::value_t;

		static constexpr order_t  O1 = O;
		static constexpr length_t O2 = O*O;
		static constexpr area_t   O4 = O*O*O*O;

		void clear(void);

		friend std::ostream& operator<<(std::ostream& out, Tile const& t) noexcept {
			// TODO [bug] Handle different blank formats.
			static_assert(O1 <= 6, "I haven't yet decided how to translate for orders > 6.");
			if (__builtin_expect(t.isClear(), false)) {
				return out << ' ';
			} else {
				if constexpr (O1 < 4) {
					return out << (int)t.value;
				} else if constexpr (O1 == 5) {
					return out << static_cast<char>('a' + t.value);
				} else {
					return (t.value < 10)
						? out << static_cast<unsigned>(t.value)
						: out << static_cast<char>('a' + t.value - 10);
				}
			}
		}

	// ========================
	public:
		// Inline functions:
		[[gnu::const]] static constexpr length_t getRow(const area_t index) noexcept { return index / O2; }
		[[gnu::const]] static constexpr length_t getCol(const area_t index) noexcept { return index % O2; }
		[[gnu::const]] static constexpr length_t getBlk(const area_t index) noexcept { return getBlk(getRow(index), getCol(index)); }
		[[gnu::const]] static constexpr length_t getBlk(const length_t row, const length_t col) noexcept {
			return ((row / O1) * O1) + (col / O1);
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
	};
}

#endif
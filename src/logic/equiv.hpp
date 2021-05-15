#ifndef HPP_SUDOKU_SOLVER_EQUIV
#define HPP_SUDOKU_SOLVER_EQUIV

#include "./size.hpp"
#include "./grid.hpp"

#include <array>

namespace Sudoku::Equiv {

	template <Order O>
	class Equiv final : public Sudoku::Grid::Grid<O> {
	public:
		using occmask_t = typename Size<O>::occmask_t;
		using order_t   = typename Size<O>::order_t;
		using length_t  = typename Size<O>::length_t;
		using area_t    = typename Size<O>::area_t;
		using value_t   = typename Size<O>::value_t;

		static constexpr order_t  order  = O;
		static constexpr length_t length = O*O;
		static constexpr area_t   area   = O*O*O*O;

	private:
		const std::array<value_t, area> grid;

		void normalizeSymbolShuffling(void);
	}; // class Equiv

} // namespace Sudoku::Equiv

#endif
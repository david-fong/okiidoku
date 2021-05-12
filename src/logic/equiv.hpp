#ifndef HPP_SUDOKU_SOLVER_ENUM
#define HPP_SUDOKU_SOLVER_ENUM

#include "./size.hpp"

namespace Sudoku::Equiv {

	template <Order O>
	class Equiv final : public Sudoku::Grid::Grid<O> {
		void normalizeSymbolShuffling();
	};

} // namespace Sudoku::Equiv

#endif
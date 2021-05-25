#include "./path.hpp"

namespace solvent::lib::gen::path {

	template<Kind PK, Order O>
	struct PathCoords_ {
		using ord1_t = typename size<O>::ord1_t;
		using ord2_t = typename size<O>::ord2_t;
		using ord4_t = typename size<O>::ord4_t;
		static constexpr ord1_t O1 = O;
		static constexpr ord2_t O2 = O*O;
		static constexpr ord4_t O4 = O*O*O*O;

		inline static constexpr ord4_t convert(const ord4_t progress) {
			if constexpr (PK == path::Kind::RowMajor) {
				return progress;
			} else {
				return path[progress];
			}
		}
	 private:
		static constexpr std::array<const ord4_t, O4> path = PathCoords_<PK,O>::_init();
		static constexpr std::array<const ord4_t, O4> _init() {
			const std::array<const ord4_t, O4> path;
			if constexpr (PK == Kind::RowMajor) {
				// std::iota(path.begin(), path.end(), 0);
			}
			else if constexpr (PK == Kind::DealRwMj) {
				ord4_t i = 0;
				for (ord1_t b_row = 0; b_row < O1; b_row++) {
					for (ord1_t b_col = 0; b_col < O1; b_col++) {
						for (ord2_t blk = 0; blk < O2; blk++) {
							ord4_t blkaddr = ((blk % O1) * O1) + (blk / O1 * O1 * O2);
							path[i++] = blkaddr + (b_row * O2) + b_col;
						}
					}
				}
			}
			else if constexpr (PK == Kind::BlockCol) {
				ord4_t i = 0;
				for (ord1_t blk_col = 0; blk_col < O1; blk_col++) {
					for (ord2_t row = 0; row < O2; row++) {
						for (ord1_t b_col = 0; b_col < O1; b_col++) {
							path[i++] = (blk_col * O1) + (row * O2) + (b_col);
						}
					}
				}
			}
			return path; // https://wikipedia.org/wiki/Copy_elision#Return_value_optimization
		}
	};
	template<const Kind PK, Order O>
	const std::array<const typename size<O>::ord4_t, PathCoords_<PK,O>::O4> PathCoords_<PK,O>::path;

	template<solvent::Order O>
	constexpr std::array<typename size<O>::ord4_t (&)(typename size<O>::ord4_t), NUM_KINDS> PathCoords = {
		PathCoords_<Kind::RowMajor, O>::convert,
		PathCoords_<Kind::DealRwMj, O>::convert,
		PathCoords_<Kind::BlockCol, O>::convert,
	};
}
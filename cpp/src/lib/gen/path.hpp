#ifndef HPP_SOLVENT_LIB_GEN_PATH
#define HPP_SOLVENT_LIB_GEN_PATH

#include "../size.hpp"

#include <array>
#include <string>
// #include <numeric> // iota
#include <iostream>

namespace solvent::lib::gen {

	enum class PathDirection : bool {
		Back, Forward,
	};

	enum class ExitStatus {
		Exhausted, Abort, Ok,
	};

	namespace PathKind {
		enum class E : unsigned {
			RowMajor,
			DealRwMj,
			BlockCol,
			__MAX__ = BlockCol,
		};
		constexpr size_t E_SIZE = static_cast<size_t>(E::__MAX__) + 1;
		// Indices of entries must match the
		// literal values of their respective enums.
		const std::array<std::string, E_SIZE> NAMES = {
			"rowmajor",
			"dealrwmj",
			"blockcol",
		};
		std::ostream& operator<<(std::ostream& os, const E path_kind) {
			return os << NAMES[static_cast<unsigned>(path_kind)];
		}
		const std::string OPTIONS_MENU = "\nGEN-PATH OPTIONS:"
			"\n- rowmajor   horizontal strips as wide as the grid one by one"
			"\n- dealrwmj   like dealing cards to each block using row-major"
			"\n- blockcol   rowmajor, but broken into columns one block wide";
	}

	template<PathKind::E PK, Order O>
	struct PathCoords {
		using ord1_t = typename size<O>::ord1_t;
		using ord2_t = typename size<O>::ord2_t;
		using ord4_t = typename size<O>::ord4_t;
		static constexpr ord1_t O1 = O;
		static constexpr ord2_t O2 = O*O;
		static constexpr ord4_t O4 = O*O*O*O;

		inline static constexpr ord4_t convert(const ord4_t progress) {
			if constexpr (PK == PathKind::E::RowMajor) {
				return progress;
			} else {
				return path[progress];
			}
		}
	 private:
		static constexpr std::array<const ord4_t, O4> path = PathCoords<P,O>::_init();
		static constexpr std::array<const ord4_t, O4> _init() {
			const std::array<const ord4_t, O4> path;
			if constexpr (PK == PathKind::E::RowMajor) {
				// std::iota(path.begin(), path.end(), 0);
			}
			else if constexpr (PK == PathKind::E::DealRwMj) {
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
			else if constexpr (PK == PathKind::E::BlockCol) {
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
	template<const PathKind::E PK, Order O>
	const std::array<const typename size<O>::ord4_t, PathCoords<PK,O>::O4> PathCoords<PK,O>::path;

	namespace PathKind {
		template<solvent::Order O>
		const std::array<typename size<O>::ord4_t (&)(typename size<O>::ord4_t), E_SIZE> PathCoords = {
			gen::PathCoords<E::RowMajor, O>::convert,
			gen::PathCoords<E::DealRwMj, O>::convert,
			gen::PathCoords<E::BlockCol, O>::convert,
		};
	}
}

#endif
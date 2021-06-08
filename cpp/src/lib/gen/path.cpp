#include ":/lib/gen/path.hpp"

namespace solvent::lib::gen::path {

	std::ostream& operator<<(std::ostream& os, const Kind path_kind) {
		return os << NAMES[static_cast<size_t>(path_kind)];
	}


	template<Kind PK, Order O>
	struct PathCoords_ {
		using ord1_t = typename size<O>::ord1_t;
		using ord2_t = typename size<O>::ord2_t;
		using ord4_t = typename size<O>::ord4_t;
		static constexpr ord1_t O1 = O;
		static constexpr ord2_t O2 = O*O;
		static constexpr ord4_t O4 = O*O*O*O;

		[[gnu::pure]] static constexpr ord4_t convert(const ord4_t progress) noexcept {
			if constexpr (PK == path::Kind::RowMajor) {
				return progress;
			} else {
				return path[progress];
			}
		}
	 private:
		static constexpr const std::array<ord4_t, O4> _init() noexcept {
			std::array<ord4_t, O4> path_tmp = {0};
			if constexpr (PK == Kind::RowMajor) {
				// std::iota(path.begin(), path.end(), 0);
			}
			else if constexpr (PK == Kind::DealRwMj) {
				ord4_t i = 0;
				for (ord1_t b_row = 0; b_row < O1; b_row++) {
					for (ord1_t b_col = 0; b_col < O1; b_col++) {
						for (ord2_t blk = 0; blk < O2; blk++) {
							ord4_t blkaddr = ((blk % O1) * O1) + (blk / O1 * O1 * O2);
							path_tmp[i++] = blkaddr + (b_row * O2) + b_col;
						}
					}
				}
			}
			else if constexpr (PK == Kind::BlockCol) {
				ord4_t i = 0;
				for (ord1_t blk_col = 0; blk_col < O1; blk_col++) {
					for (ord2_t row = 0; row < O2; row++) {
						for (ord1_t b_col = 0; b_col < O1; b_col++) {
							path_tmp[i++] = (blk_col * O1) + (row * O2) + (b_col);
						}
					}
				}
			}
			return static_cast<const std::array<ord4_t, O4>>(path_tmp);
		}
		static constexpr const std::array<ord4_t, O4> path = PathCoords_<PK,O>::_init(); // TODO make this sizeless when not used.
	};


	#define SOLVENT_TEMPL_TEMPL(O_) \
	template struct PathCoords_<Kind::RowMajor, O_>; \
	template struct PathCoords_<Kind::DealRwMj, O_>; \
	template struct PathCoords_<Kind::BlockCol, O_>;
	SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef SOLVENT_TEMPL_TEMPL


	template<solvent::Order O>
	coord_converter_t<O> GetPathCoords(const Kind path_kind) noexcept {
		switch (path_kind) {
		 case Kind::RowMajor: return PathCoords_<Kind::RowMajor, O>::convert;
		 case Kind::DealRwMj: return PathCoords_<Kind::DealRwMj, O>::convert;
		 case Kind::BlockCol: return PathCoords_<Kind::BlockCol, O>::convert;
		 default: return PathCoords_<Kind::RowMajor, O>::convert; // never
		}
	}

	#define SOLVENT_TEMPL_TEMPL(O_) \
		template coord_converter_t<O_> GetPathCoords<O_>(Kind) noexcept;
	SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef SOLVENT_TEMPL_TEMPL
}
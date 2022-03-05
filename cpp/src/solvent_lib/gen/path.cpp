#include "solvent_lib/gen/path.hpp"

namespace solvent::lib::gen::path {

	std::ostream& operator<<(std::ostream& os, const Kind path_kind) {
		return os << names[static_cast<size_t>(path_kind)];
	}


	template<Kind PK, Order O>
	struct PathCoords_ final {
	 private:
		using ord1i_t = size<O>::ord1i_t;
		using ord2i_t = size<O>::ord2i_t;
		using ord4i_t = size<O>::ord4i_t;
		using ord4x_t = size<O>::ord4x_t;
		using ord4x_least_t = size<O>::ord4x_least_t;
	 public:
		static constexpr ord1i_t O1 = O;
		static constexpr ord2i_t O2 = O*O;
		static constexpr ord4i_t O4 = O*O*O*O;

		[[gnu::const, gnu::hot]] static constexpr ord4x_t prog2coord(const ord4x_t progress) noexcept {
			if constexpr (PK == Kind::RowMajor) {
				return progress;
			} else {
				return map_prog2coord[progress];
			}
		}
		[[gnu::const, gnu::hot]] static constexpr ord4x_t coord2prog(const ord4x_t progress) noexcept {
			if constexpr (PK == Kind::RowMajor) {
				return progress;
			} else {
				return map_coord2prog[progress];
			}
		}
	 private:
		using grid_cache_t = typename std::array<ord4x_least_t, O4>;
		static consteval grid_cache_t init_map_prog2coord_() noexcept {
			grid_cache_t _{0};
			if constexpr (PK == Kind::RowMajor) {
				// std::iota(path.begin(), path.end(), 0);
			}
			else if constexpr (PK == Kind::BlockCol) {
				ord4i_t i = 0;
				for (ord1i_t blk_col = 0; blk_col < O1; ++blk_col) {
					for (ord2i_t row = 0; row < O2; ++row) {
						for (ord1i_t b_col = 0; b_col < O1; ++b_col) {
							_[i++] = static_cast<ord4x_least_t>((blk_col * O1) + (row * O2) + (b_col));
				}	}	}
			}
			else if constexpr (PK == Kind::DealRwMj) {
				ord4i_t i = 0;
				for (ord1i_t inside_b_row = 0; inside_b_row < O1; ++inside_b_row) {
					for (ord1i_t inside_b_col = 0; inside_b_col < O1; ++inside_b_col) {
						for (ord2i_t blk_i = 0; blk_i < O2; ++blk_i) {
							const ord4i_t blkaddr = static_cast<ord4i_t>(((blk_i % O1) * O1) + (blk_i / O1 * O1 * O2));
							_[i++] = static_cast<ord4x_least_t>(blkaddr + (inside_b_row * O2) + inside_b_col);
				}	}	}
			}
			return _;
		}
		static consteval grid_cache_t init_map_coord2prog_() noexcept {
			grid_cache_t _{0};
			for (ord4i_t i = 0; i < O4; ++i) {
				_[map_prog2coord[i]] = static_cast<ord4x_least_t>(i);
			}
			return _;
		}
		static constexpr grid_cache_t map_prog2coord = PathCoords_<PK,O>::init_map_prog2coord_();
		static constexpr grid_cache_t map_coord2prog = PathCoords_<PK,O>::init_map_coord2prog_();
		// Note: a compiler can optimize this away if not used.
	};


	#define M_SOLVENT_TEMPL_TEMPL(O_) \
		template struct PathCoords_<Kind::RowMajor, O_>; \
		template struct PathCoords_<Kind::BlockCol, O_>; \
		template struct PathCoords_<Kind::DealRwMj, O_>;
	M_SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef M_SOLVENT_TEMPL_TEMPL


	template<solvent::Order O>
	coord_converter_t<O> get_prog2coord_converter(const Kind path_kind) noexcept {
		switch (path_kind) {
		case Kind::RowMajor: return PathCoords_<Kind::RowMajor, O>::prog2coord;
		case Kind::BlockCol: return PathCoords_<Kind::BlockCol, O>::prog2coord;
		case Kind::DealRwMj: return PathCoords_<Kind::DealRwMj, O>::prog2coord;
		default: return PathCoords_<Kind::RowMajor, O>::prog2coord; // never
		}
	}

	template<solvent::Order O>
	coord_converter_t<O> get_coord2prog_converter(const Kind path_kind) noexcept {
		switch (path_kind) {
		case Kind::RowMajor: return PathCoords_<Kind::RowMajor, O>::coord2prog;
		case Kind::BlockCol: return PathCoords_<Kind::BlockCol, O>::coord2prog;
		case Kind::DealRwMj: return PathCoords_<Kind::DealRwMj, O>::coord2prog;
		default: return PathCoords_<Kind::RowMajor, O>::coord2prog; // never
		}
	}


	#define M_SOLVENT_TEMPL_TEMPL(O_) \
		template coord_converter_t<O_> get_prog2coord_converter<O_>(Kind) noexcept; \
		template coord_converter_t<O_> get_coord2prog_converter<O_>(Kind) noexcept;
	M_SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef M_SOLVENT_TEMPL_TEMPL
}
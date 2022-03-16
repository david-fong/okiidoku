#include "solvent/gen/bt/path.hpp"

namespace solvent::gen::bt::path {

	std::ostream& operator<<(std::ostream& os, const E path_kind) {
		return os << names[static_cast<size_t>(path_kind)];
	}


	template<E PK, Order O>
	struct SOLVENT_NO_EXPORT PathCoords_ final {
	private:
		using ord1i_t = size<O>::ord1i_t;
		using ord2i_t = size<O>::ord2i_t;
		using ord4x_least_t = size<O>::ord4x_least_t;
		using ord4x_t = size<O>::ord4x_t;
		using ord4i_t = size<O>::ord4i_t;
	public:
		static constexpr ord1i_t O1 = O;
		static constexpr ord2i_t O2 = O*O;
		static constexpr ord4i_t O4 = O*O*O*O;

		[[gnu::const, gnu::hot]]
		static constexpr ord4x_t prog_to_coord(const ord4x_t progress) noexcept {
			if constexpr (PK == E::row_major) {
				return progress;
			} else {
				return map_prog_to_coord[progress];
			}
		}
		[[gnu::const, gnu::hot]]
		static constexpr ord4x_t coord_to_prog(const ord4x_t progress) noexcept {
			if constexpr (PK == E::row_major) {
				return progress;
			} else {
				return map_coord_to_prog[progress];
			}
		}
	private:
		using grid_cache_t = typename std::array<ord4x_least_t, O4>;
		static consteval grid_cache_t init_map_prog_to_coord_() noexcept {
			grid_cache_t _{0};
			if constexpr (PK == E::row_major) {
				// std::iota(path.begin(), path.end(), 0);
			}
			else if constexpr (PK == E::block_col) {
				ord4i_t i {0};
				for (ord1i_t blk_col {0}; blk_col < O1; ++blk_col) {
					for (ord2i_t row {0}; row < O2; ++row) {
						for (ord1i_t b_col {0}; b_col < O1; ++b_col) {
							_[i++] = static_cast<ord4x_least_t>((blk_col * O1) + (row * O2) + (b_col));
				}	}	}
			}
			else if constexpr (PK == E::dealer_row_major) {
				ord4i_t i {0};
				for (ord1i_t inside_b_row {0}; inside_b_row < O1; ++inside_b_row) {
					for (ord1i_t inside_b_col {0}; inside_b_col < O1; ++inside_b_col) {
						for (ord2i_t blk_i {0}; blk_i < O2; ++blk_i) {
							const ord4i_t blkaddr = static_cast<ord4i_t>(((blk_i % O1) * O1) + (blk_i / O1 * O1 * O2));
							_[i++] = static_cast<ord4x_least_t>(blkaddr + (inside_b_row * O2) + inside_b_col);
				}	}	}
			}
			return _;
		}
		static consteval grid_cache_t init_map_coord_to_prog_() noexcept {
			grid_cache_t _{0};
			for (ord4i_t i {0}; i < O4; ++i) {
				_[map_prog_to_coord[i]] = static_cast<ord4x_least_t>(i);
			}
			return _;
		}
		static constexpr grid_cache_t map_prog_to_coord = PathCoords_<PK,O>::init_map_prog_to_coord_();
		static constexpr grid_cache_t map_coord_to_prog = PathCoords_<PK,O>::init_map_coord_to_prog_();
		// Note: a compiler can optimize this away if not used.
	};


	#define M_SOLVENT_TEMPL_TEMPL(O_) \
		template struct PathCoords_<E::row_major, O_>; \
		template struct PathCoords_<E::block_col, O_>; \
		template struct PathCoords_<E::dealer_row_major, O_>;
	M_SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef M_SOLVENT_TEMPL_TEMPL


	template<solvent::Order O>
	coord_converter_t<O> get_prog_to_coord_converter(const E path_kind) noexcept {
		switch (path_kind) {
		case E::row_major: return PathCoords_<E::row_major, O>::prog_to_coord;
		case E::block_col: return PathCoords_<E::block_col, O>::prog_to_coord;
		case E::dealer_row_major: return PathCoords_<E::dealer_row_major, O>::prog_to_coord;
		default: return PathCoords_<E::row_major, O>::prog_to_coord; // never
		}
	}

	template<solvent::Order O>
	coord_converter_t<O> get_coord_to_prog_converter(const E path_kind) noexcept {
		switch (path_kind) {
		case E::row_major: return PathCoords_<E::row_major, O>::coord_to_prog;
		case E::block_col: return PathCoords_<E::block_col, O>::coord_to_prog;
		case E::dealer_row_major: return PathCoords_<E::dealer_row_major, O>::coord_to_prog;
		default: return PathCoords_<E::row_major, O>::coord_to_prog; // never
		}
	}


	#define M_SOLVENT_TEMPL_TEMPL(O_) \
		template coord_converter_t<O_> get_prog_to_coord_converter<O_>(E) noexcept; \
		template coord_converter_t<O_> get_coord_to_prog_converter<O_>(E) noexcept;
	M_SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef M_SOLVENT_TEMPL_TEMPL
}
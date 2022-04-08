#include <okiidoku/gen/deprecated_bt/path.hpp>

namespace okiidoku::gen::bt::path {

	std::ostream& operator<<(std::ostream& os, const E path_kind) {
		return os << names[static_cast<size_t>(path_kind)];
	}


	template<E PK, Order O>
	struct OKIIDOKU_NO_EXPORT PathCoords_ final {
	private:
		using o1i_t = traits<O>::o1i_t;
		using o2i_t = traits<O>::o2i_t;
		using o4x_smol_t = traits<O>::o4x_smol_t;
		using o4x_t = traits<O>::o4x_t;
		using o4i_t = traits<O>::o4i_t;
	public:
		static constexpr o1i_t O1 = O;
		static constexpr o2i_t O2 = O*O;
		static constexpr o4i_t O4 = O*O*O*O;

		[[gnu::const, gnu::hot]]
		static constexpr o4x_t prog_to_coord(const o4x_t progress) noexcept {
			if constexpr (PK == E::row_major) {
				return progress;
			} else {
				return map_prog_to_coord[progress];
			}
		}
		[[gnu::const, gnu::hot]]
		static constexpr o4x_t coord_to_prog(const o4x_t progress) noexcept {
			if constexpr (PK == E::row_major) {
				return progress;
			} else {
				return map_coord_to_prog[progress];
			}
		}
	private:
		using grid_cache_t = typename std::array<o4x_smol_t, O4>;
		static consteval grid_cache_t init_map_prog_to_coord_() noexcept {
			grid_cache_t _{0};
			if constexpr (PK == E::row_major) {
				// std::iota(path.begin(), path.end(), 0);
			}
			else if constexpr (PK == E::box_col) {
				o4i_t i {0};
				for (o1i_t box_col {0}; box_col < O1; ++box_col) {
					for (o2i_t row {0}; row < O2; ++row) {
						for (o1i_t b_col {0}; b_col < O1; ++b_col) {
							_[i++] = static_cast<o4x_smol_t>((box_col * O1) + (row * O2) + (b_col));
				}	}	}
			}
			else if constexpr (PK == E::dealer_row_major) {
				o4i_t i {0};
				for (o1i_t inside_b_row {0}; inside_b_row < O1; ++inside_b_row) {
					for (o1i_t inside_b_col {0}; inside_b_col < O1; ++inside_b_col) {
						for (o2i_t box_i {0}; box_i < O2; ++box_i) {
							const o4i_t boxaddr = static_cast<o4i_t>(((box_i % O1) * O1) + (box_i / O1 * O1 * O2));
							_[i++] = static_cast<o4x_smol_t>(boxaddr + (inside_b_row * O2) + inside_b_col);
				}	}	}
			}
			return _;
		}
		static consteval grid_cache_t init_map_coord_to_prog_() noexcept {
			grid_cache_t _{0};
			for (o4i_t i {0}; i < O4; ++i) {
				_[map_prog_to_coord[i]] = static_cast<o4x_smol_t>(i);
			}
			return _;
		}
		static constexpr grid_cache_t map_prog_to_coord = PathCoords_<PK,O>::init_map_progo_coord_();
		static constexpr grid_cache_t map_coord_to_prog = PathCoords_<PK,O>::init_map_coordo_prog_();
		// Note: a compiler can optimize this away if not used.
	};


	#define M_OKIIDOKU_TEMPL_TEMPL(O_) \
		template struct PathCoords_<E::row_major, O_>; \
		template struct PathCoords_<E::box_col, O_>; \
		template struct PathCoords_<E::dealer_row_major, O_>;
	M_OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef M_OKIIDOKU_TEMPL_TEMPL


	template<okiidoku::Order O>
	coord_converter<O> get_prog_to_coord_converter(const E path_kind) noexcept {
		switch (path_kind) {
		case E::row_major: return PathCoords_<E::row_major, O>::prog_to_coord;
		case E::box_col: return PathCoords_<E::box_col, O>::prog_to_coord;
		case E::dealer_row_major: return PathCoords_<E::dealer_row_major, O>::prog_to_coord;
		default: return PathCoords_<E::row_major, O>::prog_to_coord; // never
		}
	}

	template<okiidoku::Order O>
	coord_converter<O> get_coord_to_prog_converter(const E path_kind) noexcept {
		switch (path_kind) {
		case E::row_major: return PathCoords_<E::row_major, O>::coord_to_prog;
		case E::box_col: return PathCoords_<E::box_col, O>::coord_to_prog;
		case E::dealer_row_major: return PathCoords_<E::dealer_row_major, O>::coord_to_prog;
		default: return PathCoords_<E::row_major, O>::coord_to_prog; // never
		}
	}


	#define M_OKIIDOKU_TEMPL_TEMPL(O_) \
		template coord_converter<O_> get_prog_to_coord_converter<O_>(E) noexcept; \
		template coord_converter<O_> get_coord_to_prog_converter<O_>(E) noexcept;
	M_OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef M_OKIIDOKU_TEMPL_TEMPL
}
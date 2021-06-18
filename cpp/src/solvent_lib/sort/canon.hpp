#ifndef HPP_SOLVENT_LIB_SORT_CANON
#define HPP_SOLVENT_LIB_SORT_CANON

#include <solvent_lib/grid.hpp>
#include <solvent_lib/size.hpp>

#include <array>

namespace solvent::lib::canon {

	template<Order O>
	class Canonicalizer final : public solvent::lib::AbstractGrid<O> {
	 private:
		using has_mask_t = typename size<O>::has_mask_t;
		using ord1_t  = typename size<O>::ord1_t;
		using ord2_t  = typename size<O>::ord2_t;
		using ord4_t  = typename size<O>::ord4_t;

	 public:
		static constexpr ord1_t O1 = O;
		static constexpr ord2_t O2 = O*O;
		static constexpr ord4_t O4 = O*O*O*O;
		[[gnu::pure]] ord2_t operator[](ord4_t coord) const override;

	 private:
		const std::array<ord2_t, O4> buf_;

		void handle_relabeling(void) noexcept;
	};


	#define SOLVENT_TEMPL_TEMPL(O_) \
		extern template class Canonicalizer<O_>;
	SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef SOLVENT_TEMPL_TEMPL
}
#endif
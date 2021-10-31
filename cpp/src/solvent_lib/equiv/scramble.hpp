#ifndef HPP_SOLVENT_LIB_EQUIV_SCRAMBLE
#define HPP_SOLVENT_LIB_EQUIV_SCRAMBLE

// #include <solvent_lib/grid.hpp>
#include <solvent_lib/size.hpp>

#include <vector>
#include <array>

namespace solvent::lib::scramble {

	template<Order O>
	using vec_grid_t = std::vector<typename size<O>::ord2_t>;

	template<Order O>
	vec_grid_t<O> scramble(vec_grid_t<O> const& input);

	//
	template<Order O>
	class Scrambler final /* : public AbstractGrid<O> */ {
	 private:
		using has_mask_t = typename size<O>::has_mask_t;
		using ord1_t = typename size<O>::ord1_t;
		using ord2_t = typename size<O>::ord2_t;
		using ord4_t = typename size<O>::ord4_t;

		using grid_arr_t = typename std::array<std::array<ord2_t, O*O>, O*O>;
		using input_it_t = typename std::array<ord2_t, O*O>::const_iterator;

	 public:
		static constexpr ord1_t O1 = O;
		static constexpr ord2_t O2 = O*O;
		static constexpr ord2_t O3 = O*O*O;
		static constexpr ord4_t O4 = O*O*O*O;
		// [[gnu::pure]] ord2_t operator[](ord4_t coord) const override;

		Scrambler(std::vector<ord2_t> const&);

		std::vector<ord2_t> operator()(void);

		// void print_pretty(std::ostream&) const;

	 private:
		std::array<std::array<ord2_t, O2>, O2> input_;
	};


	#define SOLVENT_TEMPL_TEMPL(O_) \
		extern template vec_grid_t<O_> scramble<O_>(vec_grid_t<O_> const&); \
		extern template class Scrambler<O_>;
	SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef SOLVENT_TEMPL_TEMPL
}
#endif
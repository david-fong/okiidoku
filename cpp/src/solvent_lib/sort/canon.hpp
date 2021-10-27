#ifndef HPP_SOLVENT_LIB_SORT_CANON
#define HPP_SOLVENT_LIB_SORT_CANON

#include <solvent_lib/grid.hpp>
#include <solvent_lib/size.hpp>

#include <iosfwd>
#include <vector>

namespace solvent::lib::canon {

	/**
	input must be a complete grid.

	Goal:
	- be efficient and elegant: no brute forcing.
	- be "meaningful": default to factoring as many cells as possible
		into sorting bases, and only factoring them out when necessary
		to break ties instead of vice versa.
	- be simple.

	Note that there are many possible variations on the calculations
	used in this algorithm that can just as well canonicalize. I made
	decisions according to the above goals.
	*/
	template<Order O>
	void canonicalize(std::vector<typename size<O>::ord2_t>& input) noexcept;

	//
	template<Order O>
	class Canonicalizer final /* : public AbstractGrid<O> */ {
	 private:
		using has_mask_t = typename size<O>::has_mask_t;
		using ord1_t = typename size<O>::ord1_t;
		using ord2_t = typename size<O>::ord2_t;
		using ord4_t = typename size<O>::ord4_t;

		using atom_slide_t = std::array<ord4_t, O>;
		using line_slide_t = std::array<atom_slide_t, O>;

		struct cmp_less_atom_slides {
			[[gnu::const]] bool operator()(atom_slide_t const& lhs, atom_slide_t const& rhs) const;
		};
		struct cmp_less_line_slides {
			// returns true if lhs is less than rhs
			[[gnu::const]] bool operator()(line_slide_t const& lhs, line_slide_t const& rhs) const;
		};

	 public:
		static constexpr ord1_t O1 = O;
		static constexpr ord2_t O2 = O*O;
		static constexpr ord2_t O3 = O*O*O;
		static constexpr ord4_t O4 = O*O*O*O;
		// [[gnu::pure]] ord2_t operator[](ord4_t coord) const override;

		Canonicalizer(std::vector<ord2_t> const&);

		// Generates a fresh sudoku solution.
		std::vector<ord2_t> operator()(void);

		// void print_pretty(std::ostream&) const;

	 private:
		std::array<std::array<ord2_t, O2>, O2> input_;

		//
		void relabel_(void) noexcept;

		//
		void movement_(void);
	};


	#define SOLVENT_TEMPL_TEMPL(O_) \
		extern template void canonicalize<O_>(std::vector<typename size<O_>::ord2_t>&) noexcept; \
		extern template class Canonicalizer<O_>;
	SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef SOLVENT_TEMPL_TEMPL
}
#endif
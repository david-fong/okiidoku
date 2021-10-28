#ifndef HPP_SOLVENT_LIB_SORT_CANON
#define HPP_SOLVENT_LIB_SORT_CANON

#include <solvent_lib/grid.hpp>
#include <solvent_lib/size.hpp>

#include <iosfwd>
#include <vector>
#include <compare>

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
		using ord5_t = typename size<O>::ord5_t;

		using grid_arr_t = typename std::array<std::array<ord2_t, O*O>, O*O>;
		using input_it_t = typename std::array<ord2_t, O*O>::const_iterator;

		struct AtomSlide final {
			std::array<ord5_t, O> slide_;
			static AtomSlide build(input_it_t atom_it);
			[[gnu::pure]] ord5_t operator[](ord1_t i) const { return slide_[i]; }
			[[gnu::pure]] std::strong_ordering operator<=>(AtomSlide const& that) const;
			AtomSlide& operator+=(AtomSlide const& other) { for (ord1_t i = 0; i < O; i++) { slide_[i] += other.slide_[i]; }}
		};
		struct LineSlide final {
			ord1_t orig_blkline;
			std::array<AtomSlide, O> slide_;
			static LineSlide build(ord1_t orig_blkline, std::array<ord2_t, O*O> const& line_it);
			[[gnu::pure]] AtomSlide const& operator[](ord1_t i) const { return slide_[i]; }
			[[gnu::pure]] std::strong_ordering operator<=>(LineSlide const& that) const;
			LineSlide& operator+=(LineSlide const& other) { for (ord1_t i = 0; i < O; i++) { slide_[i] += other.slide_[i]; }}

		};
		struct ChuteSlide final {
			ord1_t orig_chute;
			std::array<LineSlide, O> slide_;
			static ChuteSlide build(ord1_t orig_chute, grid_arr_t const& chute_it);
			[[gnu::const]] LineSlide const& operator[](ord1_t i) const { return slide_[i]; }
			[[gnu::const]] std::strong_ordering operator<=>(ChuteSlide const& that) const;
			ChuteSlide& operator+=(ChuteSlide const& other) { for (ord1_t i = 0; i < O; i++) { slide_[i] += other.slide_[i]; }}
		};
		struct GridSlide final {
			std::array<ChuteSlide, O> slide_;
			static GridSlide build(grid_arr_t const& grid_it);
			[[gnu::const]] ChuteSlide const& operator[](ord1_t i) const { return slide_[i]; }
			[[gnu::const]] std::strong_ordering operator<=>(GridSlide const& that) const;
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
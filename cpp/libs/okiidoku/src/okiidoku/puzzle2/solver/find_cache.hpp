// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef HPP_OKIIDOKU_PUZZLE_SOLVER2_FIND_CACHE
#define HPP_OKIIDOKU_PUZZLE_SOLVER2_FIND_CACHE

#include <okiidoku/bit_array.hpp>

#include <utility> // forward_like

namespace okiidoku::mono::detail::solver2 {

	// TODO.design hmmm why couldn't this just be part of the cands interfaces? I suppose because fish needs one of these, but fish can share cands pov with sym-major subset pov?
	// for "subsets", each house has a `Set`.
	// for "fish", each symbol has a `Set`.
	template<Order O> requires(is_order_compiled(O))
	struct CandPartitions final {
	private:
		using T = Ints<O>;
		using o2i_t = T::o2i_t;
	public:
		struct [[gnu::designated_init]] Set final {
			struct [[gnu::designated_init]] Tag final {
				// for house cell, how many cand syms - 1?
				// for house sym, how many cand cells - 1?
				// for sym line,  how many cand cells - 1?
				Ints<O>::o2xs_t num_alts;
				// for subsets, points to house set's cell or sym
				// for fish, points to sym set's line
				Ints<O>::o2xs_t at;
			};
			O2BitArr<O> is_begin;
			std::array<Tag, T::O2> tags;
		}
	private:
		std::array<Set, T::O2> sets_;
	public:
		// TODO an initialization function. maybe a reinit
		[[nodiscard, gnu::pure]]
		decltype(auto) operator[](this auto&& self, const o2i_t set) noexcept { set.check(); return std::forward<decltype(self)>(self.sets_[set]); }

		void learn_complementary_partition(const O2BitArr<O>&) noexcept;
	};


	template<Order O> requires(is_order_compiled(O))
	using FindCacheForSubsets = HouseTypeMap<CandPartitions>;
	// TODO wait... note: if we have separate for each CellOrSym major POV, need to design a way to quickly sync them.


	template<Order O> requires(is_order_compiled(O))
	using FindCacheForFish = LineTypeMap<CandPartitions>;
}
#endif
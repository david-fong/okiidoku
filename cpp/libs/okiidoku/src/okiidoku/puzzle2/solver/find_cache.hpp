// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef HPP_OKIIDOKU__PUZZLE__SOLVER2__FIND_CACHE
#define HPP_OKIIDOKU__PUZZLE__SOLVER2__FIND_CACHE

#include <okiidoku/o2_bit_arr.hpp>

#include <utility> // forward

namespace okiidoku::mono::detail::solver2 {

	// TODO.design hmmm why couldn't this just be part of the cands interfaces? I suppose because fish needs one of these, but fish can share cands pov with sym-major subset pov?
	// for "subsets", each house has a `Set`.
	// for "fish", each symbol has a `Set`.
	template<Order O> requires(is_order_compiled(O))
	struct CandPartitions final {
	private:
		using T = Ints<O>;
		using o2i_t = int_ts::o2i_t<O>;
	public:
		struct [[gnu::designated_init]] Set final {
			struct [[gnu::designated_init]] Tag final {
				// for house cell, how many cand syms - 1?
				// for house sym, how many cand cells - 1?
				// for sym line,  how many cand cells - 1?
				int_ts::o2xs_t<O> num_alts;
				// for subsets, points to house set's cell or sym
				// for fish, points to sym set's line
				int_ts::o2xs_t<O> at;
			};
			O2BitArr<O> is_begin;
			std::array<Tag, T::O2> tags;
		}
	private:
		std::array<Set, T::O2> sets_;
	public:
		// TODO an initialization function. maybe a reinit
		template<class Self> [[nodiscard, gnu::pure]]
		auto&& operator[](this Self&& self, const o2i_t set) noexcept { return std::forward<Self>(self).sets_[set]; }

		void learn_complementary_partition(const O2BitArr<O>&) noexcept;
	};


	template<Order O> requires(is_order_compiled(O))
	struct FindCacheForSubsets final {
	private:
		using T = Ints<O>;
		using o2i_t = int_ts::o2i_t<O>;
		HouseTypeMap<CandPartitions> types_;
	public:
		[[nodiscard, gnu::pure]]
		auto& house(const HouseType type, const o2i_t house) noexcept { return types_.at(type)[house]; }
	};
	// TODO wait... note: if we have separate for each CellOrSym major POV, need to design a way to quickly sync them.


	template<Order O> requires(is_order_compiled(O))
	struct FindCacheForFish final {
	private:
		using T = Ints<O>;
		using o2i_t = int_ts::o2i_t<O>;
		LineTypeMap<CandPartitions> types_;
	public:
		[[nodiscard, gnu::pure]]
		auto& sym(const LineType type, const o2i_t sym) noexcept { return types_.at(type)[sym]; }
	};
}
#endif
// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef HPP_OKIIDOKU_PUZZLE_SOLVER2_CANDS
#define HPP_OKIIDOKU_PUZZLE_SOLVER2_CANDS

#include <okiidoku/grid.hpp>
#include <okiidoku/o2_bit_arr.hpp>

#include <vector>
#include <array>

namespace okiidoku::mono::detail::solver2 {

	inline constexpr Order order_threshold_to_use_compact_cands {6u}; // TODO experiment

	// a `house_type_`-oriented PoV of which cells are solved, or what candidates remain.
	// non-memory-concerned implementation.
	template<Order O> requires(is_order_compiled(O) && (O < order_threshold_to_use_compact_cands))
	struct CandsPov final {
	private:
		using T = Ints<O>;
		using o2xs_t = T::o2xs_t;
		using o2i_t  = T::o2i_t;
		using rmi_t  = T::o4xs_t;

		detail::Gridlike<O, O2BitArr<O>> cands_;
		HouseType house_type_;
	public:
		[[nodiscard, gnu::pure]] auto is_unsolved (const o2x_t house, const o2x_t at) const noexcept -> bool { return cands_[house, at].count() == 1u; }
		[[nodiscard, gnu::pure]] auto at_unsolved (const o2x_t house, const o2x_t at) const noexcept -> const O2BitArr<O>& { OKIIDOKU_ASSERT( is_unsolved(house, at)); return cands_[house, at]; }
		[[nodiscard, gnu::pure]] auto at_solved   (const o2x_t house, const o2x_t at) const noexcept -> o2x_t             { OKIIDOKU_ASSERT(!is_unsolved(house, at)); return *(cands_[house, at].first_set_bit()); }
		void gc() const noexcept {/* noop */}
	};


	// a `house_type_`-oriented PoV of which cells are solved, or what candidates remain.
	// memory-concerned implementation. some indirection overhead. see `gc` member fn.
	template<Order O> requires(is_order_compiled(O) && (O >= order_threshold_to_use_compact_cands))
	struct CandsPov final {
	private:
		using T = Ints<O>;
		using o2xs_t = T::o2xs_t;
		using o2i_t  = T::o2i_t;
		using rmi_t  = T::o4xs_t;

		// if a bit is set in `solved`, its corresponding entry in sym_or_offset is
		// a sym, and otherwise an offset to add to `offset` to index into `cands_pool_`.
		struct HouseData final {
			O2BitArr<O> solved;
			rmi_t offset;
			std::array<o2xs_t, T::O2> sym_or_offset;
		};
		std::array<HouseData, T::O2> houses_;
		std::vector<O2BitArr<O>> cands_pool_; // initial size: O4
		HouseType house_type_;
	public:
		[[nodiscard, gnu::pure]] auto is_unsolved (const o2i_t house, const o2i_t at) const noexcept -> bool;
		[[nodiscard, gnu::pure]] auto at_unsolved (const o2i_t house, const o2i_t at) const noexcept -> const O2BitArr<O>&;
		[[nodiscard, gnu::pure]] auto at_solved   (const o2i_t house, const o2i_t at) const noexcept -> o2xs_t;
		void gc() noexcept;
	};


	template<Order O> requires(is_order_compiled(O)) struct EngineImpl;
	template<Order O> requires(is_order_compiled(O))
	FindStat unwind_one_stack_frame_of_(EngineImpl<O>&) noexcept;


	struct [[nodiscard]] FindStat final {
		#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		friend FindStat unwind_one_stack_frame_of_<(O_)>(EngineImpl<(O_)>&) noexcept;
		OKIIDOKU_FOREACH_O_DO_EMIT
		#undef OKIIDOKU_FOREACH_O_EMIT
	private:
		explicit consteval FindStat(bool did_unwind, bool did_unwind_root, bool found_any) noexcept:
			did_unwind_{did_unwind}, did_unwind_root_{did_unwind_root}, found_any_{found_any} {}
	public:
		[[nodiscard, gnu::pure]] bool did_unwind()      const noexcept { return did_unwind_; }
		[[nodiscard, gnu::pure]] bool did_unwind_root() const noexcept { return did_unwind_root_; }
		[[nodiscard, gnu::pure]] bool found_any()       const noexcept { return found_any_; }
		FindStat() = delete;
		static constexpr FindStat make_null()      noexcept { return FindStat{false, false, false}; }
		static constexpr FindStat make_found_any() noexcept { return FindStat{false, false, true}; }
	private:
		static constexpr FindStat make_did_unwind_guess() noexcept { return FindStat{true, false, false}; }
		static constexpr FindStat make_did_unwind_root()  noexcept { return FindStat{true, true, false}; }

		// TODO try using bit fields and measure differences.
		bool did_unwind_;
		bool did_unwind_root_;
		bool found_any_;
	};


	template<Order O> requires(is_order_compiled(O))
	struct CandsPovs final {
	private:
		using T = Ints<O>;
		using o2i_t = T::o2i_t;
		using o4i_t = T::o4i_t;
		using rmi_t = T::o4xs_t;

		// Invariant: all POVs must be consistent with one another at all times.
		HouseTypeMap<CandsPov<O>> pov_cell_major_;
		HouseTypeMap<CandsPov<O>> pov_sym_major_;
		o4i_t num_unsolved_;

	public:
		[[nodiscard, gnu::pure]]
		auto num_unsolved() const noexcept { return num_unsolved_; }

		[[nodiscard, gnu::pure]]
		const CandsPov<O>& pov(const CellOrSym, const HouseType, const o2i_t house) const noexcept;

		FindStat rule_out_rmi_sym(o4x_t rmi, o2x_t sym) noexcept;
		// cell has only one cand sym:
		//  for all neighbouring cell, remove that sym.
		//  for that sym in that house(s), solve that cell.
	};
}
#endif
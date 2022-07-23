#ifndef HPP_OKIIDOKU__PRINT_2D
#define HPP_OKIIDOKU__PRINT_2D

#include <okiidoku/grid.hpp>
#include <okiidoku/detail/contract.hpp>

#include <iosfwd>
#include <functional> // function
#include <tuple>
#include <span>

namespace okiidoku {

	using print_2d_grid_view = std::function<
		visitor::int_ts::o2is_t (visitor::int_ts::o4xs_t rmi)
	>;

	// contract: each grid view's domain is [0, O4), and range is [0, O2].
	// Note: tradeoff: fewer template instantiations for slightly worse perf.
	OKIIDOKU_EXPORT void print_2d_base(
		Order O,
		std::ostream&,
		rng_seed_t,
		std::span<const print_2d_grid_view>
	) noexcept;


	namespace mono {
		template<Order O, std::same_as<Grid<O>>... Gs> requires(is_order_compiled(O))
		void print_2d(std::ostream& os, rng_seed_t rng_seed, const Gs&... grids) noexcept {
			static_assert(sizeof...(grids) > 0);
			const auto printers {std::to_array<print_2d_grid_view>({
				[&](const visitor::int_ts::o4xs_t rmi){
					return static_cast<visitor::int_ts::o2is_t>(grids.at_rmi(rmi));
				}...,
			})};
			return print_2d_base(O, os, rng_seed, printers);
		}

		// Note: I like how the initializer list param doesn't have to be the last
		// one to get partial template argument deduction, but I don't know how to
		// avoid internally using a vector instead of array with this approach.
		/* template<Order O> requires(is_order_compiled(O))
		void print_2d(
			std::ostream& os,
			std::initializer_list<std::reference_wrapper<const Grid<O>>> grids,
			rng_seed_t rng_seed
		) noexcept {
			std::vector<print_2d_grid_view> printers;
			printers.reserve(grids.size());
			for (const auto& grid : grids) {
				printers.emplace_back([&](const visitor::int_ts::o4xs_t rmi){
					return grid.get().at_rmi(rmi);
				});
			}
			return okiidoku::print_2d(os, O, std::span<const print_2d_grid_view>(printers), rng_seed);
		} */
	}


	namespace visitor {
		// does nothing if not all the grids have the same grid-order.
		template<std::same_as<Grid>... Gs>
		void print_2d(std::ostream& os, const rng_seed_t rng_seed, const Gs&... grids) noexcept {
			static_assert(sizeof...(grids) > 0);
			// if (sizeof...(grids) == 0) [[unlikely]] { return; }
			const auto tup {std::forward_as_tuple(grids...)};
			if (!(... && (std::get<0>(tup).get_mono_order() == grids.get_mono_order()))) [[unlikely]] { return; }

			switch (std::get<0>(tup).get_mono_order()) {
			#define OKIIDOKU_FOR_COMPILED_O(O_) \
			case O_: return mono::print_2d<O_>(os, rng_seed, (grids.template unchecked_get_mono_exact<O_>())...);
			OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
			#undef OKIIDOKU_FOR_COMPILED_O
			}
			OKIIDOKU_CONTRACT_USE(false); // std::unreachable
		}
	}
}
#endif
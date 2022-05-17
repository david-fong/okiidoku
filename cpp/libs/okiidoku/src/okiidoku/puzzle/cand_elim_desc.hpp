#ifndef HPP_OKIIDOKU__PUZZLE__CAND_ELIM_DESC
#define HPP_OKIIDOKU__PUZZLE__CAND_ELIM_DESC

#include <okiidoku/ints.hpp>

#include <variant>
#include <memory>
#include <type_traits>

namespace okiidoku::mono::detail::cell_major_deductive_solver {

	namespace cand_elim_desc {
		// enum class TypeId : unsigned char {
		// 	cell_requires_symbol,
		// 	symbol_requires_cell,
		// 	locked_candidate,
		// 	cells_require_symbols,
		// 	symbols_require_cells,
		// };
		constexpr unsigned heap_if_large_times_ptr {2};
		template <class T>
		using HeapIfLarge = std::conditional_t<
			(sizeof(T) > (heap_if_large_times_ptr * sizeof(std::unique_ptr<T>))),
			std::unique_ptr<T>,
			T
		>;
		namespace types {
			template<Order O> requires(is_order_compiled(O))
			class CellRequiresSymbol final {
				typename Ints<O>::o4x_smol_t rmi;
				typename Ints<O>::o2x_smol_t val;
			};
			template<Order O> requires(is_order_compiled(O))
			class SymbolRequiresCell final {
				typename Ints<O>::o4x_smol_t rmi;
				typename Ints<O>::o2x_smol_t val;
				HouseType house_type;
			};
			template<Order O> requires(is_order_compiled(O))
			class LockedCandidate final {
			};
			template<Order O> requires(is_order_compiled(O))
			class CellsRequireSymbols final {
				HouseMask<O> house_cells;
				typename Ints<O>::o2x_smol_t house;
				HouseType house_type;
			};
			template<Order O> requires(is_order_compiled(O))
			class SymbolsRequireCells final {
				HouseMask<O> syms;
				typename Ints<O>::o2x_smol_t house;
				HouseType house_type;
			};
		}
		template<class... Ts>
		using TypesVariantMapHelper = std::variant<HeapIfLarge<Ts>...>;

		template<Order O> requires(is_order_compiled(O))
		using types_variant_t = TypesVariantMapHelper<
			types::CellRequiresSymbol<O>,
			types::SymbolRequiresCell<O>,
			types::LockedCandidate<O>,
			types::CellsRequireSymbols<O>,
			types::SymbolsRequireCells<O>
		>;
	}

	// TODO.asap or just design so that instead of a unified queue of any of the types, have a separate queue for each type.
	//  can make an abstraction over such a "separate-lane" queue to expose easy things like get-count-over-all-lanes.
	//  in that case, we don't need the heapify helper, since we don't need to worry about padding-related wastage.
	template<Order O> requires(is_order_compiled(O))
	struct OKIIDOKU_NO_EXPORT CandElimDesc final {
		cand_elim_desc::types_variant_t<O> variant_;
	};
}
#endif
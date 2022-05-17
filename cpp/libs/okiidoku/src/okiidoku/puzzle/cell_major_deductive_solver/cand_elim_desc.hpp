#ifndef HPP_OKIIDOKU__PUZZLE__CELL_MAJOR_DEDUCTIVE_SOLVER__CAND_ELIM_DESC
#define HPP_OKIIDOKU__PUZZLE__CELL_MAJOR_DEDUCTIVE_SOLVER__CAND_ELIM_DESC

#include <okiidoku/ints.hpp>

#include <variant>
#include <memory>
#include <type_traits>

namespace okiidoku::mono::detail::cell_major_deductive_solver {

	namespace cand_elim_desc {
		enum class TypeId : unsigned char {
		};
		template <class T>
		using HeapIfLarge = std::conditional_t<
			(sizeof(T) > sizeof(std::unique_ptr<T>)),
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
			};
			template<Order O> requires(is_order_compiled(O))
			class SymbolsRequireCells final {
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

	template<Order O> requires(is_order_compiled(O))
	struct OKIIDOKU_NO_EXPORT CandElimDesc final {
		cand_elim_desc::types_variant_t<O> variant_;
	};
}
#endif
#ifndef HPP_OKIIDOKU__DETAIL__ORDER_TEMPLATES
#define HPP_OKIIDOKU__DETAIL__ORDER_TEMPLATES

#include <okiidoku/config/defaults.hpp>
#include <okiidoku/detail/order_templates.macros.hpp>
#include <okiidoku/detail/export.h>

#include <array>
#include <variant>
#include <compare>

namespace okiidoku {

	using Order = unsigned;

	constexpr bool is_order_compiled(const Order O) {
		#define OKIIDOKU_FOR_COMPILED_O(O_) if (O == O_) { return true; }
		OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
		#undef OKIIDOKU_FOR_COMPILED_O
		return false;
	}

	namespace detail {
		template<Order Ignored, Order... Orders>
		struct CompiledOrdersHelper final {
			static constexpr auto make_arr() { return std::to_array({Orders...}); }
		};
	}
	// exists because my template instantiation macro has no delimiter
	// argument, so I hack this to ignore a leading comma at a usage site.
	constexpr auto compiled_orders {detail::CompiledOrdersHelper<
		/* ignored: */Order{0}
		#define OKIIDOKU_FOR_COMPILED_O(O_) , O_
		OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
		#undef OKIIDOKU_FOR_COMPILED_O
	>::make_arr()};

	constexpr Order largest_compiled_order {[]{
		Order largest {0};
		#define OKIIDOKU_FOR_COMPILED_O(O_) largest = O_;
		OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
		#undef OKIIDOKU_FOR_COMPILED_O
		return largest;
	}()};


	namespace visitor::detail {
		// exists so that OrderVariantFor can use container templates that have other
		// template parameters other than just the order.
		template<typename T>
		concept MonoToVisitorAdaptor = requires() {
			std::is_same_v<decltype(T::is_borrowtype), bool>;
			#define OKIIDOKU_FOR_COMPILED_O(O_) typename T::template type<O_>;
			OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
			#undef OKIIDOKU_FOR_COMPILED_O
		};

		/* This helper is here because there's no great way to correctly put commas
		between compiled order template instantiation entries if I want there to be
		no monostate entries in the variant. */
		template<class Ignored, class... Args>
		using VariantSkipFirstHelper = std::variant<Args...>;

		template<MonoToVisitorAdaptor Adaptor>
		using OrderVariantFor = VariantSkipFirstHelper<
			/* ignored: */void
			#define OKIIDOKU_FOR_COMPILED_O(O_) , typename Adaptor::template type<O_>
			OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
			#undef OKIIDOKU_FOR_COMPILED_O
		>;


		template<MonoToVisitorAdaptor Adaptor>
		struct ContainerBase {
		public:
			using variant_t = OrderVariantFor<Adaptor>;

			// delete the default constructor if Adaptor::is_borrowtype is true.
			ContainerBase() requires(Adaptor::is_borrowtype) = delete;

			// copy-from-mono constructor
			template<Order O>
			explicit ContainerBase(typename Adaptor::template type<O> mono_obj) noexcept: variant_(mono_obj) {}

			// default-for-order constructor.
			// If the provided order is not compiled, defaults to the lowest compiled order.
			explicit ContainerBase(const Order O) noexcept requires(
				!Adaptor::is_borrowtype
				#define OKIIDOKU_FOR_COMPILED_O(O_) \
				&& std::is_nothrow_default_constructible_v<typename Adaptor::template type<O_>>
				OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
				#undef OKIIDOKU_FOR_COMPILED_O
			): variant_([O]{
				switch (O) {
				#define OKIIDOKU_FOR_COMPILED_O(O_) \
				case O_: return variant_t(std::in_place_type<typename Adaptor::template type<O_>>);
				OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
				#undef OKIIDOKU_FOR_COMPILED_O
				default: return variant_t(); // default to the lowest compiled order.
				}
			}()) {}

			// ContainerBase(const variant_t& variant) noexcept: variant_(variant) {}
			// ContainerBase(variant_t&& variant) noexcept: variant_(std::forward<variant_t>(variant)) {}

			[[nodiscard, gnu::pure]] friend constexpr bool operator==(const ContainerBase&, const ContainerBase&) noexcept = default;

			// Note to self: no need to export these members which are public and inline
			[[nodiscard, gnu::pure]] constexpr Order get_mono_order() const noexcept { return compiled_orders[variant_.index()]; }

			// Though public, you shouldn't need to use this. The rest of the visitor
			// interface of the library wraps operations around it with nicer syntax.
			[[nodiscard, gnu::pure]]       variant_t& get_mono_variant()       noexcept { return variant_; }
			[[nodiscard, gnu::pure]] const variant_t& get_mono_variant() const noexcept { return variant_; }

			// Sugar wrapper around an unchecked dereference of `std::get_if` for the underlying variant.
			// Note: not using `std::get` since it could throw and we're going all in with the unchecked thing here.
			template<Order O> [[nodiscard, gnu::pure]]       typename Adaptor::template type<O>& unchecked_get_mono_exact()       noexcept { return *std::get_if<typename Adaptor::template type<O>>(&variant_); }
			template<Order O> [[nodiscard, gnu::pure]] const typename Adaptor::template type<O>& unchecked_get_mono_exact() const noexcept { return *std::get_if<typename Adaptor::template type<O>>(&variant_); }
		private:
			variant_t variant_;
		};

		template<MonoToVisitorAdaptor Adaptor>
		[[nodiscard, gnu::pure]] std::compare_three_way_result<typename Adaptor::template type<compiled_orders[0]>> operator<=>(
			const ContainerBase<Adaptor>& vis_a,
			const ContainerBase<Adaptor>& vis_b
		) noexcept {
			if (const auto cmp {vis_a.get_mono_order() <=> vis_b.get_mono_order()}; std::is_neq(cmp)) {
				return cmp;
			}
			switch (vis_a.get_mono_order()) {
			#define OKIIDOKU_FOR_COMPILED_O(O_) \
			case O_: return vis_a.template unchecked_get_mono_exact<O_>() <=> vis_b.template unchecked_get_mono_exact<O_>();
			OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
			#undef OKIIDOKU_FOR_COMPILED_O
			// TODO.wait std::unreachable
			default: return std::compare_three_way_result<typename Adaptor::template type<compiled_orders[0]>>::equivalent;
			}
		}
	}
}
#endif
#ifndef HPP_OKIIDOKU__ORDER_TEMPLATES
#define HPP_OKIIDOKU__ORDER_TEMPLATES

#include <okiidoku/config/defaults.hpp>
#include <okiidoku/order_templates.macros.hpp>
#include <okiidoku_export.h>

#include <array>
#include <variant>

namespace okiidoku {

	using Order = unsigned;

	consteval bool is_order_compiled(const Order O) {
		#define OKIIDOKU_FOR_COMPILED_O(O_) if (O == O_) { return true; }
		OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
		#undef OKIIDOKU_FOR_COMPILED_O
		return false;
	}

	constexpr Order largest_compiled_order {[]{
		Order largest {0};
		#define OKIIDOKU_FOR_COMPILED_O(O_) largest = O_;
		OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
		#undef OKIIDOKU_FOR_COMPILED_O
		return largest;
	}()};

	// exists so that OrderVariantFor can use container templates that have other
	// template parameters other than just the order.
	template<typename T>
	concept MonoToVisitorAdaptor = requires() {
		#define OKIIDOKU_FOR_COMPILED_O(O_) typename T::template type<O_>;
		OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
		#undef OKIIDOKU_FOR_COMPILED_O
	};

	namespace detail {
		/* This helper is here because there's no great way to correctly put commas
		between compiled order template instantiation entries if I want there to be
		no monostate entries in the variant. */
		template<class Ignored, class... Args>
		using VariantSkipFirstHelper = std::variant<Args...>;

		template<MonoToVisitorAdaptor Adaptor>
		using OrderVariantFor = VariantSkipFirstHelper<
			void
			#define OKIIDOKU_FOR_COMPILED_O(O_) , typename Adaptor::template type<O_>
			OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
			#undef OKIIDOKU_FOR_COMPILED_O
		>;

		template<Order Ignored, Order... Orders>
		struct CompiledOrdersHelper final {
			static constexpr auto make_arr() { return std::to_array({Orders...}); }
		};
	}
	constexpr auto compiled_orders {detail::CompiledOrdersHelper<
		Order{0}
		#define OKIIDOKU_FOR_COMPILED_O(O_) , O_
		OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
		#undef OKIIDOKU_FOR_COMPILED_O
	>::make_arr()};


	namespace visitor {

		template<MonoToVisitorAdaptor Adaptor>
		class OrderVariantBase {
		public:
			using variant_t = detail::OrderVariantFor<Adaptor>;

			template<Order O> constexpr OrderVariantBase(typename Adaptor::type<O> mono_obj) noexcept: variant_(mono_obj) {}
			[[nodiscard]] constexpr Order get_order() const noexcept { return compiled_orders[variant_.index()]; }
			[[nodiscard]] const variant_t& get_variant() const noexcept { return variant_; }
		private:
			variant_t variant_;
		};

		// namespace {
		// 	template<template<Order> class Fn, class... ArgTypes, Order O>
		// 	using VisitorResult = std::invoke_result_t<Fn<O>, ArgTypes...>;
		// }

		// template<template<Order O> class Fn, class... ArgTypes>
		// // requires std::is_invocable_v<Fn<>, ArgTypes...>
		// auto visitor_for_order_variant(
		// 	Fn fn, Args... args
		// ) {
			
		// 	return std::visit([](auto&&... lambda_args) -> auto { return fn(args...) }, args...);
		// }
	}
}
#endif
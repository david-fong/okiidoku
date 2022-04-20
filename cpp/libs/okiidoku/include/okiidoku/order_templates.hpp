#ifndef HPP_OKIIDOKU__ORDER_TEMPLATES
#define HPP_OKIIDOKU__ORDER_TEMPLATES

#include <okiidoku/config/defaults.hpp>
#include <okiidoku/order_templates.macros.hpp>
#include <okiidoku_export.h>

#include <variant>

namespace okiidoku {

	using Order = unsigned;

	consteval bool is_order_compiled(const Order O) {
		#define OKIIDOKU_FOR_COMPILED_O(O_) \
		if (O == O_) return true;
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

	template<typename T>
	concept MonoToVisitorAdaptor = requires() {
		#define OKIIDOKU_FOR_COMPILED_O(O_) typename T::type<O_>;
		OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
		#undef OKIIDOKU_FOR_COMPILED_O
	};

	template<MonoToVisitorAdaptor T>
	using OrderVariantFor = std::variant<
		std::monostate
		#define OKIIDOKU_FOR_COMPILED_O(O_) , typename T::type<O_>
		OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
		#undef OKIIDOKU_FOR_COMPILED_O
	>;

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
#endif
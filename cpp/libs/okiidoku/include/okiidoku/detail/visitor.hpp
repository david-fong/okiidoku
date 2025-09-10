// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef HPP_OKIIDOKU__DETAIL__VISITOR
#define HPP_OKIIDOKU__DETAIL__VISITOR

#include <okiidoku/detail/order_templates.macros.hpp>
#include <okiidoku/order.hpp>

#include <variant>
#include <compare>
#include <utility> // forward

namespace okiidoku::visitor::detail {
	// exists so that `OrderVariantFor` can use container templates that have other
	// template parameters other than just the order.
	template<typename T>
	concept MonoToVisitorAdaptor = requires(T x) {
		// requires std::same_as<decltype(T::is_borrow_type), bool>; // TODO.low why is this not working on gcc 12?
		// TODO consider creating a `dynamic_allocation_floor` constant, or a bool-returning
		//  `use_dynamic_allocation` template function, or a use_dynamic_allocation constant.
		//  the bool constant is the simplest. (I assume I wrote this b/c stack size warnings)

		#define OKIIDOKU_FOREACH_O_EMIT(O_) typename T::template type<O_>;
		OKIIDOKU_FOREACH_O_DO_EMIT
		#undef OKIIDOKU_FOREACH_O_EMIT
	};

	/** \internal This helper is here because there's no great way to correctly
	put commas between compiled order template instantiation entries if I want
	there to be no monostate entries in the variant. */
	template<class Ignored, class... Args>
	using VariantSkipFirstHelper = std::variant<Args...>;

	template<MonoToVisitorAdaptor Adaptor>
	using OrderVariantFor = VariantSkipFirstHelper<
		/* ignored: */void
		#define OKIIDOKU_FOREACH_O_EMIT(O_) , typename Adaptor::template type<O_>
		OKIIDOKU_FOREACH_O_DO_EMIT
		#undef OKIIDOKU_FOREACH_O_EMIT
	>;


	template<MonoToVisitorAdaptor Adaptor>
	struct ContainerBase {
	public:
		using variant_t = OrderVariantFor<Adaptor>;

		// delete the default constructor if Adaptor::is_borrow_type is true.
		ContainerBase() requires(Adaptor::is_borrow_type) = delete;

		// move-from-mono constructor
		template<Order O>
		ContainerBase(typename Adaptor::template type<O>&& mono_obj) noexcept: variant_{std::move(mono_obj)} {}

		/// default-for-order constructor.
		/// If the provided order is not compiled, defaults to the lowest compiled order.
		explicit ContainerBase(const Order O) noexcept requires(
			!Adaptor::is_borrow_type
			#define OKIIDOKU_FOREACH_O_EMIT(O_) \
			&& std::is_nothrow_default_constructible_v<typename Adaptor::template type<O_>>
			OKIIDOKU_FOREACH_O_DO_EMIT
			#undef OKIIDOKU_FOREACH_O_EMIT
		): variant_([O]{
			switch (O) {
			#define OKIIDOKU_FOREACH_O_EMIT(O_) \
			case O_: return variant_t{std::in_place_type<typename Adaptor::template type<O_>>};
			OKIIDOKU_FOREACH_O_DO_EMIT
			#undef OKIIDOKU_FOREACH_O_EMIT
			default: return variant_t{}; // default to the lowest compiled order.
			}
		}()) {}

		// ContainerBase(const variant_t& variant) noexcept: variant_(variant) {}
		// ContainerBase(variant_t&& variant) noexcept: variant_(std::forward<variant_t>(variant)) {}

		[[nodiscard, gnu::pure]] friend constexpr bool operator==(const ContainerBase&, const ContainerBase&) noexcept = default;

		/** get the `Order` of the current value
		\internal no need to export these members which are public and inline. */
		[[nodiscard, gnu::pure]] constexpr Order get_order() const noexcept { return compiled_orders[variant_.index()]; }

		/** Though public, you shouldn't need to use this. The rest of the visitor
		interface of the library should wrap around it with nicer syntax. */
		template<class Self> [[nodiscard, gnu::pure]]
		auto&& get_underlying_variant(this Self&& self) noexcept { return std::forward<Self>(self).variant_; }

		// Sugar wrapper around an unchecked dereference of `std::get_if` for the underlying variant.
		// Note: not using `std::get` since it could throw and we're going all in with the unchecked thing here.
		template<Order O, class Self> [[nodiscard, gnu::pure]]
		auto&& unchecked_get_mono_exact(this Self&& self) noexcept {
			using T_var = typename Adaptor::template type<O>;
			OKIIDOKU_CONTRACT_USE(std::holds_alternative<T_var>(std::forward<Self>(self).variant_));
			OKIIDOKU_CONTRACT_USE(std::get_if<T_var>(&std::forward<Self>(self).variant_) != nullptr);
			return *std::get_if<T_var>(&std::forward<Self>(self).variant_);
		}
	private:
		variant_t variant_;
	};

	template<MonoToVisitorAdaptor Adaptor>
	[[nodiscard, gnu::pure]] std::compare_three_way_result_t<ContainerBase<Adaptor>> operator<=>(
		const ContainerBase<Adaptor>& vis_a,
		const ContainerBase<Adaptor>& vis_b
	) noexcept {
		if (const auto cmp {vis_a.get_order() <=> vis_b.get_order()}; std::is_neq(cmp)) {
			return cmp;
		}
		OKIIDOKU_CONTRACT_USE(vis_a.get_order() == vis_b.get_order());
		switch (vis_a.get_order()) {
		#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		case O_: { \
			return vis_a.template unchecked_get_mono_exact<O_>() <=> vis_b.template unchecked_get_mono_exact<O_>(); \
		}
		OKIIDOKU_FOREACH_O_DO_EMIT
		#undef OKIIDOKU_FOREACH_O_EMIT
		}
		OKIIDOKU_UNREACHABLE;
	}
}
#endif
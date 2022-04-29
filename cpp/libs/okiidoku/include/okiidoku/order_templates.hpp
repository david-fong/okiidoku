#ifndef HPP_OKIIDOKU__ORDER_TEMPLATES
#define HPP_OKIIDOKU__ORDER_TEMPLATES

#include <okiidoku/config/defaults.hpp>
#include <okiidoku/order_templates.macros.hpp>
#include <okiidoku_export.h>

#include <array>
#include <variant>

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
			std::is_same_v<decltype(T::is_ref), bool>;
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
		class ContainerBase {
		public:
			using variant_t = OrderVariantFor<Adaptor>;

			// copy-from-mono constructor
			template<Order O>
			explicit ContainerBase(typename Adaptor::type<O> mono_obj) noexcept: variant_(mono_obj) {}

			// TODO.low this will only work for zero-argument constructors. so far that's fine.
			explicit ContainerBase(const Order O) noexcept requires(!Adaptor::is_ref): variant_() {
				switch (O) {
				#define OKIIDOKU_FOR_COMPILED_O(O_) \
				case O_: variant_.template emplace<typename Adaptor::type<O_>>(); break;
				OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
				#undef OKIIDOKU_FOR_COMPILED_O
				}
			}

			ContainerBase(const variant_t& variant) noexcept: variant_(variant) {}
			ContainerBase(variant_t&& variant) noexcept: variant_(std::forward<variant_t>(variant)) {}

			[[nodiscard, gnu::pure]] constexpr Order get_mono_order() const noexcept { return compiled_orders[variant_.index()]; }

			// Though public, you shouldn't need to use this. The rest of the visitor
			// interface of the library wraps operations around it with nicer syntax.
			// TODO.high.asap see if the library ELF gets smaller if these are made non-exported. or see if there are things in the ELF that can be forced to be hidden.
			[[nodiscard, gnu::pure]]       variant_t& get_mono_variant()       noexcept { return variant_; }
			[[nodiscard, gnu::pure]] const variant_t& get_mono_variant() const noexcept { return variant_; }
		private:
			variant_t variant_;
		};
	}
}
#endif
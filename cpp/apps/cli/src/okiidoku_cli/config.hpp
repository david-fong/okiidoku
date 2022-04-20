#ifndef HPP_OKIIDOKU_CLI__CONFIG
#define HPP_OKIIDOKU_CLI__CONFIG

#include <okiidoku_cli/enum.hpp>
#include <okiidoku/order_templates.hpp>

#include <string_view>
#include <array>

namespace okiidoku::cli {

	class Config {
	public:
		[[nodiscard, gnu::pure]] verbosity::E verbosity() const noexcept { return verbosity_; };
		void verbosity(verbosity::E);
		void verbosity(std::string_view);

		[[nodiscard, gnu::pure]] Order order() const noexcept { return order_; }
		void order(Order) noexcept;
		void order(std::string_view);

		[[nodiscard, gnu::pure]] bool canonicalize() const noexcept { return canonicalize_; };
		void canonicalize(bool);
		void canonicalize(std::string_view);

	private:
		Order order_;
		verbosity::E verbosity_ {verbosity::E::full};
		bool canonicalize_ {false};
	};
}
#endif
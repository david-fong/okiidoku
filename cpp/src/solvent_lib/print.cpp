#include <solvent_lib/print.hpp>
#include <solvent_util/ansi.hpp>

#include <iostream>


namespace solvent::lib::print {

	//
	void value(std::ostream& os, const solvent::Order O, const uint8_t value) {
		if (value == O*O) [[unlikely]] {
			os << ' ';
		} else {
			if (O < 4) {
				os << static_cast<unsigned>(value);
			} else if (O == 5) {
				os << static_cast<char>('a' + value);
			} else {
				(value < 10)
					? os << static_cast<unsigned>(value)
					: os << static_cast<char>('a' + value - 10);
			}
		}
	}


	void serial(std::ostream& os, const solvent::Order O, grid_t grid_view) {
		for (unsigned coord = 0; coord < O*O*O*O; coord++) {
			print::value(os, O, grid_view(coord));
		}
	}


	void pretty(std::ostream& os, const Order O, std::vector<grid_t> const& grid_views) {
		namespace ansi = solvent::util::ansi;
		using ord2_t = std::uint16_t;

		const auto blk_row_sep_str = [O]() {
			std::string vbar = " " + std::string((((O * (O + 1)) + 1) * 2 - 1), '-');
			for (unsigned i = 0; i <= O; i++) {
				vbar[(2 * (O + 1) * i) + 1] = '+';
			}
			return vbar;
		}();

		const bool is_pretty = &os == &std::cout;
		auto print_blk_row_sep_str = [&]() mutable {
			if (!is_pretty) return;
			os << '\n' << blk_row_sep_str;
			for (unsigned i = 1; i < grid_views.size(); i++) {
				os << "   " << blk_row_sep_str;
			}
		};

		os << ansi::DIM.ON;
		for (ord2_t row = 0; row < O*O; row++) {
			if (row % O == 0) {
				print_blk_row_sep_str();
			}
			os << '\n';
			for (unsigned grid_i = 0; grid_i < grid_views.size(); grid_i++) {
				for (ord2_t col = 0; col < O*O; col++) {
					if (is_pretty && (col % O) == 0) { os << ansi::DIM.ON << " |" << ansi::DIM.OFF; }
					os << ' ';
					print::value(os, O, grid_views[grid_i](row * O*O + col));
				}
				if (is_pretty) { os << ansi::DIM.ON << " |"; }
				if (grid_i != grid_views.size() - 1) {
					os << "   ";
				}
			}
		}
		print_blk_row_sep_str();
		os << ansi::DIM.OFF;
	}


	template<Order O>
	void serial(std::ostream& os, typename gen::Generator<O>::GenResult const& gen_result) {
		const grid_t grid([&gen_result](uint16_t coord) { return gen_result.grid[coord]; });
		print::serial(os, O, grid);
	}


	template<Order O>
	void pretty(std::ostream& os, typename gen::Generator<O>::GenResult const& gen_result) {
		const std::vector<grid_t> grids = {
			grid_t([&gen_result](uint16_t coord) { return gen_result.grid[coord]; })
		};
		print::pretty(os, O, grids);
	}


	#define SOLVENT_TEMPL_TEMPL(O_) \
		template void serial<O_>(std::ostream&, typename gen::Generator<O_>::GenResult const&); \
		template void pretty<O_>(std::ostream&, typename gen::Generator<O_>::GenResult const&);
	SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef SOLVENT_TEMPL_TEMPL
}
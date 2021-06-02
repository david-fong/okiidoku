#include "./print.hpp"
#include ":/util/ansi.hpp"

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


	void serial(std::ostream& os, const solvent::Order O, grid_t grid) {
		for (unsigned coord = 0; coord < O*O*O*O; coord++) {
			print::value(os, O, grid(coord));
		}
	}


	void pretty(std::ostream& os, const Order O, std::vector<grid_t> const& grids) {
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
			for (unsigned i = 1; i < grids.size(); i++) {
				os << "   " << blk_row_sep_str;
			}
		};
		for (ord2_t row = 0; row < O*O; row++) {
			if (row % O == 0) {
				print_blk_row_sep_str();
			}
			os << '\n' << ansi::DIM.ON;
			for (unsigned grid_i = 0; grid_i < grids.size(); grid_i++) {
				for (ord2_t col = 0; col < O*O; col++) {
					if (is_pretty && (col % O) == 0) os << ansi::DIM.ON << " |" << ansi::DIM.OFF;
					print::value(os, O, grids[grid_i](row * O*O + col));
				}
				if (is_pretty) os << ansi::DIM.ON << " |";
				if (grid_i != grids.size() - 1) {
					os << "   ";
				}
			}
			if (is_pretty) os << ansi::DIM.ON << " |";
		}
		print_blk_row_sep_str();
	}


	template<Order O>
	void serial(std::ostream& os, typename gen::Generator<O>::GenResult const& gen_result) {
		std::vector grids = {
			[&gen_result](uint16_t coord) { return gen_result.grid[coord]; }
		};
		print::serial(os, O, grids);
	}


	template<Order O>
	void pretty(std::ostream& os, typename gen::Generator<O>::GenResult const& gen_result) {
		std::vector<grid_t> grids = {
			[&gen_result](uint16_t coord) { return gen_result.grid[coord]; }
		};
		print::pretty(os, O, grids);
	}
}
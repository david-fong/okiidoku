#include "./mod.hpp"
#include ":/lib/size.hpp"
#include ":/util/ansi.hpp"

#include <iostream>


template<solvent::Order O>
std::ostream& operator<<(std::ostream& os, solvent::lib::gen::Generator<O> const& g) {
	using namespace solvent::lib::gen;
	namespace ansi = solvent::util::ansi;
	using ord2_t = typename solvent::size<O>::ord2_t;

	static const auto blk_row_sep_str = []() {
		std::string vbar = " " + std::string((((O * (O + 1)) + 1) * 2 - 1), '-');
		for (unsigned i = 0; i <= O; i++) {
			vbar[(2 * (O + 1) * i) + 1] = '+';
		}
		return vbar;
	}();

	#define M_PRINT_GRID0_TILE(PRINTER_STATEMENT) {\
		for (ord2_t col = 0; col < g.O2; col++) {\
			if (is_pretty && (col % g.O1) == 0) os << ansi::DIM.ON << " |" << ansi::DIM.OFF;\
			PRINTER_STATEMENT;\
		}\
	}
	#define M_PRINT_GRID_TILE(PRINTER_STATEMENT) {\
		if (is_pretty) {\
			os << ansi::DIM.ON << " |"/* ; << ansi::DIM.OFF */\
			os << GRID_SEP;\
			M_PRINT_GRID0_TILE(PRINTER_STATEMENT)}\
		}

	const bool is_pretty = &os == &std::cout;
	const auto print_blk_row_sep_str = [&](){
		if (!is_pretty) return;
		os << '\n' << ansi::DIM.ON;
		os << g.blk_row_sep_str;
		os << GRID_SEP << g.blk_row_sep_str;
		os << ansi::DIM.OFF;
	};
	for (ord2_t row = 0; row < g.O2; row++) {
		if (row % g.O1 == 0) {
			print_blk_row_sep_str();
		}
		os << '\n';
		// Tile content:
		#define _M_index ((row * g.O2) + col)
		M_PRINT_GRID0_TILE(os << ' ' << g.values_[_M_index])
		M_PRINT_GRID_TILE(g.print_shaded_backtrack_stat(g.backtracks_[_M_index]))
		// M_PRINT_GRID_TILE(os << std::setw(2) << values_[coord].next_try_index)
		// M_PRINT_GRID_TILE(os << ' ' << val_try_order_[row][col])
		#undef _M_index
		if (is_pretty) os << ansi::DIM.ON << " |" << ansi::DIM.OFF;
	}
	print_blk_row_sep_str();
	#undef M_PRINT_GRID_TILE
	#undef M_PRINT_GRID0_TILE
	return os;
}


namespace solvent::lib::gen {

	namespace ansi = solvent::util::ansi;

	struct MyNumpunct : std::numpunct<char> {
		std::string do_grouping(void) const {
			return "\03";
		}
	};

	// std::string shaded_backtrack_stat(backtrack_t);

	std::string shaded_backtrack_stat(const long out_of, const long count) {
		const unsigned int relative_intensity
			= static_cast<double>(count - 1)
			* ansi::BLOCK_CHARS.size()
			/ out_of;
		return (count == 0) ? " " : ansi::BLOCK_CHARS[relative_intensity];
	}

	friend std::ostream& operator<<(std::ostream& os, Tile const& t) noexcept {
		static_assert(O1 <= 6, "I haven't yet decided how to translate for orders > 6.");
		if (t.is_clear()) [[unlikely]] {
			return os << ' ';
		} else {
			if constexpr (O1 < 4) {
				return os << (int)t.value;
			} else if constexpr (O1 == 5) {
				return os << static_cast<char>('a' + t.value);
			} else {
				return (t.value < 10)
					? os << static_cast<unsigned>(t.value)
					: os << static_cast<char>('a' + t.value - 10);
			}
		}
	}

	// Output formatting:
	if (is_pretty) {
		os.imbue(std::locale(os.getloc(), new MyNumpunct()));
	}
	os.precision(3);
	os << std::fixed;


	// Prints to std::cout and the output file if it exists.
	void print(void) const;
	void print_simple(void) const; // No newlines included.
	void print_msg_bar(std::string const&, unsigned int, char = '=') const;
	void print_msg_bar(std::string const&, char = '=') const;


	template<Order O>
	void Generator<O>::print(void) const {
		if (&os != &std::cout) {
			std::cout << *this;
		}
		os << *this;
	}


	template<Order O>
	void Generator<O>::print_simple(void) const {
		auto const helper = [this](std::ostream& os, const bool is_pretty){
			const bool do_dim = is_pretty && (gen_result_.get_exit_status() != ExitStatus::Ok);
			if (do_dim) os << ansi::DIM.ON;
			for (auto const& t : values_) {
				os << t;
			}
			if (do_dim) os << ansi::DIM.OFF;
		};
		if (&os == &std::cout) {
			helper(std::cout, true);
		} else {
			helper(std::cout, true);
			helper(os, false);
		}
	}


	template<Order O>
	void Generator<O>::print_msg_bar(
		std::string const& msg,
		unsigned bar_length,
		const char fill_char
	) const {
		if (bar_length < msg.length() + 8) {
			bar_length = msg.length() + 8;
		}
		std::string bar(bar_length, fill_char);
		if (!msg.empty()) {
			bar.replace(4, msg.length(), msg);
			bar.at(3) = ' ';
			bar.at(4 + msg.length()) = ' ';
		}
		os << '\n' <<bar;
	}


	template<Order O>
	void Generator<O>::print_msg_bar(std::string const& msg, const char fill_char) const {
		const unsigned grid_bar_length = (is_pretty)
			? ((O2 + O1 + 1) * 2)
			: (O2 * 2);
		constexpr unsigned num_grids = 2u;
		unsigned all_bar_length = (num_grids * grid_bar_length);
		if (num_grids > 1) all_bar_length += (num_grids - 1) * GRID_SEP.length();
		return print_msg_bar(msg, all_bar_length + 1, fill_char);
	}

	static constexpr unsigned STATS_WIDTH = (0.4 * O2) + 4;
}
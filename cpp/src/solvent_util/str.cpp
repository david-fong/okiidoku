#include <solvent_util/str.hpp>
#include <iostream>

namespace solvent::util::str {

	void print_msg_bar(
		const std::string& msg,
		unsigned bar_length,
		const std::string fill_char
	) {
		if (bar_length < msg.length() + 8) {
			bar_length = msg.length() + 8;
		}
		std::cout << '\n';
		if (msg.length()) {
			for (unsigned i = 0; i < 3; i++) { std::cout << fill_char; }
			std::cout << ' ' << msg << ' ';
			for (unsigned i = msg.length() + 5; i < bar_length; i++) { std::cout << fill_char; }
		} else {
			for (unsigned i = 0; i < bar_length; i++) { std::cout << fill_char; }
		}
	}


	void print_msg_bar(const std::string& msg, const std::string fill_char) {
		return print_msg_bar(msg, 64, fill_char);
	}
}
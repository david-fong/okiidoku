// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef HPP_OKIIDOKU_CLI_UTILS_CONSOLE_SETUP
#define HPP_OKIIDOKU_CLI_UTILS_CONSOLE_SETUP

#include <string>
#include <locale> // numpunct

namespace okiidoku::util {

	class MyNumPunct final : public std::numpunct<char> {
	public: void set_grouping(char grouping) noexcept { grouping_.front() = grouping; }
	protected: [[nodiscard, gnu::pure]] std::string do_grouping() const noexcept override { return grouping_; }
	private: std::string grouping_ {"\003"};
	};

	/**
	Use this to setup console apps made by this project.

	On Windows, this enables VT sequences and changes the input and
	output codepages to UTF-8.
	*/
	[[gnu::cold]] void setup_console();
}
#endif
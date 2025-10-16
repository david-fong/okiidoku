// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#include <okiidoku_cli_utils/console_setup.hpp>

// #include <ios>      // ios_base::sync_with_stdio
#include <iostream> // cout
#include <cstdlib>  // atexit

#ifdef _WIN32
#include <windows.h>
#include <optional>
#endif

// https://docs.microsoft.com/en-us/windows/console/classic-vs-vt#exceptions-for-using-windows-console-apis

namespace okiidoku::util {

	namespace {
		#ifdef _WIN32
		constinit std::optional<DWORD> old_con_mode            {std::nullopt};
		constinit std::optional<UINT>  old_con_input_codepage  {std::nullopt};
		constinit std::optional<UINT>  old_con_output_codepage {std::nullopt};
		#endif

		void restore_console_config() {
			#ifdef _WIN32
			if (old_con_mode) { SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), old_con_mode.value()); }
			if (old_con_input_codepage) { SetConsoleCP(old_con_input_codepage.value()); }
			if (old_con_output_codepage) { SetConsoleOutputCP(old_con_output_codepage.value()); }
			#endif
		}
	}

	void setup_console() {
		// libokiidoku specifies this as safe:
		std::ios_base::sync_with_stdio(false);
		auto* numpunct {new MyNumPunct};
		std::cout.imbue(std::locale{std::cout.getloc(), numpunct});

		#ifdef _WIN32
		{
			DWORD con_mode;
			GetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), &con_mode);
			old_con_mode = con_mode;
			con_mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
			SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), con_mode);
		} {
			old_con_input_codepage = GetConsoleCP();
			SetConsoleCP(CP_UTF8);
		} {
			old_con_output_codepage = GetConsoleOutputCP();
			SetConsoleOutputCP(CP_UTF8);
		}
		#endif

		if (std::atexit(&restore_console_config) != 0) {
			restore_console_config(); // just undo the console things immediately.
		}
	}
}
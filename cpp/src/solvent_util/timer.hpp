#ifndef HPP_SOLVENT_UTIL__TIMER
#define HPP_SOLVENT_UTIL__TIMER

#include <chrono>

namespace solvent::util {
	//
	struct Timer {
		using time_point = std::chrono::steady_clock::time_point;

		struct Elapsed {
			double proc_seconds;
			double wall_seconds;
		};

		Elapsed read_elapsed() const noexcept {
			using namespace std::chrono;
			return Elapsed {
				.proc_seconds = (static_cast<double>(std::clock() - proc_clock_start_) / CLOCKS_PER_SEC),
				.wall_seconds = (static_cast<double>(
					duration_cast<microseconds>(steady_clock::now() - wall_clock_start_).count()
				) / 1'000'000),
			};
		}

		time_point wall_clock_start_ = std::chrono::steady_clock::now();
		std::clock_t proc_clock_start_ = std::clock();
	};
}
#endif
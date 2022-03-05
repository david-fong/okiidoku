#include "solvent_lib/gen/batch.hpp"
#include "solvent_util/str.hpp"

#include <iostream>
#include <iomanip>
#include <string>

namespace solvent::lib::gen::batch {

	void BatchReport::print(std::ostream& os, const Order O) const {
		static const std::string throughput_bar_str("-------------------------");
		static const std::string table_separator =
		"\n├─────────────┼────────────┼───────────────┼───────────────┤";
		static const std::string table_header =
		"\n│     max     │  marginal  │   marginal    │      net      │"
		"\n│  dead ends  │    oks     │  average ops  │  average ops  │";

		const auto prev_fmtflags = os.flags();
		os << table_separator
			<< table_header
			<< table_separator
			<< std::fixed << std::setprecision(2);

		const auto& best_sample = max_dead_end_samples[max_dead_end_samples_best_i];
		for (const auto& sample : max_dead_end_samples) {

			// max_dead_ends:
			if (O <= 4) {
				os << "\n│" << std::setw(11) << sample.max_dead_ends;
			} else {
				os << "\n│" << std::setw(10) << (static_cast<double>(sample.max_dead_ends) / 1'000.0) << 'K';
			}

			// marginal_oks:
			os << "  │";
			if (sample.marginal_oks == 0) { os << util::str::dim.on; }
			os << std::setw(10) << sample.marginal_oks;
			if (sample.marginal_oks == 0) { os << util::str::dim.off; }

			// marginal_average_ops:
			os << "  │";
			if (sample.marginal_oks == 0) { os << util::str::dim.on; }
			os << std::setw(12);
			if (sample.marginal_average_ops.has_value()) {
				os << (sample.marginal_average_ops.value() / ((O < 5) ? 1 : 1000));
			} else {
				os << " ";
			}
			os << ((O < 5) ? ' ' : 'K');
			if (sample.marginal_oks == 0) { os << util::str::dim.off; }

			// net_average_ops:
			os << "  │";
			if (sample.marginal_oks == 0) { os << util::str::dim.on; }
			os << std::setw(12);
			if (sample.net_average_ops.has_value()) {
				os << (sample.net_average_ops.value() / ((O < 5) ? 1 : 1000));
			} else {
				os << " ";
			}
			os << ((O < 5) ? ' ' : 'K');
			if (sample.marginal_oks == 0) { os << util::str::dim.off; }

			os << "  │";

			// Print a bar to visualize throughput relative to that
			// of the best. Note visual exaggeration via exponents
			// (the exponent value was chosen by taste / visual feel)
			const std::size_t bar_length = (best_sample.net_average_ops.has_value() && sample.net_average_ops.has_value())
				? static_cast<std::size_t>(static_cast<double>(throughput_bar_str.length())
					/ (sample.net_average_ops.value() / best_sample.net_average_ops.value())
				)
				: 0;
			if (&sample != &best_sample) os << util::str::dim.on;
			os << ' ' << throughput_bar_str.substr(0, bar_length);
			if (&sample != &best_sample) os << util::str::dim.off;
		}
		os << table_separator;
		if (total_oks < max_dead_end_samples.size() * gen::batch::BatchReport::recommended_oks_per_sample) {
			os << "\nexercise caution against small datasets!" << std::endl;
		}
		os.flags(prev_fmtflags);
	}
}
#ifndef HPP_SOLVENT__MORPH__REL_PROB
#define HPP_SOLVENT__MORPH__REL_PROB

#include "solvent/order.hpp"
#include "solvent_util/math.hpp"
#include <array>
#include <cmath>

namespace solvent::lib::morph {

	template<Order O>
	struct RelCountProb final {
		static constexpr unsigned O1 = O;
		static constexpr unsigned O2 = O*O;
			
		/** The probability of values A and B being in the same atom within a
		block is `p(n) = 2/(o+1)` (simplified from `2(o-1)/(o^2-1)`). The
		probability of this ocurring k times in a grid is given by a binomial
		distribution B(o^2, 2/(o+1)). */
		static constexpr std::array<double, O2+1> ALL = [](){
			std::array<double, O2+1> _;
			for (unsigned i = 0; i < O2+1; ++i) {
				_[i] = (static_cast<double>(n_choose_r(O2, i)) * (1<<i) * std::pow(O1-1, O2-i)) / std::pow(O1+1, O2); }
			return _;
		}();
		static constexpr double ALL_E = O2 * (2.0/(O+1));

		static constexpr std::array<double, O2+1> POLAR = [](){
			std::array<double, O2+1> _;
			for (unsigned i = 0; i < O2+1; ++i) {
				_[i] = (static_cast<double>(n_choose_r(O2, i)) * std::pow(O1, O2-i)) / std::pow(O1+1, O2); }
			return _;
		}();
		static constexpr double polar_e = O2 * (1.0/(O+1));
	};
}
#endif
#ifndef HPP_SOLVENT_LIB__MORPH__CANON
#define HPP_SOLVENT_LIB__MORPH__CANON

#include <solvent_lib/size.hpp>
#include <solvent_util/math.hpp>
#include <array>
#include <cmath>

namespace solvent::lib::morph {

	template<Order O>
	struct RelCountProb {
			
		/** The probability of values A and B being in the same atom within a
		block is `p(n) = 2/(o+1)` (simplified from `2(o-1)/(o^2-1)`). The
		probability of this ocurring k times in a grid is given by a binomial
		distribution B(o^2, 2/(o+1)). */
		static constexpr std::array<double, O2+1> ALL = [](){
			std::array<double, O2+1> _;
			for (unsigned i = 0; i < O2+1; i++) {
				_[i] = static_cast<double>(n_choose_r(O2, i) * (1<<i) * std::pow(O1-1, O2-i)) / std::pow(O1+1, O2); }
			return _;
		}();

		static constexpr std::array<double, O2+1> POLAR = [](){
			std::array<double, O2+1> _;
			for (unsigned i = 0; i < O2+1; i++) {
				_[i] = static_cast<double>(n_choose_r(O2, i) * std::pow(O1, O2-i)) / std::pow(O1+1, O2); }
			return _;
		}();
	};
}
#endif
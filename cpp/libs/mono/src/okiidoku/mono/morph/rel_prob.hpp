#ifndef HPP_OKIIDOKU__MONO__MORPH__REL_PROB
#define HPP_OKIIDOKU__MONO__MORPH__REL_PROB

#include <okiidoku/order.hpp>
#include <okiidoku/math.hpp>

#include <array>
#include <cmath>

namespace okiidoku::mono::morph {

	template<Order O>
	struct RelCountProb final {
		static constexpr unsigned O1 = O;
		static constexpr unsigned O2 = O*O;
			
		/** The probability of values A and B being in the same atom within a
		box is `p(n) = 2/(o+1)` (simplified from `2(o-1)/(o^2-1)`). The
		probability of this ocurring k times in a grid is given by a binomial
		distribution B(o^2, 2/(o+1)). */
		static constexpr std::array<long double, O2+1> all {[]{
			std::array<long double, O2+1> _;
			for (unsigned i {0}; i < O2+1; ++i) {
				_[i] = (
					static_cast<long double>(n_choose_r(O2, i))
					* std::pow(static_cast<long double>(   2)/(O1+1),    i)
					* std::pow(static_cast<long double>(O1-1)/(O1+1), O2-i)
				);
			}
			// TODO.mid the below fixes some times for order 3, but only some :/
			// if constexpr (O == 3) {
			// 	for (unsigned i {0}; i < O2+1; ++i) {
			// 		_[i] *= 1.0 - (static_cast<double>(O2-i) / (O2 * 64));
			// 	}
			// }
			return _;
		}()};

		static constexpr double all_e = O2 * (2.0/(O+1));

		static constexpr std::array<long double, O2+1> polar {[]{
			std::array<long double, O2+1> _;
			for (unsigned i {0}; i < O2+1; ++i) {
				_[i] = (
					static_cast<long double>(n_choose_r(O2, i))
					* std::pow(static_cast<long double>( 1)/(O1+1),    i)
					* std::pow(static_cast<long double>(O1)/(O1+1), O2-i)
				);
			}
			return _;
		}()};

		static constexpr double polar_e = O2 * (1.0/(O+1));
	};
}
#endif
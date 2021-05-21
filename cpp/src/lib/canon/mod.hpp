#ifndef HPP_SOLVENT_LIB_CANON
#define HPP_SOLVENT_LIB_CANON

#include "../size.hpp"
#include "../grid.hpp"

#include <array>

namespace solvent::lib::canon {

	template <Order O>
	class Canonicalizer final : public solvent::lib::Grid<O> {
	public:
		using occmask_t = typename size<O>::occmask_t;
		using order_t   = typename size<O>::order_t;
		using length_t  = typename size<O>::length_t;
		using area_t    = typename size<O>::area_t;
		using value_t   = typename size<O>::value_t;

		static constexpr order_t  O1 = O;
		static constexpr length_t O2 = O*O;
		static constexpr area_t   O4 = O*O*O*O;

	private:
		const std::array<value_t, O4> buf;

		void normalizeSymbolShuffling(void);
	};
}

#endif
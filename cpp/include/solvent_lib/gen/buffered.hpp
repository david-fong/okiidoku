#ifndef HPP_SOLVENT_LIB__GEN_BUFFERED
#define HPP_SOLVENT_LIB__GEN_BUFFERED

#include <solvent_lib/gen/mod.hpp>

namespace solvent::lib::gen::buf {


	/** Doing a batch of generations first before processing the results
	*/
	template<Order O>
	class BufferedGenerator final {
	 public:
		static constexpr unsigned DEFAULT_BUFFERING[]{ 0, 1,
			/*2*/1000,
			/*3*/1000,
			/*4*/500,  // cautiously space conservative
			/*5*/1,    // because it's so slow already
			1, 1, 1, 1, 1
		};
		struct Params final {
			gen::Params gen_params;
			unsigned buffer_size = DEFAULT_BUFFERING;
			bool only_count_oks;

		};
		
	 private:
		Generator<O> gen_{};
		
	};
}
#endif
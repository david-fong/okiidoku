#ifndef HPP_SOLVENT_LIB_TOOLKIT
#define HPP_SOLVENT_LIB_TOOLKIT

#include <solvent_lib/gen/batch.hpp>
#include <solvent_lib/gen/mod.hpp>
#include <solvent_lib/grid.hpp>
#include <solvent_lib/size.hpp>

// #include <iosfwd>
#include <array>
#include <string>
#include <optional>


namespace solvent::lib::toolkit {

	class Toolkit final {
	 public:
		// void canonicalize();
		gen::GenResult gen(Order O, gen::Params);
		gen::GenResult gen_continue_prev(Order O);
		void gen_batch(Order O, gen::batch::Params);

	 private:
		#define SOLVENT_TEMPL_TEMPL(O_) \
			gen::Generator<O_> gen ## O_ ## _;
		SOLVENT_INSTANTIATE_ORDER_TEMPLATES
		#undef SOLVENT_TEMPL_TEMPL
	};
}
#endif
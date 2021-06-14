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
		void gen(gen::Params);
		void gen_continue_prev();
		void gen_batch(gen::batch::Params);

	 private:
		Order O = 4;
		#define SOLVENT_TEMPL_TEMPL(O_) \
			gen::Generator<O_> gen##O_;
		SOLVENT_INSTANTIATE_ORDER_TEMPLATES
		#undef SOLVENT_TEMPL_TEMPL
	};
}
#endif
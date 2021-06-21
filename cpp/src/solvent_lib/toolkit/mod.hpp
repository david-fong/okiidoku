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
		Toolkit(Order);
		void set_order(Order);

		// void canonicalize();
		gen::GenResult gen(gen::Params);
		gen::GenResult gen_continue_prev();
		void gen_batch(gen::batch::Params);

	 private:
		Order O;
		union generator_union_t {
		#define SOLVENT_TEMPL_TEMPL(O_) \
			gen::Generator<O_> o ## O_;
		SOLVENT_INSTANTIATE_ORDER_TEMPLATES
		#undef SOLVENT_TEMPL_TEMPL
		} gen_;
	};
}
#endif
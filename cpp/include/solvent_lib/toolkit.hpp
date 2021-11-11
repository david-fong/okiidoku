#ifndef HPP_SOLVENT_LIB__TOOLKIT
#define HPP_SOLVENT_LIB__TOOLKIT

#include <solvent_lib/gen/batch.hpp>
// #include <solvent_lib/equiv/canon.hpp>
#include <solvent_lib/gen/mod.hpp>
#include <solvent_lib/grid.hpp>
#include <solvent_lib/size.hpp>

#include <iosfwd>

namespace solvent::lib {

	class Toolkit final {
	 public:
		Toolkit(Order);
		void set_order(Order);

		gen::GenResult gen(gen::Params);
		gen::GenResult gen_continue_prev();

		// Note: commented out since currently not in use (and not very useful either).
		// void gen_batch(gen::batch::Params);

		// Note: commented out since currently not in use.
		// void canonicalize(std::vector<std::uint_fast8_t>) const;

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
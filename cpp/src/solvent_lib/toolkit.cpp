#include <solvent_lib/toolkit.hpp>
#include <solvent_lib/print.hpp>

#include <iostream>

namespace solvent::lib {

	Toolkit::Toolkit(Order O):
		gen_([O]() -> generator_union_t {
			switch (O) {
			 #define SOLVENT_TEMPL_TEMPL(O_) \
				case O_: return generator_union_t { .o ## O_ = gen::Generator<O_>() };
			 SOLVENT_INSTANTIATE_ORDER_TEMPLATES
			 #undef SOLVENT_TEMPL_TEMPL

			 default: return generator_union_t {
				.SOLVENT_TEMPL_UNION_DEFAULT(o) = gen::Generator<SOLVENT_DEFAULT_ORDER>()
			 };
			}
		}())
	{
		set_order(O);
	}


	void Toolkit::set_order(const Order O) {
		this->O = O;
		switch (O) {
		#define SOLVENT_TEMPL_TEMPL(O_) \
			case O_: { \
				gen_.o ## O_ = gen::Generator<O_>(); break; \
			}
		SOLVENT_INSTANTIATE_ORDER_TEMPLATES
		#undef SOLVENT_TEMPL_TEMPL
		}
	}


	gen::GenResult Toolkit::gen(gen::Params params) {
		switch (O) {
		#define SOLVENT_TEMPL_TEMPL(O_) \
			case O_: return gen_.o ## O_(params);
		SOLVENT_INSTANTIATE_ORDER_TEMPLATES
		#undef SOLVENT_TEMPL_TEMPL
		}
		return gen::GenResult {}; // never
	}


	gen::GenResult Toolkit::gen_continue_prev() {
		switch (O) {
		#define SOLVENT_TEMPL_TEMPL(O_) \
			case O_: return gen_.o ## O_.continue_prev();
		SOLVENT_INSTANTIATE_ORDER_TEMPLATES
		#undef SOLVENT_TEMPL_TEMPL
		}
		return gen::GenResult {}; // never
	}


	void Toolkit::gen_print_pretty(std::ostream& os) const {
		switch (O) {
		#define SOLVENT_TEMPL_TEMPL(O_) \
			case O_: gen_.o ## O_.print_pretty(os); break;
		SOLVENT_INSTANTIATE_ORDER_TEMPLATES
		#undef SOLVENT_TEMPL_TEMPL
		}
	}


	/* void Toolkit::gen_batch(gen::batch::Params params) {
		gen::batch::batch(O, params, [](gen::GenResult gen_result){
			gen_result.print_pretty(std::cout);
		});
	} */


	/* void Toolkit::canonicalize(std::vector<std::uint_fast8_t> params) const {
		switch (O) {
		#define SOLVENT_TEMPL_TEMPL(O_) \
			case O_: equiv::canonicalize<O_>(params); break;
		SOLVENT_INSTANTIATE_ORDER_TEMPLATES
		#undef SOLVENT_TEMPL_TEMPL
		}
	} */
}
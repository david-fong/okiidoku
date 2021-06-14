#include <solvent_lib/toolkit/mod.hpp>

#include<iostream>

namespace solvent::lib::toolkit {


	gen::GenResult Toolkit::gen(const Order O, gen::Params params) {
		switch (O) {
		#define SOLVENT_TEMPL_TEMPL(O_) \
			case O_: return gen ## O_ ## _(params);
		SOLVENT_INSTANTIATE_ORDER_TEMPLATES
		#undef SOLVENT_TEMPL_TEMPL
		}
		return gen::GenResult {}; // never
	}


	gen::GenResult Toolkit::gen_continue_prev(const Order O) {
		switch (O) {
		#define SOLVENT_TEMPL_TEMPL(O_) \
			case O_: return gen ## O_ ## _.continue_prev();
		SOLVENT_INSTANTIATE_ORDER_TEMPLATES
		#undef SOLVENT_TEMPL_TEMPL
		}
		return gen::GenResult {}; // never
	}


	void Toolkit::gen_batch(const Order O, gen::batch::Params params) {
		gen::batch::batch(O, params, [](gen::GenResult gen_result){
			gen_result.print_pretty(std::cout);
		});
	}
}
#include <solvent_lib/toolkit/mod.hpp>

#include<iostream>

namespace solvent::lib::toolkit {


	void Toolkit::gen(gen::Params params) {
		switch (O) {
		#define SOLVENT_TEMPL_TEMPL(O_) \
			case O_: gen##O_(params).print_pretty(std::cout); break;
		SOLVENT_INSTANTIATE_ORDER_TEMPLATES
		#undef SOLVENT_TEMPL_TEMPL
		}
	}


	void Toolkit::gen_continue_prev() {
		switch (O) {
		#define SOLVENT_TEMPL_TEMPL(O_) \
			case O_: gen##O_.continue_prev().print_pretty(std::cout); break;
		SOLVENT_INSTANTIATE_ORDER_TEMPLATES
		#undef SOLVENT_TEMPL_TEMPL
		}
	}


	void Toolkit::gen_batch(gen::batch::Params params) {
		gen::batch::batch(O, params, [](gen::GenResult gen_result){
			gen_result.print_pretty(std::cout);
		});
	}
}
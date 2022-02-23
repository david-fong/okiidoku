#include <solvent_lib/toolkit.hpp>

// #include <iostream>

namespace solvent::lib {

	Toolkit::Toolkit(Order order):
		order_{order},
		gen_{[order]() -> generator_union_t {
			switch (order) {
			 #define M_SOLVENT_TEMPL_TEMPL(O_) \
				case O_: return generator_union_t { .o ## O_ = gen::Generator<O_>() };
			 M_SOLVENT_INSTANTIATE_ORDER_TEMPLATES
			 #undef M_SOLVENT_TEMPL_TEMPL

			 default: return generator_union_t {
				.M_SOLVENT_TEMPL_UNION_DEFAULT(o) = gen::Generator<M_SOLVENT_DEFAULT_ORDER>()
			 };
			}
		}()}
	{
	}


	void Toolkit::set_order(const Order new_order) {
		this->order_ = new_order;
		switch (new_order) {
		#define M_SOLVENT_TEMPL_TEMPL(O_) \
			case O_: { \
				gen_.o ## O_ = gen::Generator<O_>(); break; \
			}
		M_SOLVENT_INSTANTIATE_ORDER_TEMPLATES
		#undef M_SOLVENT_TEMPL_TEMPL
		}
	}


	gen::GenResult Toolkit::gen(gen::Params params) {
		switch (order_) {
		#define M_SOLVENT_TEMPL_TEMPL(O_) \
			case O_: return gen_.o ## O_(params);
		M_SOLVENT_INSTANTIATE_ORDER_TEMPLATES
		#undef M_SOLVENT_TEMPL_TEMPL
		}
		return gen::GenResult {}; // never
	}


	gen::GenResult Toolkit::gen_continue_prev() {
		switch (order_) {
		#define M_SOLVENT_TEMPL_TEMPL(O_) \
			case O_: return gen_.o ## O_.continue_prev();
		M_SOLVENT_INSTANTIATE_ORDER_TEMPLATES
		#undef M_SOLVENT_TEMPL_TEMPL
		}
		return gen::GenResult {}; // never
	}


	/* void Toolkit::gen_batch(gen::batch::Params params) {
		gen::batch::batch(order_, params, [](gen::GenResult gen_result){
			gen_result.print_pretty(std::cout);
		});
	} */


	/* void Toolkit::canonicalize(std::vector<std::uint_fast8_t> params) const {
		switch (order_) {
		#define M_SOLVENT_TEMPL_TEMPL(O_) \
			case O_: morph::canonicalize<O_>(params); break;
		M_SOLVENT_INSTANTIATE_ORDER_TEMPLATES
		#undef M_SOLVENT_TEMPL_TEMPL
		}
	} */
}
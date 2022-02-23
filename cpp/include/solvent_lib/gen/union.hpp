#ifndef HPP_SOLVENT_LIB__GEN_UNION
#define HPP_SOLVENT_LIB__GEN_UNION

#include <solvent_lib/gen/batch.hpp>
#include <solvent_lib/gen/mod.hpp>
// #include <solvent_lib/morph/canon.hpp>
#include <solvent_lib/grid.hpp>
#include <solvent_lib/size.hpp>
#include <solvent_config.hpp>

namespace solvent::lib::gen {

	class GeneratorUnion final {
	 public:
		explicit GeneratorUnion(Order);
		void set_order(Order);

		[[nodiscard]] gen::GenResult gen(gen::Params);
		[[nodiscard]] gen::GenResult gen_continue_prev();

		// Note: commented out since currently not in use (and not very useful either).
		// void gen_batch(gen::batch::Params);

		// Note: commented out since currently not in use.
		// void canonicalize(std::vector<std::uint_fast8_t>) const;

	 private:
		Order order_;

		union generator_union_t {
		#define M_SOLVENT_TEMPL_TEMPL(O_) \
			gen::Generator<O_> o ## O_;
		M_SOLVENT_INSTANTIATE_ORDER_TEMPLATES
		#undef M_SOLVENT_TEMPL_TEMPL
		} gen_;
	};
}
#endif
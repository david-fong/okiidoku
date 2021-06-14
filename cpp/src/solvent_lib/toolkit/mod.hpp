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
		void canonicalize();
		void gen(gen::Params);
		void gen_batch(gen::batch::Params);
	 private:
	};
}
#endif
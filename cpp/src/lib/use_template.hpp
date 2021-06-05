#ifndef HPP_SOLVENT_LIB_USE_TEMPLATE
#define HPP_SOLVENT_LIB_USE_TEMPLATE

#include ":/lib/grid.hpp"
#include ":/lib/print.hpp"
#include ":/lib/gen/path.hpp"
#include ":/lib/gen/mod.hpp"
#include ":/lib/gen/batch.hpp"
// #include ":/lib/canon.hpp"

//
#define SOLVENT_LIB_USE_ORDER(ORDER)\
namespace solvent::lib {\
	template class Grid<ORDER>;\
	namespace print {\
		template void serial<ORDER>(std::ostream&, typename gen::Generator<ORDER>::GenResult const&);\
		template void pretty<ORDER>(std::ostream&, typename gen::Generator<ORDER>::GenResult const&);\
	}\
	namespace gen {\
		template class Generator<ORDER>;\
		namespace batch {\
			template class ThreadFunc<ORDER>;\
			template const BatchReport batch<ORDER>(Params&, callback_t<ORDER>);\
		}\
	}\
}

#endif
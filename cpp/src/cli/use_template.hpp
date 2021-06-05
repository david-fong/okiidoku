#ifndef HPP_SOLVENT_CLI_USE_TEMPLATE
#define HPP_SOLVENT_CLI_USE_TEMPLATE

#include ":/cli/repl.hpp"
#include ":/lib/use_template.hpp"

//
#define SOLVENT_CLI_USE_ORDER(ORDER)\
SOLVENT_LIB_USE_ORDER(ORDER)\
namespace solvent::cli {\
	template class Repl<ORDER>;\
}

#endif
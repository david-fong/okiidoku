#include <string>
#include <iostream>  // cout,

// a
#define INSTANTIATE_TEMPL_TEMPL \
TEMPL_TEMPL(2)\
TEMPL_TEMPL(3)\
TEMPL_TEMPL(4)



int main(const int argc, char const *const argv[]) {
	#define TEMPL_TEMPL(ORDER) std::cout << "first: " << ORDER << std::endl;
	INSTANTIATE_TEMPL_TEMPL
	#undef TEMPL_TEMPL

	#define TEMPL_TEMPL(ORDER) std::cout << "second: " << ORDER << std::endl;
	INSTANTIATE_TEMPL_TEMPL
	#undef TEMPL_TEMPL
}

FetchContent_MakeAvailable(pybind11)
# https://github.com/pybind/cmake_example
# https://pybind11.readthedocs.io/en/stable/cmake/index.html

pybind11_add_module(okiidoku_py MODULE okiidoku.pybind.cpp)
set_target_properties(okiidoku_py PROPERTIES OUTPUT_NAME "okiidoku")
target_link_libraries(okiidoku_py
	PRIVATE okiidoku_compile_options_private
	PRIVATE okiidoku
)
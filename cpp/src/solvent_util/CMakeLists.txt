add_library(solvent_util STATIC
   console_setup.cpp
)
target_include_directories(solvent_util PRIVATE ..)
target_include_directories(solvent_util PRIVATE ../../include) # currently only used for USE_ANSI_ESC in str.hpp, which could really be deprecated/removed.
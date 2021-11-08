add_library(solvent_util STATIC
   str.cpp
)
target_include_directories(solvent_util PRIVATE ..)
target_include_directories(solvent_util PUBLIC ../../include)
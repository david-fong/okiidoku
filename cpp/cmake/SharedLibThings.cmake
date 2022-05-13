
# set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
# ^tried in last-ditch attempt to support DLLs. It didn't work.

# Alex Reinking's tutorial suggests respecting user-specified values, but
# that doesn't make intuitive sense to me as to why, so I'm not doing it.
# if (NOT DEFINED CMAKE_CXX_VISIBILITY_PRESET AND NOT DEFINED CMAKE_VISIBILITY_INLINES_HIDDEN)
#	# use strict settings unless values are overridden by the user.
	set(CMAKE_CXX_VISIBILITY_PRESET hidden)
	set(CMAKE_VISIBILITY_INLINES_HIDDEN TRUE)
# endif()
# https://gitlab.kitware.com/cmake/community/-/wikis/doc/cmake/RPATH-handling
# https://docs.conan.io/en/latest/howtos/manage_shared_libraries/rpaths.html
if(APPLE)
	set(CMAKE_INSTALL_RPATH "@executable_path/../${CMAKE_INSTALL_LIBDIR}")
else()
	set(CMAKE_INSTALL_RPATH "\$ORIGIN/../${CMAKE_INSTALL_LIBDIR}")
endif()
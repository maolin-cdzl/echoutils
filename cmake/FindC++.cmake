find_library(C++_LIBRARIES
	c++
	PATHS /usr/local/lib /usr/lib64 /usr/lib/x86_64-linux-gnu /usr/lib)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(C++
	REQUIRED_VARS C++_LIBRARIES 
)

mark_as_advanced(C++_LIBRARIES)


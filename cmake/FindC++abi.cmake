find_library(C++ABI_LIBRARIES
	c++abi
	PATHS /usr/local/lib /usr/lib64 /usr/lib/x86_64-linux-gnu /usr/lib)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(C++abi
	REQUIRED_VARS C++ABI_LIBRARIES 
)

mark_as_advanced(C++ABI_LIBRARIES)



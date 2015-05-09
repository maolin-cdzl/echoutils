find_library(UNWIND_LIBRARIES
	unwind
	PATHS /usr/local/lib /usr/lib64 /usr/lib/x86_64-linux-gnu /usr/lib)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Unwind
	REQUIRED_VARS UNWIND_LIBRARIES 
)

mark_as_advanced(UNWIND_LIBRARIES)



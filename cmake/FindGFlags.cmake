find_path(GFLAGS_INCLUDE_DIR
	gflags.h 
	PATHS /usr/local/include /usr/include
	PATH_SUFFIXES gflags google
	)

find_library(GFLAGS_LIBRARIES
	gflags
	PATHS /usr/local/lib /usr/lib64 /usr/lib/x86_64-linux-gnu /usr/lib)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GFlags
	REQUIRED_VARS GFLAGS_LIBRARIES GFLAGS_INCLUDE_DIR
)

mark_as_advanced(GFLAGS_LIBRARIES GFLAGS_INCLUDE_DIR)

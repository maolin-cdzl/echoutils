find_path(EV_INCLUDE_DIR
	ev.h 
	PATHS /usr/local/include /usr/include
	)

find_library(EV_LIBRARIES
	ev
	PATHS /usr/local/lib /usr/lib64 /usr/lib/x86_64-linux-gnu /usr/lib)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(libev
	REQUIRED_VARS EV_LIBRARIES EV_INCLUDE_DIR
)

mark_as_advanced(EV_LIBRARIES EV_INCLUDE_DIR)


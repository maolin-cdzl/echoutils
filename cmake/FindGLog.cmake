find_path(GLOG_INCLUDE_DIR
	NAMES logging.h
	PATHS /usr/local/include /usr/include
	PATH_SUFFIXES glog 
	)

find_library(GLOG_LIBRARIES
	NAMES glog
	PATHS /usr/local/lib /usr/lib64 /usr/lib/x86_64-linux-gnu /usr/lib)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GLog
	REQUIRED_VARS GLOG_LIBRARIES GLOG_INCLUDE_DIR
)

mark_as_advanced(GLOG_LIBRARIES GLOG_INCLUDE_DIR)


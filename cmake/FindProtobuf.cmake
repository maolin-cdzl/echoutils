find_path(PROTOBUF_INCLUDE_DIR
	protobuf/message.h
	PATHS /usr/local/include /usr/include
	PATH_SUFFIXES google
	)

find_library(PROTOBUF_LIBRARIES
	NAMES protobuf 
	PATHS /usr/local/lib /usr/lib64 /usr/lib/x86_64-linux-gnu /usr/lib)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Protobuf
	REQUIRED_VARS PROTOBUF_LIBRARIES PROTOBUF_INCLUDE_DIR
)

mark_as_advanced(PROTOBUF_LIBRARIES PROTOBUF_INCLUDE_DIR)


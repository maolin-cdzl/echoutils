CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)
project(echoutils)

include(CMakeProtobuf)

set(CMAKE_CXX_FLAGS "-g ${CMAKE_CXX_FLAGS}")
set(PROTO_DEF_FILES proto/echo.proto)
set(PROTOBUF_IMPORT_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/proto)

build_pb_cxx_sources(${CMAKE_CURRENT_BINARY_DIR}/include/proto ${CMAKE_CURRENT_BINARY_DIR}/src PROTO_SRCS PROTO_HDRS ${PROTO_DEF_FILES})
add_library(proto STATIC ${PROTO_SRCS} ${PROTO_HDRS})

target_include_directories(proto 
	PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/include/proto
	PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/include
	PUBLIC ${PROTOBUF_INCLUDE_DIR}
	)

find_package(Protobuf REQUIRED)

add_executable(echosvc src/server.cc src/proto.h)
target_include_directories(echosvc PRIVATE
	PRIVATE $<TARGET_PROPERTY:proto,INTERFACE_INCLUDE_DIRECTORIES>
	)
target_link_libraries(echosvc LINK_PRIVATE
	proto
	ev
	gflags
	glog
	protobuf
	)


add_executable(echocli src/client.cc src/proto.h)
target_include_directories(echocli PRIVATE
	PRIVATE $<TARGET_PROPERTY:proto,INTERFACE_INCLUDE_DIRECTORIES>
	)
target_link_libraries(echocli LINK_PRIVATE
	proto
	ev
	gflags
	glog
	protobuf
	)

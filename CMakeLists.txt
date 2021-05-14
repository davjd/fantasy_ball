cmake_minimum_required(VERSION 3.15)
set (CMAKE_CXX_STANDARD 17)

project(fantasy_ball)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(HEADER_FILES src/util.cc
                 src/curl_fetch.cc
                 src/team_fetcher.cc 
                 src/player_fetcher.cc 
                 src/postgre_sql_fetch.cc 
                 src/league_fetcher.cc
                 src/widgets/wxglade_out.cpp
                 src/account_manager.cc
)

set(LEAGUE_SERVER_SOURCES
    src/league_service_server.cc
    src/util.cc
    src/curl_fetch.cc
    src/postgre_sql_fetch.cc 
    src/league_fetcher.cc
)

set(PLAYER_TEAM_SERVER_SOURCES
    src/player_team_service_server.cc
    src/util.cc
    src/curl_fetch.cc
    src/team_fetcher.cc
    src/player_fetcher.cc
)

set(CLIENT_SOURCES
    src/fantasy_service_client.cc
)

include(FetchContent)
include_directories(src/)

FetchContent_Declare(
  gRPC
  GIT_REPOSITORY https://github.com/grpc/grpc
  GIT_TAG        v1.28.0
  )
set(FETCHCONTENT_QUIET OFF)
FetchContent_MakeAvailable(gRPC)

FetchContent_Declare(
  fmt
	GIT_REPOSITORY "https://github.com/fmtlib/fmt.git"
	GIT_TAG 7.1.3
	GIT_SHALLOW TRUE
  )
FetchContent_MakeAvailable(fmt)

# Protobuf variables.
set(_PROTOBUF_LIBPROTOBUF libprotobuf)
set(_REFLECTION grpc++_reflection)
set(_PROTOBUF_PROTOC $<TARGET_FILE:protoc>)
set(_GRPC_GRPCPP grpc++)
set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:grpc_cpp_plugin>)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/proto)
get_filename_component(league_service_proto "config/protos/league_service.proto" ABSOLUTE)
get_filename_component(league_service_proto_path "${league_service_proto}" PATH)
get_filename_component(player_team_service_proto "config/protos/player_team_service.proto" ABSOLUTE)
get_filename_component(player_team_service_proto_path "${player_team_service_proto}" PATH)
# Generate protobufs.
set(league_service_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/proto/league_service.pb.cc")
set(league_service_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/proto/league_service.pb.h")
set(league_service_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/proto/league_service.grpc.pb.cc")
set(league_service_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/proto/league_service.grpc.pb.h")
set(player_team_service_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/proto/player_team_service.pb.cc")
set(player_team_service_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/proto/player_team_service.pb.h")
set(player_team_service_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/proto/player_team_service.grpc.pb.cc")
set(player_team_service_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/proto/player_team_service.grpc.pb.h")
message(STATUS "protoc: ${_PROTOBUF_PROTOC}")
add_custom_command(
        OUTPUT "${league_service_proto_srcs}" "${league_service_proto_hdrs}" "${league_service_grpc_srcs}" "${league_service_grpc_hdrs}"
        COMMAND ${_PROTOBUF_PROTOC}
        ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}/proto"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}/proto"
        -I "${league_service_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${league_service_proto}"
        DEPENDS "${league_service_proto}")
add_custom_command(
  OUTPUT "${player_team_service_proto_srcs}" "${player_team_service_proto_hdrs}" "${player_team_service_grpc_srcs}" "${player_team_service_grpc_hdrs}"
  COMMAND ${_PROTOBUF_PROTOC}
  ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}/proto"
  --cpp_out "${CMAKE_CURRENT_BINARY_DIR}/proto"
  -I "${player_team_service_proto_path}"
  --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
  "${player_team_service_proto}"
  DEPENDS "${player_team_service_proto}")
add_library(league_service_proto_library ${league_service_proto_srcs} ${league_service_grpc_srcs})
target_include_directories(league_service_proto_library INTERFACE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(league_service_proto_library # Link this with any executable/library.
        ${_REFLECTION}
        ${_GRPC_GRPCPP}
        ${_PROTOBUF_LIBPROTOBUF})

add_library(player_team_service_proto_library ${player_team_service_proto_srcs} ${player_team_service_grpc_srcs})
target_include_directories(player_team_service_proto_library INTERFACE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(player_team_service_proto_library # Link this with any executable/library.
        ${_REFLECTION}
        ${_GRPC_GRPCPP}
        ${_PROTOBUF_LIBPROTOBUF})

set(CURL_LIBRARY "-lcurl")
find_package(CURL REQUIRED)
find_library(PQXX_LIB pqxx REQUIRED)
find_library(PQ_LIB pq REQUIRED)

find_package(wxWidgets REQUIRED COMPONENTS net core base)
include(${wxWidgets_USE_FILE})

find_package(nlohmann_json 3.2.0 REQUIRED)

# Experimental binaries for testing.
# add_executable(fantasy_ball src/experimental.cc ${HEADER_FILES})
# add_executable(fantasy_ball src/widgets/main_app.cc ${HEADER_FILES})

include_directories(${CURL_INCLUDE_DIR})
# target_link_libraries(fantasy_ball ${wxWidgets_LIBRARIES} nlohmann_json::nlohmann_json ${CURL_LIBRARIES} ${PQXX_LIB} ${PQ_LIB} grpc++ service_proto_library fmt::fmt)

add_executable(league_server ${LEAGUE_SERVER_SOURCES})
target_link_libraries(league_server nlohmann_json::nlohmann_json ${CURL_LIBRARIES} ${PQXX_LIB} ${PQ_LIB} grpc++ league_service_proto_library fmt::fmt)

add_executable(player_team_server ${PLAYER_TEAM_SERVER_SOURCES})
target_link_libraries(player_team_server nlohmann_json::nlohmann_json ${CURL_LIBRARIES} ${PQXX_LIB} ${PQ_LIB} grpc++ player_team_service_proto_library fmt::fmt)

add_executable(league_client src/widgets/main_app.cc ${CLIENT_SOURCES} ${HEADER_FILES})
target_link_libraries(league_client ${wxWidgets_LIBRARIES} nlohmann_json::nlohmann_json ${CURL_LIBRARIES} ${PQXX_LIB} ${PQ_LIB} grpc++ league_service_proto_library fmt::fmt)
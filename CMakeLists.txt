cmake_minimum_required(VERSION 3.7)
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
)

include_directories(src/)

set(CURL_LIBRARY "-lcurl")
find_package(CURL REQUIRED)

find_package(wxWidgets REQUIRED COMPONENTS net core base)
include(${wxWidgets_USE_FILE})

find_package(nlohmann_json 3.2.0 REQUIRED)

add_executable(fantasy_ball src/main.cc ${HEADER_FILES})

include_directories(${CURL_INCLUDE_DIR})
target_link_libraries(fantasy_ball ${wxWidgets_LIBRARIES} nlohmann_json::nlohmann_json ${CURL_LIBRARIES})
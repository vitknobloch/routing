cmake_minimum_required(VERSION 3.10)
project(Heuristic)

if(${PROJECT_IS_TOP_LEVEL})
    set(JSON_BuildTests OFF CACHE INTERNAL "")
    add_subdirectory(lib/json)
endif ()


if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O2")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

include_directories(include)

set(COMMON
    src/common/optal_comms.cpp
    src/common/portfolio.cpp
    src/common/serializer.cpp
    src/common/routing_instance.cpp
    src/common/tsplib_loader.cpp
)

set(TSP

)

add_executable(Heuristic
        src/main.cpp
        ${COMMON}
        ${TSP}
)

target_link_libraries(Heuristic PRIVATE nlohmann_json::nlohmann_json)
target_include_directories(Heuristic PRIVATE lib/json/include/nlohmann)
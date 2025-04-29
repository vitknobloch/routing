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
    src/common/solomon_loader.cpp
    src/common/logger.cpp
    src/common/SA_schedule_functions.cpp
)

set(HEURISTIC_FRAMEWORK
    src/heuristic_framework/callbacks.cpp
    src/heuristic_framework/stochastic_local_search.cpp
    src/heuristic_framework/population.cpp
    src/heuristic_framework/tournament_selection.cpp
    src/heuristic_framework/truncation_replacement.cpp
    src/heuristic_framework/genetic_algorithm.cpp
    src/heuristic_framework/population_stochastic_ranking.cpp
    src/heuristic_framework/stochastic_ranking.cpp
    src/heuristic_framework/exhaustive_local_search.cpp
    src/heuristic_framework/basic_schedule_memory.cpp
    src/heuristic_framework/simulated_annealing_basic.cpp
    src/heuristic_framework/memetic.cpp
    src/heuristic_framework/simulated_annealing_schedule.cpp
    src/heuristic_framework/simulated_annealing.cpp
)

set(TSP
    src/TSP/setup.cpp
    src/TSP/tsp_individual.cpp
    src/TSP/tsp_local_search.cpp
    src/TSP/tsp_mutation_2opt.cpp
    src/TSP/tsp_pmx_crossover.cpp
    src/TSP/tsp_genetic_algorithm.cpp
    src/TSP/tsp_individual_structured.cpp
    src/TSP/tsp_mutation_double_bridge.cpp
    src/TSP/tsp_neighborhood.cpp
    src/TSP/tsp_memetic.cpp
    src/TSP/tsp_pmx_crossover_structured.cpp
    src/TSP/tsp_exhaustive_local_search.cpp
    src/TSP/tsp_SA_step.cpp
    src/TSP/tsp_simulated_annealing.cpp
)

set(CVRP
    src/CVRP/setup.cpp
    src/CVRP/cvrp_individual.cpp
    src/CVRP/cvrp_structured_individual.cpp
    src/CVRP/cvrp_stochastic_local_search.cpp
    src/CVRP/cvrp_mutation_random.cpp
    src/CVRP/cvrp_stochastic_ranking.cpp
    src/CVRP/cvrp_pmx_crossover.cpp
    src/CVRP/cvrp_neighborhood.cpp
    src/CVRP/cvrp_exhaustive_local_search.cpp
    src/CVRP/cvrp_memetic.cpp
    src/CVRP/cvrp_pmx_crossover_structured.cpp
    src/CVRP/cvrp_mutation_reinsert.cpp
    src/CVRP/cvrp_SA_step.cpp
    src/CVRP/cvrp_simulated_annealing.cpp
)

set(VRP-TW
    src/VRP-TW/setup.cpp
    #src/VRP-TW/vrptw_individual.cpp
    #src/VRP-TW/vrptw_mutation_random.cpp
    src/VRP-TW/vrptw_structured_individual.cpp
    src/VRP-TW/vrptw_neighborhood.cpp
    src/VRP-TW/vrptw_exhaustive_local_search.cpp
    src/VRP-TW/vrptw_SA_basic_step.cpp
    src/VRP-TW/vrptw_SA_basic_schedule.cpp
    src/VRP-TW/vrptw_SA_basic.cpp
    src/VRP-TW/vrptw_memetic.cpp
    src/VRP-TW/vrptw_pmx_crossover_structured.cpp
    src/VRP-TW/vrptw_mutation_reinsert.cpp
)

add_executable(Heuristic
    src/main.cpp
    ${COMMON}
    ${HEURISTIC_FRAMEWORK}
    ${TSP}
    ${CVRP}
    ${VRP-TW}
)

target_link_libraries(Heuristic PRIVATE nlohmann_json::nlohmann_json)
target_include_directories(Heuristic PRIVATE lib/json/include/nlohmann)
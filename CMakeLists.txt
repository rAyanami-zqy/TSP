cmake_minimum_required(VERSION 3.16)

project(TSPBranchBound
    VERSION 1.0.0
    LANGUAGES CXX
)

include(CTest)

function(tsp_apply_common_settings target)
    target_include_directories(${target}
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

    target_compile_features(${target} PRIVATE cxx_std_17)

    if(MSVC)
        target_compile_options(${target} PRIVATE /W4)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endfunction()

# All solver variants are built from the same sources. With neither feature
# macro defined, the implementation is the 2026-07-14 degree/incremental
# variant used by the main `tsp_bb` target.
function(tsp_add_solver target)
    set(options EXCLUDE_FROM_ALL MIN_EDGE FULL_ONETREE)
    cmake_parse_arguments(VARIANT "${options}" "" "" ${ARGN})

    if(VARIANT_EXCLUDE_FROM_ALL)
        add_executable(${target} EXCLUDE_FROM_ALL
            src/main.cpp
            src/TspSolver.cpp
        )
    else()
        add_executable(${target}
            src/main.cpp
            src/TspSolver.cpp
        )
    endif()

    tsp_apply_common_settings(${target})

    if(VARIANT_MIN_EDGE)
        target_compile_definitions(${target} PRIVATE TSP_BRANCH_STRATEGY_MIN_EDGE=1)
    endif()
    if(VARIANT_FULL_ONETREE)
        target_compile_definitions(${target} PRIVATE TSP_DISABLE_INCREMENTAL_ONETREE=1)
    endif()
endfunction()

# Default/main target: optimized 2026-07-20 MST implementation.
# The archived executable keeps the `tsp_bb` name under `solver/07-20-MST/`.
tsp_add_solver(tsp_bb)

# Reproducible named experiment targets. They are excluded from the default
# build; build one by name or use the aggregate `tsp_bb_variants` target.
tsp_add_solver(tsp_bb_26_07_14_deg EXCLUDE_FROM_ALL)
tsp_add_solver(tsp_bb_26_07_14_min_edge EXCLUDE_FROM_ALL MIN_EDGE)
tsp_add_solver(tsp_bb_26_07_14_full_deg EXCLUDE_FROM_ALL FULL_ONETREE)
tsp_add_solver(tsp_bb_26_07_14_full_min_edge EXCLUDE_FROM_ALL MIN_EDGE FULL_ONETREE)

add_custom_target(tsp_bb_variants
    DEPENDS
        tsp_bb_26_07_14_deg
        tsp_bb_26_07_14_min_edge
        tsp_bb_26_07_14_full_deg
        tsp_bb_26_07_14_full_min_edge
)

if(BUILD_TESTING)
    add_executable(tsp_solver_tests
        tests/TspSolverTests.cpp
        src/TspSolver.cpp
    )

    tsp_apply_common_settings(tsp_solver_tests)

    # No strategy macro: tests exercise the same degree/incremental
    # implementation as the default `tsp_bb` executable. Verification mode
    # additionally rebuilds reused 1-trees and checks the incremental result.
    target_compile_definitions(tsp_solver_tests PRIVATE
        TSP_VERIFY_INCREMENTAL_STATE=1
        TSP_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
    )
    add_dependencies(tsp_solver_tests tsp_bb)
    add_test(NAME tsp_solver_tests COMMAND tsp_solver_tests)

    add_test(NAME tsp_cli_exact
        COMMAND $<TARGET_FILE:tsp_bb>
            ${CMAKE_CURRENT_SOURCE_DIR}/examples/five-city.txt
    )
    set_tests_properties(tsp_cli_exact PROPERTIES
        PASS_REGULAR_EXPRESSION "Method: exact"
    )

    add_test(NAME tsp_cli_exact_limit_rejected
        COMMAND $<TARGET_FILE:tsp_bb>
            --exact-max-n 4
            ${CMAKE_CURRENT_SOURCE_DIR}/examples/five-city.txt
    )
    set_tests_properties(tsp_cli_exact_limit_rejected PROPERTIES
        WILL_FAIL TRUE
    )

    add_test(NAME tsp_cli_tsplib
        COMMAND $<TARGET_FILE:tsp_bb>
            ${CMAKE_CURRENT_SOURCE_DIR}/examples/tsplib/five-node-euc.tsp
    )
    set_tests_properties(tsp_cli_tsplib PROPERTIES
        PASS_REGULAR_EXPRESSION "Optimal cost: 8"
    )
endif()

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

# All solver variants are built from the same sources. With no feature macro
# defined, the implementation is the 2026-07-14 degree/incremental BP variant
# used by the main `tsp_bb` target.
function(tsp_add_solver target)
    set(options EXCLUDE_FROM_ALL MIN_EDGE FULL_ONETREE SMART_DFS)
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
    if(VARIANT_SMART_DFS)
        target_compile_definitions(${target} PRIVATE TSP_DISABLE_BP=1)
    endif()
endfunction()

# Default/main executable: 2026-07-14 degree strategy + incremental 1-tree.
tsp_add_solver(tsp_bb)

# Reproducible named experiment targets. They are excluded from the default
# build; build one by name or use the aggregate `tsp_bb_variants` target.
tsp_add_solver(tsp_bb_26_07_14_deg EXCLUDE_FROM_ALL)
tsp_add_solver(tsp_bb_26_07_14_min_edge EXCLUDE_FROM_ALL MIN_EDGE)
tsp_add_solver(tsp_bb_26_07_14_full_deg EXCLUDE_FROM_ALL FULL_ONETREE)
tsp_add_solver(tsp_bb_26_07_14_full_min_edge EXCLUDE_FROM_ALL MIN_EDGE FULL_ONETREE)

# Rebuilt 2026-07-02 experiment: current solver implementation with BP
# disabled and the historical smart edge selector restored. The two targets
# differ only in incremental versus complete 1-tree construction.
tsp_add_solver(tsp_bb_26_07_02 EXCLUDE_FROM_ALL SMART_DFS)
tsp_add_solver(tsp_bb_26_07_02_full EXCLUDE_FROM_ALL SMART_DFS FULL_ONETREE)

add_custom_target(tsp_bb_26_07_02_variants
    DEPENDS
        tsp_bb_26_07_02
        tsp_bb_26_07_02_full
)

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
    target_compile_definitions(tsp_solver_tests PRIVATE TSP_VERIFY_INCREMENTAL_STATE=1)
    add_dependencies(tsp_solver_tests tsp_bb)
    add_test(NAME tsp_solver_tests COMMAND tsp_solver_tests)

    # Exercise the rebuilt 07_02 smart DFS with incremental-state verification.
    # The same source-level regression suite is used so numerical, rollback,
    # sparse-graph and brute-force comparison coverage stays aligned with BP.
    add_executable(tsp_solver_26_07_02_tests
        tests/TspSolverTests.cpp
        src/TspSolver.cpp
    )
    tsp_apply_common_settings(tsp_solver_26_07_02_tests)
    target_compile_definitions(tsp_solver_26_07_02_tests PRIVATE
        TSP_DISABLE_BP=1
        TSP_VERIFY_INCREMENTAL_STATE=1
    )
    add_dependencies(tsp_solver_26_07_02_tests tsp_bb_26_07_02)
    add_test(NAME tsp_solver_26_07_02_tests COMMAND tsp_solver_26_07_02_tests)

    add_executable(tsp_solver_26_07_02_full_tests
        tests/TspSolverTests.cpp
        src/TspSolver.cpp
    )
    tsp_apply_common_settings(tsp_solver_26_07_02_full_tests)
    target_compile_definitions(tsp_solver_26_07_02_full_tests PRIVATE
        TSP_DISABLE_BP=1
        TSP_DISABLE_INCREMENTAL_ONETREE=1
    )
    add_dependencies(tsp_solver_26_07_02_full_tests tsp_bb_26_07_02_full)
    add_test(NAME tsp_solver_26_07_02_full_tests COMMAND tsp_solver_26_07_02_full_tests)

    add_test(NAME tsp_26_07_02_legacy_smart_cli
        COMMAND $<TARGET_FILE:tsp_bb_26_07_02>
            --branch-strategy smart
            ${CMAKE_CURRENT_SOURCE_DIR}/examples/five-city.txt
    )
    set_tests_properties(tsp_26_07_02_legacy_smart_cli PROPERTIES
        PASS_REGULAR_EXPRESSION "Optimal cost: 26"
    )

    add_test(NAME tsp_26_07_02_simple_rejected
        COMMAND $<TARGET_FILE:tsp_bb_26_07_02>
            --branch-strategy simple
            ${CMAKE_CURRENT_SOURCE_DIR}/examples/five-city.txt
    )
    set_tests_properties(tsp_26_07_02_simple_rejected PROPERTIES
        WILL_FAIL TRUE
    )

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

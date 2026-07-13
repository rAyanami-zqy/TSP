cmake_minimum_required(VERSION 3.16)

project(TSPBranchBound
    VERSION 1.0.0
    LANGUAGES CXX
)

include(CTest)

add_executable(tsp_bb
    src/main.cpp
    src/TspSolver.cpp
)

target_include_directories(tsp_bb
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(tsp_bb PRIVATE cxx_std_17)

if(MSVC)
    target_compile_options(tsp_bb PRIVATE /W4)
else()
    target_compile_options(tsp_bb PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(BUILD_TESTING)
    add_executable(tsp_solver_tests
        tests/TspSolverTests.cpp
        src/TspSolver.cpp
    )

    target_include_directories(tsp_solver_tests
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

    target_compile_features(tsp_solver_tests PRIVATE cxx_std_17)

    if(MSVC)
        target_compile_options(tsp_solver_tests PRIVATE /W4)
    else()
        target_compile_options(tsp_solver_tests PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    add_test(NAME tsp_solver_tests COMMAND tsp_solver_tests)
endif()

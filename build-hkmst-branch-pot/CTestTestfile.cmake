# CMake generated Testfile for 
# Source directory: /Users/a/Ayanami/code/TSP
# Build directory: /Users/a/Ayanami/code/TSP/build-hkmst-branch-pot
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(tsp_solver_tests "/Users/a/Ayanami/code/TSP/build-hkmst-branch-pot/tsp_solver_tests")
set_tests_properties(tsp_solver_tests PROPERTIES  _BACKTRACE_TRIPLES "/Users/a/Ayanami/code/TSP/CMakeLists.txt;110;add_test;/Users/a/Ayanami/code/TSP/CMakeLists.txt;0;")
add_test(tsp_cli_exact "/Users/a/Ayanami/code/TSP/build-hkmst-branch-pot/tsp_bb" "/Users/a/Ayanami/code/TSP/examples/five-city.txt")
set_tests_properties(tsp_cli_exact PROPERTIES  PASS_REGULAR_EXPRESSION "Method: exact" _BACKTRACE_TRIPLES "/Users/a/Ayanami/code/TSP/CMakeLists.txt;112;add_test;/Users/a/Ayanami/code/TSP/CMakeLists.txt;0;")
add_test(tsp_cli_exact_limit_rejected "/Users/a/Ayanami/code/TSP/build-hkmst-branch-pot/tsp_bb" "--exact-max-n" "4" "/Users/a/Ayanami/code/TSP/examples/five-city.txt")
set_tests_properties(tsp_cli_exact_limit_rejected PROPERTIES  WILL_FAIL "TRUE" _BACKTRACE_TRIPLES "/Users/a/Ayanami/code/TSP/CMakeLists.txt;120;add_test;/Users/a/Ayanami/code/TSP/CMakeLists.txt;0;")
add_test(tsp_cli_unlimited_potential_update_budget "/Users/a/Ayanami/code/TSP/build-hkmst-branch-pot/tsp_bb" "--hk-potential-update" "subtree-depth" "--hk-update-budget" "0" "/Users/a/Ayanami/code/TSP/examples/five-city.txt")
set_tests_properties(tsp_cli_unlimited_potential_update_budget PROPERTIES  PASS_REGULAR_EXPRESSION "Method: exact" _BACKTRACE_TRIPLES "/Users/a/Ayanami/code/TSP/CMakeLists.txt;129;add_test;/Users/a/Ayanami/code/TSP/CMakeLists.txt;0;")
add_test(tsp_cli_tsplib "/Users/a/Ayanami/code/TSP/build-hkmst-branch-pot/tsp_bb" "/Users/a/Ayanami/code/TSP/examples/tsplib/five-node-euc.tsp")
set_tests_properties(tsp_cli_tsplib PROPERTIES  PASS_REGULAR_EXPRESSION "Optimal cost: 8" _BACKTRACE_TRIPLES "/Users/a/Ayanami/code/TSP/CMakeLists.txt;139;add_test;/Users/a/Ayanami/code/TSP/CMakeLists.txt;0;")

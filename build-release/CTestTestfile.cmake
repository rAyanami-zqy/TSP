# CMake generated Testfile for 
# Source directory: /home/wj/code/TSP
# Build directory: /home/wj/code/TSP/build-release
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(tsp_solver_tests "/home/wj/code/TSP/build-release/tsp_solver_tests")
set_tests_properties(tsp_solver_tests PROPERTIES  _BACKTRACE_TRIPLES "/home/wj/code/TSP/CMakeLists.txt;85;add_test;/home/wj/code/TSP/CMakeLists.txt;0;")
add_test(tsp_cli_exact "/home/wj/code/TSP/build-release/tsp_bb" "/home/wj/code/TSP/examples/five-city.txt")
set_tests_properties(tsp_cli_exact PROPERTIES  PASS_REGULAR_EXPRESSION "Method: exact" _BACKTRACE_TRIPLES "/home/wj/code/TSP/CMakeLists.txt;87;add_test;/home/wj/code/TSP/CMakeLists.txt;0;")
add_test(tsp_cli_exact_limit_rejected "/home/wj/code/TSP/build-release/tsp_bb" "--exact-max-n" "4" "/home/wj/code/TSP/examples/five-city.txt")
set_tests_properties(tsp_cli_exact_limit_rejected PROPERTIES  WILL_FAIL "TRUE" _BACKTRACE_TRIPLES "/home/wj/code/TSP/CMakeLists.txt;95;add_test;/home/wj/code/TSP/CMakeLists.txt;0;")
add_test(tsp_cli_tsplib "/home/wj/code/TSP/build-release/tsp_bb" "/home/wj/code/TSP/examples/tsplib/five-node-euc.tsp")
set_tests_properties(tsp_cli_tsplib PROPERTIES  PASS_REGULAR_EXPRESSION "Optimal cost: 8" _BACKTRACE_TRIPLES "/home/wj/code/TSP/CMakeLists.txt;104;add_test;/home/wj/code/TSP/CMakeLists.txt;0;")

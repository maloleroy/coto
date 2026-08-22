cmake_minimum_required(VERSION 3.14)
project(coto)

# GoogleTest requires at least C++20
set(CMAKE_GENERATOR Ninja)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/b10fad38c4026a29ea6561ab15fc4818170d1c10.zip
  DOWNLOAD_EXTRACT_TIMESTAMP true
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
include_directories(include/)

enable_testing()
find_package(Python3 COMPONENTS Interpreter REQUIRED)

# Amplitude test
add_library(amplitude src/amplitude.cpp)
add_executable(amplitude_test test/amplitude_test.cpp)
target_link_libraries(amplitude_test amplitude powarray GTest::gtest_main)

add_library(cartesian src/absi/cartesian.cpp)
target_link_libraries(cartesian amplitude)
add_library(polar src/absi/polar.cpp)
target_link_libraries(polar amplitude)

# PowArray library
add_library(powarray src/powarray.cpp src/powmatrix.cpp)
target_link_libraries(powarray cartesian polar)

add_library(absi INTERFACE)
target_link_libraries(absi INTERFACE powarray)

# ABStract Interpretation (ABSI) tests
# Encompasses cartesian intervals test and polar intervals tests
add_executable(absi_test test/absi_test.cpp src/powarray.cpp)
target_link_libraries(absi_test powarray GTest::gtest_main)

# Diagram test
add_library(diagram STATIC src/diagram.cpp src/random-diagram.cpp)
target_link_libraries(diagram absi)
add_executable(diagram_test test/diagram_test.cpp)
target_link_libraries(diagram_test diagram GTest::gtest_main)

# Selection test
add_library(selection STATIC src/selection.cpp)
target_link_libraries(selection diagram)
add_executable(selection_test test/selection_test.cpp)
target_link_libraries(selection_test selection GTest::gtest_main)

# Gate appliers test
add_library(gateappliers STATIC src/gateappliers.cpp)
target_link_libraries(gateappliers diagram)
add_executable(gateappliers_test test/gateappliers_test.cpp)
target_link_libraries(gateappliers_test gateappliers reduction GTest::gtest_main)

# Reduction test
add_library(reduction STATIC src/reduction.cpp)
target_link_libraries(reduction selection)
add_executable(reduction_test test/reduction_test.cpp)
target_link_libraries(reduction_test reduction GTest::gtest_main)

# QASM library
FILE(GLOB QasmSources src/qasm/*.cpp)
add_library(qasm STATIC ${QasmSources})
target_link_libraries(qasm reduction gateappliers)
add_executable(qasm_test test/qasm/qasm_test.cpp)
target_link_libraries(qasm_test qasm GTest::gtest_main)

# Command-line prompt
add_executable(prompt src/prompt.cpp src/amplitude.cpp)
target_link_libraries(prompt qasm)

include(GoogleTest)
gtest_discover_tests(amplitude_test)
gtest_discover_tests(absi_test)
gtest_discover_tests(diagram_test)
gtest_discover_tests(gateappliers_test)
gtest_discover_tests(selection_test)
gtest_discover_tests(reduction_test)
gtest_discover_tests(qasm_test)

set(ANALYSIS_PYTHON ${Python3_EXECUTABLE})
if(EXISTS "${CMAKE_SOURCE_DIR}/analysis/.venv/bin/python")
  set(ANALYSIS_PYTHON "${CMAKE_SOURCE_DIR}/analysis/.venv/bin/python")
endif()
add_test(
  NAME analysis_test
  COMMAND ${CMAKE_COMMAND} -E env COTO_PROMPT=$<TARGET_FILE:prompt>
          ${ANALYSIS_PYTHON} -m unittest discover -s tests -v
)
set_tests_properties(analysis_test PROPERTIES WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/analysis)

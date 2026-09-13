# OpenHydroQual - Codegen subproject
# Builds:
#   ohq_runtime     : header-only runtime for GENERATED models (no Qt/armadillo)
#   two_pond_demo   : hand-written target model exercising the runtime
#   ohq_codegen     : the generator library (ExpressionEmitter now; analyzer/
#                     generator as they land) -- links the OHQ core
#   test_emitter    : ExpressionEmitter live test against real Expression trees
#
# Configure, e.g.:
#   cmake -S codegen -B codegen/build \
#         -DOHQLIB_DIR=/home/arash/Projects/OpenHydroQual/OHQLib/build \
#         -DOHQ_SRC=/home/arash/Projects/OpenHydroQual/aquifolium
cmake_minimum_required(VERSION 3.16)
project(ohq_codegen LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(OHQ_SRC    "${CMAKE_CURRENT_SOURCE_DIR}/../aquifolium" CACHE PATH "aquifolium source dir")
set(OHQ_ROOT   "${CMAKE_CURRENT_SOURCE_DIR}/.." CACHE PATH "OpenHydroQual repo root")
set(OHQLIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../OHQLib/build" CACHE PATH "dir with libOHQLib.so")

# --- runtime for generated models: header-only, zero external deps -----------
add_library(ohq_runtime INTERFACE)
target_include_directories(ohq_runtime INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/runtime)

# --- demo: proves the runtime compiles and solves (no core, no Qt) -----------
add_executable(two_pond_demo demo/main_two_pond.cpp)
target_link_libraries(two_pond_demo PRIVATE ohq_runtime)

# --- generator + emitter test: need the OHQ core (Qt6) -----------------------
find_package(Qt6 COMPONENTS Core QUIET)

add_library(ohq_codegen STATIC
  src/ExpressionEmitter.cpp
  src/DependencyAnalyzer.cpp
  src/CodeGenerator.cpp)
target_include_directories(ohq_codegen PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/runtime
  ${OHQ_SRC}/include
  ${OHQ_SRC}/include/GA
  ${OHQ_SRC}/include/MCMC
  ${OHQ_SRC}/src            # TimeSeries.h includes TimeSeries.hpp from src/
  ${OHQ_ROOT}
  ${OHQ_ROOT}/jsoncpp/include)
# Match the definitions libOHQLib was compiled with so the core headers agree.
target_compile_definitions(ohq_codegen PUBLIC
  Terminal_version Q_JSON_SUPPORT GSL ubuntu_version ARMA_USE_LAPACK ARMA_USE_BLAS)
if(Qt6_FOUND)
  target_link_libraries(ohq_codegen PUBLIC Qt6::Core)
endif()

if(Qt6_FOUND AND EXISTS "${OHQLIB_DIR}/libOHQLib.so")
  add_executable(test_emitter tests/test_emitter.cpp)
  target_link_libraries(test_emitter PRIVATE ohq_codegen Qt6::Core
    "${OHQLIB_DIR}/libOHQLib.so")
  set_target_properties(test_emitter PROPERTIES
    BUILD_RPATH "${OHQLIB_DIR}")
  enable_testing()
  add_test(NAME emitter COMMAND test_emitter)

  add_executable(ohq_generate src/ohq_generate.cpp)
  target_link_libraries(ohq_generate PRIVATE ohq_codegen Qt6::Core
    "${OHQLIB_DIR}/libOHQLib.so")
  set_target_properties(ohq_generate PROPERTIES BUILD_RPATH "${OHQLIB_DIR}")
else()
  message(STATUS "test_emitter skipped: need Qt6 and ${OHQLIB_DIR}/libOHQLib.so")
endif()

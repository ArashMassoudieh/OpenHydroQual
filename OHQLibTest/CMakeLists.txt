cmake_minimum_required(VERSION 3.20)

project(OHQLibTest LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Qt6 ──────────────────────────────────────────────────────────
find_package(Qt6 REQUIRED COMPONENTS Core)

# ── OHQLib ───────────────────────────────────────────────────────
# Point this to wherever OHQLib was built
set(OHQLIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../OHQLib"
    CACHE PATH "Root of OHQLib source tree")
set(OHQLIB_BUILD_DIR "${OHQLIB_DIR}/build"
    CACHE PATH "OHQLib CMake build directory")

# Headers
set(AQUIFOLIUM_ROOT "${OHQLIB_DIR}/../aquifolium")

# Import the already-built OHQLib as an imported target
add_library(OHQLib SHARED IMPORTED)
set_target_properties(OHQLib PROPERTIES
    IMPORTED_LOCATION_RELEASE
        "${OHQLIB_BUILD_DIR}/Release/OHQLib.dll"
    IMPORTED_LOCATION_DEBUG
        "${OHQLIB_BUILD_DIR}/Debug/OHQLib.dll"
    IMPORTED_LOCATION_MINSIZEREL
        "${OHQLIB_BUILD_DIR}/Release/OHQLib.dll"
    IMPORTED_LOCATION_RELWITHDEBINFO
        "${OHQLIB_BUILD_DIR}/Release/OHQLib.dll"
    IMPORTED_IMPLIB_RELEASE
        "${OHQLIB_BUILD_DIR}/Release/OHQLib.lib"
    IMPORTED_IMPLIB_DEBUG
        "${OHQLIB_BUILD_DIR}/Debug/OHQLib.lib"
    IMPORTED_IMPLIB_MINSIZEREL
        "${OHQLIB_BUILD_DIR}/Release/OHQLib.lib"
    IMPORTED_IMPLIB_RELWITHDEBINFO
        "${OHQLIB_BUILD_DIR}/Release/OHQLib.lib"
)
target_link_libraries(OHQLib INTERFACE Qt6::Core)

set(ARMADILLO_INCLUDE_DIRS "${OHQLIB_DIR}/../include"
    CACHE PATH "Path to Armadillo headers")

set(GSL_INCLUDE_DIRS "C:/Projects/vcpkg/installed/x64-windows/include"
    CACHE PATH "Path to GSL headers")

target_include_directories(OHQLib INTERFACE
    "${AQUIFOLIUM_ROOT}/include"
    "${AQUIFOLIUM_ROOT}/include/GA"
    "${AQUIFOLIUM_ROOT}/include/MCMC"
    "${AQUIFOLIUM_ROOT}/src"
    "${OHQLIB_DIR}/../"
    "${OHQLIB_DIR}/../jsoncpp/include"
    "${ARMADILLO_INCLUDE_DIRS}"
    "${GSL_INCLUDE_DIRS}"
)

# ── Executable ───────────────────────────────────────────────────
add_executable(OHQLibTest main.cpp)

target_compile_definitions(OHQLibTest PRIVATE
    GSL
    Terminal_version
    Q_JSON_SUPPORT
    windows_version
    ARMA_USE_LAPACK
    ARMA_USE_BLAS
)

target_link_libraries(OHQLibTest PRIVATE
    OHQLib
    Qt6::Core
)

# ── MSVC flags ───────────────────────────────────────────────────
if(MSVC)
    target_compile_options(OHQLibTest PRIVATE /W3 /utf-8)

    set(LAPACK_DLL_DIR "${OHQLIB_DIR}/../libs/lapack-blas_lib_win64")
    set(GSL_DLL_DIR    "C:/Projects/vcpkg/installed/x64-windows/bin")
    set(QT_DLL_DIR     "C:/Qt/6.8.1/msvc2022_64/bin")
    set(OHQLIB_DLL     "${OHQLIB_BUILD_DIR}/Release/OHQLib.dll")

    add_custom_command(TARGET OHQLibTest POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${OHQLIB_DLL}"
            "$<TARGET_FILE_DIR:OHQLibTest>/OHQLib.dll"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${LAPACK_DLL_DIR}/lapack_win64_MT.dll"
            "$<TARGET_FILE_DIR:OHQLibTest>/lapack_win64_MT.dll"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${LAPACK_DLL_DIR}/blas_win64_MT.dll"
            "$<TARGET_FILE_DIR:OHQLibTest>/blas_win64_MT.dll"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${GSL_DLL_DIR}/gsl.dll"
            "$<TARGET_FILE_DIR:OHQLibTest>/gsl.dll"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${GSL_DLL_DIR}/gslcblas.dll"
            "$<TARGET_FILE_DIR:OHQLibTest>/gslcblas.dll"
        COMMAND "${QT_DLL_DIR}/windeployqt.exe"
            "$<TARGET_FILE:OHQLibTest>"
        COMMENT "Copying runtime DLLs and running windeployqt"
    )
endif()
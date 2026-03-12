cmake_minimum_required(VERSION 3.20)

project(OHQLib
    VERSION 2.0.4
    DESCRIPTION "OpenHydroQual Aquifolium Simulation Engine"
    LANGUAGES CXX
)

# ── C++ standard (matches CONFIG += c++14) ──────────────────────────────────
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ── Qt6 Core only (QT -= gui, QT += core) ───────────────────────────────────
# Windows/MSVC:  cmake -DQt6_DIR="C:/Qt/6.8.2/msvc2022_64/lib/cmake/Qt6" ..
# Linux:         cmake ..   (Qt6 found via system or module path)
# macOS:         cmake -DCMAKE_PREFIX_PATH=/usr/local/opt/qt6 ..
find_package(Qt6 REQUIRED COMPONENTS Core)
qt_standard_project_setup()

# ── OpenMP ───────────────────────────────────────────────────────────────────
# Matches: QMAKE_CXXFLAGS += -fopenmp / /openmp, LIBS += -lgomp
find_package(OpenMP)
if(NOT OpenMP_CXX_FOUND)
    message(WARNING "OpenMP not found — building without parallel support.")
endif()

# ── LAPACK / BLAS ────────────────────────────────────────────────────────────
if(WIN32)
    # Pre-built LAPACK/BLAS libs shipped with the repo under libs/
    # Layout matches the .pro:  libs/lapack-blas_lib_win64/release|debug
    set(LAPACK_LIB_DIR_RELEASE
        "${CMAKE_CURRENT_SOURCE_DIR}/../libs/lapack-blas_lib_win64/release")
    set(LAPACK_LIB_DIR_DEBUG
        "${CMAKE_CURRENT_SOURCE_DIR}/../libs/lapack-blas_lib_win64/debug")

    add_library(lapack_win STATIC IMPORTED)
    set_target_properties(lapack_win PROPERTIES
        IMPORTED_LOCATION_RELEASE "${LAPACK_LIB_DIR_RELEASE}/lapack_win64_MT.lib"
        IMPORTED_LOCATION_DEBUG   "${LAPACK_LIB_DIR_DEBUG}/lapack_win64_MTd.lib"
    )
    add_library(blas_win STATIC IMPORTED)
    set_target_properties(blas_win PROPERTIES
        IMPORTED_LOCATION_RELEASE "${LAPACK_LIB_DIR_RELEASE}/blas_win64_MT.lib"
        IMPORTED_LOCATION_DEBUG   "${LAPACK_LIB_DIR_DEBUG}/blas_win64_MTd.lib"
    )
    set(LAPACK_LIBRARIES lapack_win blas_win)

else()
    # Linux/macOS: use system LAPACK/BLAS
    # Linux:  sudo apt install liblapack-dev libblas-dev
    # macOS:  brew install openblas  (Accelerate framework also works)
    find_package(LAPACK REQUIRED)
    find_package(BLAS  REQUIRED)
    set(LAPACK_LIBRARIES ${LAPACK_LIBRARIES} ${BLAS_LIBRARIES})
endif()

# ── Armadillo ────────────────────────────────────────────────────────────────
# Linux:   sudo apt install libarmadillo-dev
# macOS:   brew install armadillo
# Windows: header-only mode via ARMA_USE_LAPACK/BLAS + the LAPACK libs above
find_package(Armadillo QUIET)
if(NOT ARMADILLO_FOUND)
    message(STATUS "Armadillo cmake package not found — using header-only mode")
    find_path(ARMADILLO_INCLUDE_DIRS armadillo
        HINTS
            /usr/include
            /usr/local/include
            /opt/homebrew/include
            "${CMAKE_CURRENT_SOURCE_DIR}/../include"
    )
    if(NOT ARMADILLO_INCLUDE_DIRS)
        message(FATAL_ERROR
            "Armadillo headers not found.\n"
            "  Linux:   sudo apt install libarmadillo-dev\n"
            "  macOS:   brew install armadillo\n"
            "  Windows: download from http://arma.sourceforge.net and set "
            "-DARMADILLO_INCLUDE_DIRS=<path>")
    endif()
    set(ARMADILLO_LIBRARIES "")
endif()

# ── GSL (Linux only, matches: linux { LIBS += -lgsl }) ──────────────────────
if(UNIX AND NOT APPLE)
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(GSL gsl)
    endif()
    if(NOT GSL_FOUND)
        find_library(GSL_LIBRARIES NAMES gsl)
        find_path(GSL_INCLUDE_DIRS gsl/gsl_math.h)
    endif()
    if(NOT GSL_LIBRARIES)
        message(WARNING "GSL not found (sudo apt install libgsl-dev). "
                        "Building without GSL support.")
    endif()
endif()

# ── Sources (exact list from OHQLib.pro) ─────────────────────────────────────
set(AQUIFOLIUM_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../aquifolium")
set(JSONCPP_ROOT    "${CMAKE_CURRENT_SOURCE_DIR}/../jsoncpp")

set(OHQ_SOURCES
    # aquifolium core
    ${AQUIFOLIUM_ROOT}/src/Block.cpp
    ${AQUIFOLIUM_ROOT}/src/Command.cpp
    ${AQUIFOLIUM_ROOT}/src/Condition.cpp
    ${AQUIFOLIUM_ROOT}/src/ErrorHandler.cpp
    ${AQUIFOLIUM_ROOT}/src/Expression.cpp
    ${AQUIFOLIUM_ROOT}/src/Link.cpp
    ${AQUIFOLIUM_ROOT}/src/Matrix.cpp
    ${AQUIFOLIUM_ROOT}/src/Matrix_arma.cpp
    ${AQUIFOLIUM_ROOT}/src/MetaModel.cpp
    ${AQUIFOLIUM_ROOT}/src/NormalDist.cpp
    ${AQUIFOLIUM_ROOT}/src/Object.cpp
    ${AQUIFOLIUM_ROOT}/src/Objective_Function.cpp
    ${AQUIFOLIUM_ROOT}/src/Objective_Function_Set.cpp
    ${AQUIFOLIUM_ROOT}/src/Parameter.cpp
    ${AQUIFOLIUM_ROOT}/src/Parameter_Set.cpp
    ${AQUIFOLIUM_ROOT}/src/Precipitation.cpp
    ${AQUIFOLIUM_ROOT}/src/Quan.cpp
    ${AQUIFOLIUM_ROOT}/src/QuanSet.cpp
    ${AQUIFOLIUM_ROOT}/src/QuickSort.cpp
    ${AQUIFOLIUM_ROOT}/src/Rule.cpp
    ${AQUIFOLIUM_ROOT}/src/RxnParameter.cpp
    ${AQUIFOLIUM_ROOT}/src/Script.cpp
    ${AQUIFOLIUM_ROOT}/src/Source.cpp
    ${AQUIFOLIUM_ROOT}/src/System.cpp
    ${AQUIFOLIUM_ROOT}/src/Utilities.cpp
    ${AQUIFOLIUM_ROOT}/src/Vector.cpp
    ${AQUIFOLIUM_ROOT}/src/Vector_arma.cpp
    ${AQUIFOLIUM_ROOT}/src/constituent.cpp
    ${AQUIFOLIUM_ROOT}/src/observation.cpp
    ${AQUIFOLIUM_ROOT}/src/precalculatedfunction.cpp
    ${AQUIFOLIUM_ROOT}/src/reaction.cpp
    ${AQUIFOLIUM_ROOT}/src/restorepoint.cpp
    ${AQUIFOLIUM_ROOT}/src/solutionlogger.cpp
    # GA
    ${AQUIFOLIUM_ROOT}/src/GA/Binary.cpp
    ${AQUIFOLIUM_ROOT}/src/GA/Individual.cpp
    ${AQUIFOLIUM_ROOT}/src/GA/DistributionNUnif.cpp
    ${AQUIFOLIUM_ROOT}/src/GA/Distribution.cpp
    # jsoncpp (git submodule at ../jsoncpp)
    ${JSONCPP_ROOT}/src/lib_json/json_reader.cpp
    ${JSONCPP_ROOT}/src/lib_json/json_value.cpp
    ${JSONCPP_ROOT}/src/lib_json/json_writer.cpp
)

set(OHQ_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/../XString.h
    ${AQUIFOLIUM_ROOT}/include/Objective_Function.h
    ${AQUIFOLIUM_ROOT}/include/Objective_Function_Set.h
    ${AQUIFOLIUM_ROOT}/include/Precipitation.h
    ${AQUIFOLIUM_ROOT}/include/RxnParameter.h
    ${AQUIFOLIUM_ROOT}/include/constituent.h
    ${AQUIFOLIUM_ROOT}/include/observation.h
    ${AQUIFOLIUM_ROOT}/include/precalculatedfunction.h
    ${AQUIFOLIUM_ROOT}/include/solutionlogger.h
    ${AQUIFOLIUM_ROOT}/include/GA/GA.h
    ${AQUIFOLIUM_ROOT}/include/MCMC/MCMC.h
    ${AQUIFOLIUM_ROOT}/include/MCMC/MCMC.hpp
    ${AQUIFOLIUM_ROOT}/include/Utilities.h
    ${AQUIFOLIUM_ROOT}/include/restorepoint.h
    ${AQUIFOLIUM_ROOT}/include/Block.h
    ${AQUIFOLIUM_ROOT}/include/BTC.h
    ${AQUIFOLIUM_ROOT}/include/BTCSet.h
    ${AQUIFOLIUM_ROOT}/include/Expression.h
    ${AQUIFOLIUM_ROOT}/include/Link.h
    ${AQUIFOLIUM_ROOT}/include/Matrix.h
    ${AQUIFOLIUM_ROOT}/include/Matrix_arma.h
    ${AQUIFOLIUM_ROOT}/include/MetaModel.h
    ${AQUIFOLIUM_ROOT}/include/NormalDist.h
    ${AQUIFOLIUM_ROOT}/include/Object.h
    ${AQUIFOLIUM_ROOT}/include/Parameter.h
    ${AQUIFOLIUM_ROOT}/include/Parameter_Set.h
    ${AQUIFOLIUM_ROOT}/include/Quan.h
    ${AQUIFOLIUM_ROOT}/include/QuanSet.h
    ${AQUIFOLIUM_ROOT}/include/System.h
    ${AQUIFOLIUM_ROOT}/include/Vector.h
    ${AQUIFOLIUM_ROOT}/include/Vector_arma.h
    ${AQUIFOLIUM_ROOT}/include/Command.h
    ${AQUIFOLIUM_ROOT}/include/Script.h
    ${AQUIFOLIUM_ROOT}/include/reaction.h
    ${AQUIFOLIUM_ROOT}/src/BTC.hpp
    ${AQUIFOLIUM_ROOT}/src/BTCSet.hpp
)

# ── Library target ───────────────────────────────────────────────────────────
add_library(OHQLib SHARED ${OHQ_SOURCES} ${OHQ_HEADERS})

# ── Include directories ──────────────────────────────────────────────────────
target_include_directories(OHQLib
    PUBLIC
        $<BUILD_INTERFACE:${AQUIFOLIUM_ROOT}/include>
        $<BUILD_INTERFACE:${AQUIFOLIUM_ROOT}/include/GA>
        $<BUILD_INTERFACE:${AQUIFOLIUM_ROOT}/include/MCMC>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../>
        $<INSTALL_INTERFACE:include/openhydroqual>
    PRIVATE
        ${AQUIFOLIUM_ROOT}/src
        ${JSONCPP_ROOT}/include
        ${ARMADILLO_INCLUDE_DIRS}
)

if(WIN32)
    target_include_directories(OHQLib PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/../include"   # LAPACK_INCLUDE from .pro
    )
endif()

if(GSL_INCLUDE_DIRS)
    target_include_directories(OHQLib PRIVATE ${GSL_INCLUDE_DIRS})
endif()

if(APPLE)
    # Homebrew armadillo path (matches .pro: INCLUDEPATH += homebrew path)
    target_include_directories(OHQLib PRIVATE /opt/homebrew/include)
endif()

# ── Compile definitions (exact match to .pro DEFINES) ───────────────────────
target_compile_definitions(OHQLib PRIVATE
    Terminal_version
    Q_JSON_SUPPORT
)

if(WIN32)
    target_compile_definitions(OHQLib PRIVATE
        windows_version
        ARMA_USE_LAPACK
        ARMA_USE_BLAS
    )
elseif(UNIX AND NOT APPLE)
    target_compile_definitions(OHQLib PRIVATE
        ubuntu_version
        ARMA_USE_LAPACK
        ARMA_USE_BLAS
    )
    if(GSL_LIBRARIES)
        target_compile_definitions(OHQLib PRIVATE GSL)
    endif()
elseif(APPLE)
    target_compile_definitions(OHQLib PRIVATE mac_version)
endif()

if(NOT OpenMP_CXX_FOUND)
    # Matches CONFIG(debug): DEFINES += NO_OPENMP
    target_compile_definitions(OHQLib PRIVATE NO_OPENMP)
endif()

target_compile_definitions(OHQLib PRIVATE
    $<$<CONFIG:Debug>:DEBUG>
)

# ── Link libraries ───────────────────────────────────────────────────────────
target_link_libraries(OHQLib PUBLIC Qt6::Core)

if(OpenMP_CXX_FOUND)
    target_link_libraries(OHQLib PRIVATE OpenMP::OpenMP_CXX)
endif()

target_link_libraries(OHQLib PRIVATE ${LAPACK_LIBRARIES})

if(ARMADILLO_FOUND AND ARMADILLO_LIBRARIES)
    target_link_libraries(OHQLib PRIVATE ${ARMADILLO_LIBRARIES})
endif()

if(GSL_LIBRARIES)
    target_link_libraries(OHQLib PRIVATE ${GSL_LIBRARIES})
endif()

if(UNIX)
    find_package(Threads REQUIRED)
    target_link_libraries(OHQLib PRIVATE Threads::Threads)
endif()

if(APPLE)
    # Homebrew libomp (matches .pro: LIBS += /usr/local/lib/libomp.dylib)
    target_link_libraries(OHQLib PRIVATE /usr/local/lib/libomp.dylib)
endif()

# ── MSVC flags (matches win32: QMAKE_CXXFLAGS += /MP) ───────────────────────
if(MSVC)
    target_compile_options(OHQLib PRIVATE
        /MP             # parallel compilation
        /W3
        /wd4819         # code page warning
        /wd4251         # dll-interface (Qt internals)
        /wd4275         # non dll-interface base
        /utf-8
    )
    # Auto-export all symbols — avoids needing __declspec(dllexport) everywhere
    set_target_properties(OHQLib PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(OHQLib PRIVATE
        -Wall
        -Wno-unused-parameter
        -Wno-sign-compare
    )
endif()

# ── Versioning ───────────────────────────────────────────────────────────────
set_target_properties(OHQLib PROPERTIES
    VERSION   ${PROJECT_VERSION}        # libOHQLib.so.2.0.4
    SOVERSION ${PROJECT_VERSION_MAJOR}  # libOHQLib.so.2
    OUTPUT_NAME OHQLib
)

# ── Install rules ────────────────────────────────────────────────────────────
include(GNUInstallDirs)

install(TARGETS OHQLib
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}    # OHQLib.dll       (Windows)
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}    # libOHQLib.so     (Linux/macOS)
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}    # OHQLib.lib       (Windows import)
)

install(DIRECTORY ${AQUIFOLIUM_ROOT}/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/openhydroqual
    FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
)

# ── Build summary ────────────────────────────────────────────────────────────
message(STATUS "")
message(STATUS "── OHQLib ${PROJECT_VERSION} ──────────────────────────────")
message(STATUS "  Compiler     : ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "  Build type   : ${CMAKE_BUILD_TYPE}")
message(STATUS "  Qt6Core      : ${Qt6Core_VERSION} @ ${Qt6Core_DIR}")
message(STATUS "  OpenMP       : ${OpenMP_CXX_FOUND}")
message(STATUS "  Armadillo    : ${ARMADILLO_FOUND} (${ARMADILLO_VERSION_STRING})")
message(STATUS "  LAPACK       : ${LAPACK_FOUND}")
message(STATUS "  GSL          : ${GSL_FOUND}")
message(STATUS "  Install dir  : ${CMAKE_INSTALL_PREFIX}")
message(STATUS "────────────────────────────────────────────────────────────")
message(STATUS "")

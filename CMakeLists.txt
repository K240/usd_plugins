cmake_minimum_required(VERSION 3.19 FATAL_ERROR)
project(UsdAssetResolverPlugin VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(BUILD_SHARED_LIBS ON)

# --- Houdini SDK paths ---
# Try environment variable first, then auto-detect on Windows.
set(HFS $ENV{HFS} CACHE PATH "Houdini install directory")
if(NOT HFS OR NOT EXISTS "${HFS}")
    if(WIN32)
        file(GLOB _hfs_candidates "C:/Program Files/Side Effects Software/Houdini 21.*")
        list(SORT _hfs_candidates ORDER DESCENDING)
        if(_hfs_candidates)
            list(GET _hfs_candidates 0 HFS)
            set(HFS "${HFS}" CACHE PATH "Houdini install directory" FORCE)
            message(STATUS "Auto-detected HFS: ${HFS}")
        endif()
    endif()
endif()
if(NOT HFS OR NOT EXISTS "${HFS}")
    message(FATAL_ERROR
        "HFS is not set or does not exist. "
        "Please set the HFS environment variable or pass -DHFS=<path>.")
endif()

if(WIN32)
    set(AR_HOUDINI_LIB_DIR "${HFS}/custom/houdini/dsolib")
else()
    set(AR_HOUDINI_LIB_DIR "${HFS}/dsolib")
endif()
set(AR_HOUDINI_INCLUDE_DIR "${HFS}/toolkit/include")

# --- Python (needed for Boost.Python headers) ---
# Specify version: cmake .. -DPYTHON_VERSION=3.11
set(PYTHON_VERSION "3.11" CACHE STRING "Python version to use (e.g. 3.11, 3.10, 3.9)")
string(REPLACE "." "" _pyver_nodot "${PYTHON_VERSION}")
if(WIN32)
    set(_pydir "${HFS}/python${_pyver_nodot}")
    if(NOT EXISTS "${_pydir}")
        message(FATAL_ERROR
            "Python ${PYTHON_VERSION} not found at ${_pydir}. "
            "Available versions: check ${HFS}/python* directories.")
    endif()
    set(AR_PYTHON_LIB "python${_pyver_nodot}")
    set(AR_PYTHON_INCLUDE_DIR "${_pydir}/include")
    set(AR_PYTHON_LIB_DIR "${_pydir}/libs")
else()
    set(_pybin "${HFS}/python/bin/python${PYTHON_VERSION}")
    if(NOT EXISTS "${_pybin}")
        message(FATAL_ERROR
            "Python ${PYTHON_VERSION} not found at ${_pybin}. "
            "Available versions: check ${HFS}/python/bin/ directory.")
    endif()
    set(AR_PYTHON_LIB "python${PYTHON_VERSION}")
    set(AR_PYTHON_INCLUDE_DIR "${HFS}/python/include/python${PYTHON_VERSION}")
    set(AR_PYTHON_LIB_DIR "${HFS}/python/lib")
endif()
message(STATUS "Using Python ${PYTHON_VERSION} (${AR_PYTHON_LIB})")

# --- PXR library prefix ---
if(WIN32)
    set(AR_PXR_LIB_PREFIX "libpxr_")
else()
    set(AR_PXR_LIB_PREFIX "pxr_")
endif()

# --- Boost (Houdini 21.0 uses pxr/external/boost) ---
if(EXISTS "${AR_HOUDINI_INCLUDE_DIR}/pxr/external/boost")
    set(AR_BOOST_NAMESPACE pxr_boost)
    add_compile_definitions(BOOST_NS=${AR_BOOST_NAMESPACE})
else()
    message(FATAL_ERROR "pxr/external/boost not found in Houdini include dir.")
endif()

# --- Compiler flags ---
add_compile_definitions(BOOST_ALL_NO_LIB)
if(WIN32)
    add_compile_definitions(NOMINMAX)
    add_compile_options(/Zc:inline- /w)
else()
    add_compile_options(-fPIC -Wno-deprecated -Wno-deprecated-declarations)
    add_compile_definitions(_GLIBCXX_USE_CXX11_ABI=1)
endif()

# --- Link directories ---
link_directories("${AR_HOUDINI_LIB_DIR}")
link_directories("${AR_PYTHON_LIB_DIR}")

# --- Install prefix ---
set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/dist" CACHE PATH "Install directory")

# --- Subdirectories ---
add_subdirectory(src/AssetPathResolver)

# --- Codeless schemas (resource-only, no compilation) ---
install(FILES
    src/AssetInfoSchema/plugInfo.json
    src/AssetInfoSchema/generatedSchema.usda
    DESTINATION assetInfoSchema/resources
)

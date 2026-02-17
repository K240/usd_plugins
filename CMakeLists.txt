cmake_minimum_required(VERSION 3.19 FATAL_ERROR)
project(UsdAssetResolverPlugin VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(BUILD_SHARED_LIBS ON)

# --- Houdini SDK paths ---
set(HFS $ENV{HFS} CACHE PATH "Houdini install directory")
if(NOT HFS OR NOT EXISTS "${HFS}")
    message(FATAL_ERROR
        "HFS is not set or does not exist. "
        "Please set the HFS environment variable to your Houdini install path.")
endif()

if(WIN32)
    set(AR_HOUDINI_LIB_DIR "${HFS}/custom/houdini/dsolib")
else()
    set(AR_HOUDINI_LIB_DIR "${HFS}/dsolib")
endif()
set(AR_HOUDINI_INCLUDE_DIR "${HFS}/toolkit/include")

# --- Python (needed for Boost.Python headers) ---
if(WIN32)
    if(EXISTS "${HFS}/python311")
        set(AR_PYTHON_LIB python311)
        set(AR_PYTHON_INCLUDE_DIR "${HFS}/python311/include")
        set(AR_PYTHON_LIB_DIR "${HFS}/python311/libs")
    elseif(EXISTS "${HFS}/python310")
        set(AR_PYTHON_LIB python310)
        set(AR_PYTHON_INCLUDE_DIR "${HFS}/python310/include")
        set(AR_PYTHON_LIB_DIR "${HFS}/python310/libs")
    elseif(EXISTS "${HFS}/python39")
        set(AR_PYTHON_LIB python39)
        set(AR_PYTHON_INCLUDE_DIR "${HFS}/python39/include")
        set(AR_PYTHON_LIB_DIR "${HFS}/python39/libs")
    else()
        message(FATAL_ERROR "Could not find Python in Houdini install.")
    endif()
else()
    if(EXISTS "${HFS}/python/bin/python3.11")
        set(AR_PYTHON_INCLUDE_DIR "${HFS}/python/include/python3.11")
        set(AR_PYTHON_LIB_DIR "${HFS}/python/lib")
    elseif(EXISTS "${HFS}/python/bin/python3.10")
        set(AR_PYTHON_INCLUDE_DIR "${HFS}/python/include/python3.10")
        set(AR_PYTHON_LIB_DIR "${HFS}/python/lib")
    else()
        message(FATAL_ERROR "Could not find Python in Houdini install.")
    endif()
endif()

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

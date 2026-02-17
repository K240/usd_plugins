set(TARGET_NAME assetPathResolver)

# --- Sources ---
add_library(${TARGET_NAME} SHARED
    resolver.cpp
    debugCodes.cpp
)

# --- Include directories ---
target_include_directories(${TARGET_NAME} PRIVATE
    "${AR_HOUDINI_INCLUDE_DIR}"
    "${AR_PYTHON_INCLUDE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

# --- Link PXR libraries + Python ---
target_link_libraries(${TARGET_NAME} PRIVATE
    ${AR_PXR_LIB_PREFIX}ar
    ${AR_PXR_LIB_PREFIX}tf
    ${AR_PXR_LIB_PREFIX}arch
    ${AR_PXR_LIB_PREFIX}js
    ${AR_PXR_LIB_PREFIX}vt
    ${AR_PXR_LIB_PREFIX}sdf
    ${AR_PYTHON_LIB}
)

# --- Export macro ---
target_compile_definitions(${TARGET_NAME} PRIVATE
    ASSETPATHRESOLVER_EXPORTS
)

# --- Install rules ---
install(TARGETS ${TARGET_NAME}
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION lib
)

# --- plugInfo.json ---
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/plugInfo.json"
    "${CMAKE_CURRENT_BINARY_DIR}/plugInfo.json"
    COPYONLY
)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/plugInfo.json"
    DESTINATION resources
)

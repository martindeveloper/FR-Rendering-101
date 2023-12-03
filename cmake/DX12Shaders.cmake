# Find dxc.exe compiler for shader compilation
find_program(SHADER_DXC_COMPILER NAMES dxc.exe PATHS "C:/Program Files (x86)/Windows Kits/10/bin/${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}/x64")

if(NOT SHADER_DXC_COMPILER)
    message(FATAL_ERROR "[Shaders] Could not find dxc.exe compiler")
endif()

# dxc version
execute_process(COMMAND ${SHADER_DXC_COMPILER} --version OUTPUT_VARIABLE SHADER_DXC_VERSION)
message(STATUS "[Shaders] Using dxc.exe compiler version: ${SHADER_DXC_VERSION}")

# Compile HLSL shaders to DXIL
set(SHADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
set(SHADER_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/shaders")

# Set shader model to 6.0
set(SHADER_MODEL 6_0)
set(SHADER_ENTRYPOINT main)

# Compile every .hlsl file in shaders directory
file(GLOB HLSL_SOURCE_FILES "${SHADER_DIR}/*.hlsl")

# Create a dummy target to associate the post-build commands with
add_custom_target(Shaders ALL)

foreach(SHADER_HLSL_FILE ${HLSL_SOURCE_FILES})
    get_filename_component(SHADER_FILE_NAME ${SHADER_HLSL_FILE} NAME)
    set(SHADER_DXIL_FILE_NAME "${SHADER_FILE_NAME}.dxil")
    set(SHADER_DXIL_FILE "${SHADER_OUTPUT_DIR}/${SHADER_DXIL_FILE_NAME}")

    # Emit debug information
    set(SHADER_PDB_FILE_NAME "${SHADER_FILE_NAME}.pdb")
    set(SHADER_PDB_FILE "${SHADER_OUTPUT_DIR}/${SHADER_PDB_FILE_NAME}")
    set(DXC_DEBUG_ARGS -Zi -Fd "${SHADER_PDB_FILE}")

    # Shader type
    if(${SHADER_FILE_NAME} MATCHES ".*VertexShader.hlsl")
        set(SHADER_TYPE "vs")
    elseif(${SHADER_FILE_NAME} MATCHES ".*PixelShader.hlsl")
        set(SHADER_TYPE "ps")
    else()
        message(FATAL_ERROR "[Shaders] Unknown shader type for ${SHADER_FILE_NAME}")
    endif()

    set(DXC_ARGS -T ${SHADER_TYPE}_${SHADER_MODEL} -E ${SHADER_ENTRYPOINT} ${DXC_DEBUG_ARGS} -Fo ${SHADER_DXIL_FILE} ${SHADER_HLSL_FILE})

    # Add custom command to compile HLSL to DXIL as a post-build step
    add_custom_command(
        TARGET Shaders POST_BUILD
        COMMAND ${SHADER_DXC_COMPILER} ${DXC_ARGS}
        COMMAND ${CMAKE_COMMAND} -E echo "[Shaders] Compiling ${SHADER_FILE_NAME} to ${SHADER_DXIL_FILE_NAME}"
        DEPENDS ${SHADER_HLSL_FILE}
        VERBATIM
    )

    # Copy the compiled DXIL file to the target directory
    add_custom_command(
        TARGET Shaders POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SHADER_DXIL_FILE}"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders/${SHADER_DXIL_FILE_NAME}"
        COMMAND ${CMAKE_COMMAND} -E echo "[Shaders] Copying ${SHADER_DXIL_FILE_NAME} to target directory"
    )
endforeach()

if(SHADER_DXC_COMPILER)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        PROJECT_SHADER_BYTECODE_DIRECTORY="$<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders/"
        PROJECT_SHADER_MODEL="${SHADER_MODEL}"
    )
endif()

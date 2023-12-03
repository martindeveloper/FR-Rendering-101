if(DX12_AGILITY_SDK_FORCE_DISABLE)
    message(STATUS "[DX12AgilitySDK] Forcefully disabled")
    return()
endif()

set(DX12_AGILITY_SDK_ENABLED ON)
set(DX12_AGILITY_SDK_DIR "${CMAKE_SOURCE_DIR}/external/DX12AgilitySDK")
set(DX12_AGILITY_SDK_BIN_DIR "${DX12_AGILITY_SDK_DIR}/build/native/bin/x64")
set(DX12_AGILITY_SDK_INC_DIR "${DX12_AGILITY_SDK_DIR}/build/native/include")

message(STATUS "[DX12AgilitySDK] Source directory: ${DX12_AGILITY_SDK_BIN_DIR}")
message(STATUS "[DX12AgilitySDK] Include directory: ${DX12_AGILITY_SDK_INC_DIR}")

if(NOT EXISTS ${DX12_AGILITY_SDK_BIN_DIR})
    set(DX12_AGILITY_SDK_ENABLED OFF)
    message(FATAL_ERROR "[DX12AgilitySDK] Source directory does not exist")
    return()
endif()

# Post-build command to copy the DLL and PDB files
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${DX12_AGILITY_SDK_BIN_DIR}
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/external/DX12AgilitySDK"
    COMMENT "[DX12AgilitySDK] Copying DX12AgilitySDK DLL and PDB files"
)

if (DX12_AGILITY_SDK_ENABLED)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        DX12_AGILITY_SDK_ENABLED=1
        DX12_AGILITY_SDK_VERSION=611
        DX12_AGILITY_SDK_BUILD_DLL_PATH="./external/DX12AgilitySDK/"
        DX12_AGILITY_SDK_INCLUDE_PATH="${DX12_AGILITY_SDK_BIN_DIR}/build/native/include/"
    )

    target_include_directories(${PROJECT_NAME} PRIVATE
        "${DX12_AGILITY_SDK_INC_DIR}"
    )
endif()

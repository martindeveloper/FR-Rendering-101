set(PIX_WINDOWS_ROOT "C:/Program Files/Microsoft PIX")

file(GLOB PIX_WINDOWS_VERSIONS RELATIVE "${PIX_WINDOWS_ROOT}" "${PIX_WINDOWS_ROOT}/*")

# Function to compare PIX versions
function(compare_versions result a b)
    string(REPLACE "." ";" VERSION_LIST_A ${a})
    string(REPLACE "." ";" VERSION_LIST_B ${b})

    list(GET VERSION_LIST_A 0 MAJOR_A)
    list(GET VERSION_LIST_A 1 MINOR_A)
    list(GET VERSION_LIST_B 0 MAJOR_B)
    list(GET VERSION_LIST_B 1 MINOR_B)

    if (MAJOR_A GREATER MAJOR_B OR (MAJOR_A EQUAL MAJOR_B AND MINOR_A GREATER MINOR_B))
        set(${result} 1 PARENT_SCOPE)
    else()
        set(${result} -1 PARENT_SCOPE)
    endif()
endfunction()

# Sort versions to find the newest
set(PIX_WINDOWS_LATEST_VERSION "0.0")

foreach(version IN LISTS PIX_WINDOWS_VERSIONS)
    compare_versions(cmp_result ${version} ${PIX_WINDOWS_LATEST_VERSION})
    if(cmp_result EQUAL 1)
        set(PIX_WINDOWS_LATEST_VERSION ${version})
    endif()
endforeach()

# Set the variable to the newest version DLL path
set(PIX_WINDOWS_CAPTURER_DLL "${PIX_WINDOWS_ROOT}/${PIX_WINDOWS_LATEST_VERSION}/WinPixGpuCapturer.dll")

message(STATUS "[PIX-Windows] WinPixGpuCapturer.dll found at: ${PIX_WINDOWS_CAPTURER_DLL}")

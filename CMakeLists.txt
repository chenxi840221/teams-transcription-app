cmake_minimum_required(VERSION 3.20)
project(TeamsTranscriptionApp VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)

# Platform-specific settings
if(WIN32)
    add_definitions(-DWIN32_LEAN_AND_MEAN -DNOMINMAX -DUNICODE -D_UNICODE)
    add_definitions(-D_WIN32_WINNT=0x0A00)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
endif()

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/json/include
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/httplib
)

# Source files
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "src/*.h" "include/*.h")

# Resource files
if(WIN32)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/resources/app.rc")
        set(RESOURCES "resources/app.rc")
    else()
        set(RESOURCES "")
    endif()
endif()

# Create executable
add_executable(${PROJECT_NAME} WIN32 ${SOURCES} ${HEADERS} ${RESOURCES})

# Windows-specific libraries
if(WIN32)
    target_link_libraries(${PROJECT_NAME}
        kernel32 user32 gdi32 winspool comdlg32 advapi32 shell32
        ole32 oleaut32 uuid odbc32 odbccp32 winmm mmdevapi
        comctl32 shlwapi
    )
endif()

# Copy config files to output directory
configure_file(${CMAKE_SOURCE_DIR}/config/settings.json 
               ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/settings.json COPYONLY)

# Install targets
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
install(FILES config/settings.json DESTINATION bin)

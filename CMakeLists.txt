cmake_minimum_required(VERSION 3.15)
project(HapticEnvironment CXX)

# Set build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
endif()

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin/Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin/Release)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Add external dependencies
add_subdirectory(external/rpclib)
add_subdirectory(external/chai3d)

# Find OpenGL
find_package(OpenGL REQUIRED)

# Find required packages
if(UNIX AND NOT APPLE)
    # Find Linux-specific dependencies
    find_package(ALSA REQUIRED)
    find_package(X11 REQUIRED)
    find_package(Threads REQUIRED)
    
    # Find libusb-1.0
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBUSB REQUIRED libusb-1.0)
    include_directories(${LIBUSB_INCLUDE_DIRS})
endif()

# Source files
file(GLOB_RECURSE HAPTIC_SOURCES 
    "src/*.cpp"
    "src/*.h"
)

# Main executable
add_executable(${PROJECT_NAME} ${HAPTIC_SOURCES})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/external/chai3d/src
    ${CMAKE_CURRENT_SOURCE_DIR}/external/chai3d/externals/Eigen
    ${CMAKE_CURRENT_SOURCE_DIR}/external/chai3d/externals/glew/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external/chai3d/extras/GLFW/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external/rpclib/include
    ${CMAKE_CURRENT_SOURCE_DIR}/common
    ${CMAKE_CURRENT_SOURCE_DIR}/common/platform/windows
    ${OPENGL_INCLUDE_DIR}
)

# if(WIN32)
#     target_compile_definitions(${PROJECT_NAME} PRIVATE 
#         GLEW_STATIC
#         ASIO_STANDALONE
#         NOMINMAX
#         WIN32_LEAN_AND_MEAN
#         _WINSOCK_DEPRECATED_NO_WARNINGS
#         _WIN32_WINNT=0x0601
#         _WINDOWS
#         _UNICODE
#         UNICODE
#     )
# endif()

if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE 
        GLEW_STATIC
        WIN32_LEAN_AND_MEAN
        _WINDOWS
        _UNICODE
        UNICODE
    )
endif()

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    rpc
    chai3d 
    ${OPENGL_LIBRARIES}
    glfw
)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE 
        ws2_32
        Iphlpapi
        winmm
    )
elseif(UNIX AND NOT APPLE)
    # Set DHD paths
    set(DHD_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/chai3d/externals/DHD/lib/lin-${CMAKE_SYSTEM_PROCESSOR})
    set(DHD_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/chai3d/externals/DHD/include)

    # Add DHD include directory
    target_include_directories(${PROJECT_NAME} PRIVATE
        ${DHD_INCLUDE_DIR}
    )
    
    # Add DHD library
    find_library(DHD_LIBRARY NAMES drd PATHS ${DHD_LIB_DIR} NO_DEFAULT_PATH)
    if(NOT DHD_LIBRARY)
        message(FATAL_ERROR "DHD library not found in ${DHD_LIB_DIR}")
    endif()
    
    target_link_libraries(${PROJECT_NAME} PRIVATE
        ${ALSA_LIBRARIES}
        ${X11_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
        dl
        udev
        pthread
        ${DHD_LIBRARY}  # Use the full path to the library
        ${LIBUSB_LIBRARIES}  # Add libusb
    )
endif()

# Message Handler executable
file(GLOB MSG_SOURCES "messaging/MessageHandler/*.cpp" "messaging/MessageHandler/*.h")
add_executable(messageHandler ${MSG_SOURCES})
target_link_libraries(messageHandler PRIVATE rpc)
target_include_directories(messageHandler PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/external/rpclib/include
    ${CMAKE_CURRENT_SOURCE_DIR}/common
)

if(WIN32)
    target_compile_definitions(messageHandler PRIVATE 
        ASIO_STANDALONE
        WIN32_LEAN_AND_MEAN
        _WINSOCK_DEPRECATED_NO_WARNINGS
        _WIN32_WINNT=0x0601
    )
    target_link_libraries(messageHandler PRIVATE ws2_32 Iphlpapi)
endif()

# CHAI3D Demo executable
add_executable(chai3d-demo test/chai3d_demo.cpp)
target_include_directories(chai3d-demo PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/external/chai3d/src
    ${CMAKE_CURRENT_SOURCE_DIR}/external/chai3d/externals/Eigen
    ${CMAKE_CURRENT_SOURCE_DIR}/external/chai3d/externals/glew/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external/chai3d/extras/GLFW/include
    ${OPENGL_INCLUDE_DIR}
)

if(WIN32)
    target_compile_definitions(chai3d-demo PRIVATE 
        GLEW_STATIC
        WIN32_LEAN_AND_MEAN
        _WINDOWS
        _UNICODE
        UNICODE
    )
endif()

target_link_libraries(chai3d-demo PRIVATE 
    chai3d 
    ${OPENGL_LIBRARIES}
    glfw
)

if(WIN32)
    target_link_libraries(chai3d-demo PRIVATE winmm)
elseif(UNIX AND NOT APPLE)
    # Add DHD include directory
    target_include_directories(chai3d-demo PRIVATE
        ${DHD_INCLUDE_DIR}
    )
    
    target_link_libraries(chai3d-demo PRIVATE
        ${ALSA_LIBRARIES}
        ${X11_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
        dl
        udev
        pthread
        ${DHD_LIBRARY}  # Use the full path to the library
        ${LIBUSB_LIBRARIES}  # Add libusb
    )
endif() 
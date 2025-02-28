cmake_minimum_required(VERSION 3.0.0)
project(chai3d_demo)

# Find required packages
find_package(GLFW3 REQUIRED)
find_package(OpenGL REQUIRED)

# Add CHAI3D library
set(CHAI3D_DIR "${CMAKE_SOURCE_DIR}/external/chai3d")
include_directories(${CHAI3D_DIR}/src)
include_directories(${CHAI3D_DIR}/external/glew/include)
link_directories(${CHAI3D_DIR}/lib/Release/x64)

# Add executable
add_executable(chai3d-demo chai3d_demo.cpp)

# Link libraries
target_link_libraries(chai3d-demo 
    chai3d
    glfw
    ${OPENGL_LIBRARIES}
)

# Set output directory
set_target_properties(chai3d-demo
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
)

# Copy required DLLs to output directory
if(WIN32)
    add_custom_command(TARGET chai3d-demo POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CHAI3D_DIR}/bin/chai3d.dll"
        "${CMAKE_SOURCE_DIR}/bin/chai3d.dll"
    )
endif() 
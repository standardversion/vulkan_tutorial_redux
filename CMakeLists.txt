# CMakeList.txt : CMake project for vulkan_tutorial_redux, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project (vulkan_tutorial_redux VERSION 0.1 LANGUAGES CXX)
set(APP_TARGET vulkan_tutorial_redux)

# Gather source files
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS src/*.cpp src/*.h)

# Add executable
add_executable (${APP_TARGET} ${SRC_FILES})
target_include_directories(${APP_TARGET} PRIVATE src)

# C++20 and strict compiler flags
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
    target_compile_options(${APP_TARGET} PRIVATE /W4 /permissive- /std:c++20)
else()
    target_compile_options(${APP_TARGET} PRIVATE -Wall -Wextra -pedantic)
endif()

# Vulkan
find_package(Vulkan REQUIRED)
target_include_directories(${APP_TARGET} PRIVATE ${Vulkan_INCLUDE_DIRS})
target_link_libraries(${APP_TARGET} PRIVATE ${Vulkan_LIBRARIES})

# Fetch external dependencies
include(FetchContent)

# GLFW
FetchContent_Declare(
	glfw
	GIT_REPOSITORY https://github.com/glfw/glfw.git
	GIT_TAG 3.4
)
FetchContent_MakeAvailable(glfw)
target_link_libraries(${APP_TARGET} PRIVATE glfw)

# GLM
FetchContent_Declare(
	glm
	GIT_REPOSITORY https://github.com/g-truc/glm.git
	GIT_TAG bf71a834948186f4097caa076cd2663c69a10e1e #refs/tags/1.0.1
)
FetchContent_MakeAvailable(glm)
target_link_libraries(${APP_TARGET} PRIVATE glm::glm)

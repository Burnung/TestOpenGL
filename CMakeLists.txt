cmake_minimum_required(VERSION 3.5)
project(TestOPenGL)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_MODULE_PATH})
find_package(OpenGL REQUIRED)

find_package(PkgConfig REQUIRED)

#pkg_check_modules(GLFW REQUIRED glfw3)

find_package(GLFW REQUIRED)

if (GLFW_FOUND)
    message(STATUS "Glfw found")
    include_directories(${GLFW_INCLUDE_DIRS})
    link_libraries (${GLFW_LIBRARIES})
endif (GLFW_FOUND)

include_directories(
        ${OPENGL_INCLUDE_DIR}
        ${GLFW_INCLUDE_DIRS}
)

#LINK_DIRECTORIES("/usr/local/lib/")
#link_libraries(glfw)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp)
add_executable(TestOPenGL ${SOURCE_FILES})

target_link_libraries(TestOPenGL ${OPENGL_gl_LIBRARY} ${GLFW_LIBRARIES})

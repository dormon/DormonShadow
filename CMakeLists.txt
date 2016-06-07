cmake_minimum_required(VERSION 3.0.0)

PROJECT(AntTest)

set(INCLUDES
  include/VariableRegisterManipulator.h
  )

set(SOURCES
  src/main.cpp
  src/VariableRegisterManipulator.cpp
  )

include_directories(include /usr/local/include)

IF(CMAKE_COMPILER_IS_GNUCXX)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y -W -Wall -pedantic")
ENDIF()

set(CMAKE_MODULE_PATH "${AntTest_SOURCE_DIR}/CMakeModules;${CMAKE_MODULE_PATH}")

find_package(SDL2          REQUIRED)

set(APP_NAME AntTest)
add_executable(${APP_NAME} ${SOURCES} ${INCLUDES})
target_link_libraries(${APP_NAME} SDL2 geGLd geCored geDEd geParserd geUtild geCored SDLWindowd AntTweakBar GL assimp)

add_custom_target(copy-shaders ALL
  COMMAND cmake -E copy_directory ${CMAKE_SOURCE_DIR}/shaders ${CMAKE_BINARY_DIR}/shaders
  DEPENDS ${MY_TARGET})


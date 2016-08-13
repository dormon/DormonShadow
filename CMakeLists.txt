cmake_minimum_required(VERSION 3.0.0)

PROJECT(AntTest)

set(INCLUDES
  include/VariableRegisterManipulator.h
  include/RegisterKeyboard.h
  include/RegisterMouse.h
  include/Functions.h
  )

set(SOURCES
  src/main.cpp
  src/VariableRegisterManipulator.cpp
  src/RegisterKeyboard.cpp
  src/RegisterMouse.cpp
  src/Functions.cpp
  )

include_directories(include /usr/local/include)

IF(CMAKE_COMPILER_IS_GNUCXX)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y -W -Wall -pedantic")
ENDIF()

set(CMAKE_MODULE_PATH "${AntTest_SOURCE_DIR}/CMakeModules;${CMAKE_MODULE_PATH}")

find_package(SDL2          REQUIRED)

set(APP_NAME AntTest)
add_executable(${APP_NAME} ${SOURCES} ${INCLUDES})
if(${CMAKE_BUILD_TYPE} MATCHES Debug)
  target_link_libraries(${APP_NAME} SDL2 geGLd geCored geDEd geParserd geUtild geCored SDLWindowd AntTweakBar GL assimp)
else()
  target_link_libraries(${APP_NAME} SDL2 geGL geCore geDE geParser geUtil geCore SDLWindow AntTweakBar GL assimp)
endif()

add_custom_target(copy-shaders ALL
  COMMAND cmake -E copy_directory ${CMAKE_SOURCE_DIR}/shaders ${CMAKE_BINARY_DIR}/shaders
  DEPENDS ${MY_TARGET})


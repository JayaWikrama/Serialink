cmake_minimum_required(VERSION 3.0.0)

# Project configuration
project(Serialink LANGUAGES CXX)

# Define versioning variables
set(PROJECT_VERSION_MAJOR "1")
set(PROJECT_VERSION_MINOR "1")
set(PROJECT_VERSION_PATCH "0")
set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

# Include necessary modules
include(FindPkgConfig)
include(FetchContent)

# Set the default build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Add an option to use USB Serial Module (default: OFF)
option(USE_USB_SERIAL "Enable building for USB Serial Support" OFF)

# Add an option to build tests (default: OFF)
option(BUILD_TESTS "Enable building of unit tests" OFF)

# Declare GoogleTest fetch content (only when tests are enabled)
if(BUILD_TESTS)
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/5376968f6948923e2411081fd9372e71a59d8e77.zip
  )
  FetchContent_MakeAvailable(googletest)
endif()

# Add definition to activate USB Serial Support
if (USE_USB_SERIAL)
  add_definitions(-D__USE_USB_SERIAL__)
endif()

# Verbose compile option
option(VERBOSE "Enable verbose compile" OFF)
if(VERBOSE)
  set(CMAKE_VERBOSE_MAKEFILE ON)
endif()

# Define source files
set(SOURCE_FILES
    src/serial.cpp
    src/usb-serial.cpp
    src/virtuser.cpp
    src/serialink.cpp
    src/virtual-proxy.cpp
)

# Create static library
add_library(${PROJECT_NAME}-ar STATIC ${SOURCE_FILES})
set_target_properties(${PROJECT_NAME}-ar PROPERTIES OUTPUT_NAME ${PROJECT_NAME})

# Create shared library from static library
add_library(${PROJECT_NAME}-lib SHARED $<TARGET_OBJECTS:${PROJECT_NAME}-ar>)
set_target_properties(${PROJECT_NAME}-lib PROPERTIES OUTPUT_NAME ${PROJECT_NAME})

# Create example executables
add_executable(${PROJECT_NAME}
  examples/framed-serial-protocol/src/main.cpp
  examples/framed-serial-protocol/src/data-formating.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/examples/framed-serial-protocol/include>)

if(USE_USB_SERIAL)
  add_executable(${PROJECT_NAME}-usb examples/main-usb-serial.cpp)
endif()
add_executable(${PROJECT_NAME}-virtual examples/main-virtual.cpp)
add_executable(${PROJECT_NAME}-proxy examples/main-proxy.cpp)

# Include directories for the project
set(INCLUDE_DIRS
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/external/DataFrame/include>
  $<INSTALL_INTERFACE:include>
)
target_include_directories(${PROJECT_NAME}-ar PUBLIC ${INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME}-lib PUBLIC ${INCLUDE_DIRS})

# Add dependencies for examples
add_dependencies(${PROJECT_NAME}-lib ${PROJECT_NAME}-ar)
add_dependencies(${PROJECT_NAME} DataFrame-lib)
if(USE_USB_SERIAL)
  add_dependencies(${PROJECT_NAME}-usb DataFrame-lib)
endif()
add_dependencies(${PROJECT_NAME}-virtual DataFrame-lib)
add_dependencies(${PROJECT_NAME}-proxy DataFrame-lib)

# Link libraries to executables
if(USE_USB_SERIAL)
  target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}-lib DataFrame-lib -lpthread -lusb-1.0)
  target_link_libraries(${PROJECT_NAME}-usb PRIVATE ${PROJECT_NAME}-lib DataFrame-lib -lpthread -lusb-1.0)
  target_link_libraries(${PROJECT_NAME}-virtual PRIVATE ${PROJECT_NAME}-lib DataFrame-lib -lpthread -lusb-1.0)
  target_link_libraries(${PROJECT_NAME}-proxy PRIVATE ${PROJECT_NAME}-lib DataFrame-lib -lpthread -lusb-1.0)
else()
  target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}-lib DataFrame-lib -lpthread)
  target_link_libraries(${PROJECT_NAME}-virtual PRIVATE ${PROJECT_NAME}-lib DataFrame-lib -lpthread)
  target_link_libraries(${PROJECT_NAME}-proxy PRIVATE ${PROJECT_NAME}-lib DataFrame-lib -lpthread)
endif()

# Add test configuration (only when tests are enabled)
if(BUILD_TESTS)
  add_executable(${PROJECT_NAME}-test test/test-simple.cpp test/test-framed-data.cpp)
  target_include_directories(${PROJECT_NAME}-test PUBLIC ${INCLUDE_DIRS} ${GTest_INCLUDE_DIRS})
  if(USE_USB_SERIAL)
    target_link_libraries(${PROJECT_NAME}-test PRIVATE ${PROJECT_NAME}-lib DataFrame-lib gtest gtest_main -lpthread -lusb-1.0)
  else()
    target_link_libraries(${PROJECT_NAME}-test PRIVATE ${PROJECT_NAME}-lib DataFrame-lib gtest gtest_main -lpthread)
  endif()
  enable_testing()
  add_test(NAME example_test COMMAND ${PROJECT_NAME}-test)
endif()

# Compiler and linker flags
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -fPIC")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -fPIC")

# Setup for package installation
install(TARGETS ${PROJECT_NAME}-lib
  EXPORT "${PROJECT_NAME}Targets"
  DESTINATION "/usr/lib/"
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Add subdirectory for DataFrame
add_subdirectory(external/DataFrame EXCLUDE_FROM_ALL)
set(USE_EXE_FUNC ON)
set(USE_POST_EXE_FUNC ON)
add_definitions(-D__USE_EXE_FUNC)
add_definitions(-D__USE_POST_FUNC)

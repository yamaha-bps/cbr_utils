cmake_minimum_required(VERSION 3.15)
project(cbr_utils VERSION 0.1.0)

# ---------------------------------------------------------------------------------------
# GLOBAL FLAGS
# ---------------------------------------------------------------------------------------

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


# ---------------------------------------------------------------------------------------
# OPTIONS
# ---------------------------------------------------------------------------------------

# Release type
set(default_build_type "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Build examples
if(NOT BUILD_EXAMPLES)
  set(BUILD_EXAMPLES OFF CACHE BOOL "Build examples." FORCE)
endif()

# Build examples
if(NOT BUILD_TESTING)
  set(BUILD_TESTING OFF CACHE BOOL "Build tests." FORCE)
endif()

# ---------------------------------------------------------------------------------------
# DEPENDENCIES
# ---------------------------------------------------------------------------------------

find_package(Boost REQUIRED)
find_package(yaml-cpp REQUIRED)


# ---------------------------------------------------------------------------------------
# TARGETS
# ---------------------------------------------------------------------------------------

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME}
INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
target_link_libraries(${PROJECT_NAME}
INTERFACE
  Boost::headers
  yaml-cpp
)


# ---------------------------------------------------------------------------------------
# INSTALLATION
# ---------------------------------------------------------------------------------------

include(GNUInstallDirs)

# Header files
install(
  DIRECTORY    ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}
  DESTINATION  ${CMAKE_INSTALL_INCLUDEDIR}
)

# Targets
install(
  TARGETS               ${PROJECT_NAME}
  EXPORT                ${PROJECT_NAME}_targets
  LIBRARY DESTINATION   ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION   ${CMAKE_INSTALL_LIBDIR}
)


# ---------------------------------------------------------------------------------------
# CMAKE CONFIGURATION
# ---------------------------------------------------------------------------------------

set(export_dest_dir "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")

# Generate TARGETS files
install(
  EXPORT       ${PROJECT_NAME}_targets
  FILE         ${PROJECT_NAME}Targets.cmake
  NAMESPACE    ${PROJECT_NAME}::
  DESTINATION  ${export_dest_dir}
)

# Generate CONFIG file
include(CMakePackageConfigHelpers)
configure_package_config_file(
  cmake/ConfigTemplate.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
  INSTALL_DESTINATION ${export_dest_dir}
)

# Generate VERSION file
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Version.cmake
  VERSION ${PACKAGE_VERSION}
  COMPATIBILITY SameMajorVersion
)

# Install CONFIG and VERSION files
install(
  FILES        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Version.cmake
  DESTINATION  ${export_dest_dir}
)

# Add target for uninstallation
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/uninstall.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/uninstall.cmake
  IMMEDIATE @ONLY
)
add_custom_target(uninstall COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/uninstall.cmake)

# ---------------------------------------------------------------------------------------
# EXAMPLES
# ---------------------------------------------------------------------------------------
if(BUILD_EXAMPLES)
  file(GLOB EXAMPLES_SRC "examples/*")
  foreach(testsourcefile ${EXAMPLES_SRC})
    string(REPLACE ".cpp" "" testname ${testsourcefile})
    add_executable(${testname} ${testsourcefile})
    target_link_libraries(${testname} YourLib)
  endforeach(testsourcefile ${PROJECT_NAME})
endif()

# ---------------------------------------------------------------------------------------
# TESTING
# ---------------------------------------------------------------------------------------

if(BUILD_TESTING)
  enable_testing()
  include(GoogleTest)
  find_package(GTest REQUIRED)

  add_compile_options(-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Werror)

  # Cyber enum
  add_executable(${PROJECT_NAME}_test_cyber_enum test/test_cyber_enum.cpp)
  target_link_libraries(${PROJECT_NAME}_test_cyber_enum PRIVATE ${PROJECT_NAME} GTest::Main)
  gtest_discover_tests(${PROJECT_NAME}_test_cyber_enum)

  # Cyber timer
  add_executable(${PROJECT_NAME}_test_cyber_timer test/test_cyber_timer.cpp)
  target_link_libraries(${PROJECT_NAME}_test_cyber_timer PRIVATE ${PROJECT_NAME} GTest::Main)
  gtest_discover_tests(${PROJECT_NAME}_test_cyber_timer)

  # Loop timer
  add_executable(${PROJECT_NAME}_test_loop_timer test/test_loop_timer.cpp)
  target_link_libraries(${PROJECT_NAME}_test_loop_timer PRIVATE ${PROJECT_NAME} GTest::Main)
  gtest_discover_tests(${PROJECT_NAME}_test_loop_timer)

  # Synchronizer
  add_executable(${PROJECT_NAME}_test_synchronizer test/test_synchronizer.cpp)
  target_link_libraries(${PROJECT_NAME}_test_synchronizer PRIVATE ${PROJECT_NAME} GTest::Main)
  gtest_discover_tests(${PROJECT_NAME}_test_synchronizer)

  # Threadpool
  add_executable(${PROJECT_NAME}_test_threadpool test/test_threadpool.cpp)
  target_link_libraries(${PROJECT_NAME}_test_threadpool PRIVATE ${PROJECT_NAME} GTest::Main)
  gtest_discover_tests(${PROJECT_NAME}_test_threadpool)

  # Introspection
  add_executable(${PROJECT_NAME}_test_introspection test/test_introspection.cpp)
  target_link_libraries(${PROJECT_NAME}_test_introspection PRIVATE ${PROJECT_NAME} GTest::Main)
  gtest_discover_tests(${PROJECT_NAME}_test_introspection)

  # Utils
  add_executable(${PROJECT_NAME}_test_utils test/test_utils.cpp)
  target_link_libraries(${PROJECT_NAME}_test_utils PRIVATE ${PROJECT_NAME} GTest::Main)
  gtest_discover_tests(${PROJECT_NAME}_test_utils)

  # Yaml
  add_executable(${PROJECT_NAME}_test_yaml test/test_yaml.cpp)
  target_link_libraries(${PROJECT_NAME}_test_yaml PRIVATE ${PROJECT_NAME} GTest::Main)
  gtest_discover_tests(${PROJECT_NAME}_test_yaml)
endif()


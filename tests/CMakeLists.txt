cmake_minimum_required(VERSION 3.9.2)

project (eventqueue-tests)

# Download gtest (https://github.com/google/googletest).
# See https://stackoverflow.com/a/27160614
include(ExternalProject)
ExternalProject_Add(gtest
  URL https://github.com/google/googletest/archive/release-1.8.0.zip

  PREFIX ${CMAKE_CURRENT_BINARY_DIR}/gtest
  INSTALL_COMMAND ""
)
ExternalProject_Get_Property(gtest source_dir binary_dir)

add_executable(${PROJECT_NAME} eventqueue-tests.cpp)

add_dependencies(${PROJECT_NAME} gtest)

include_directories(${source_dir}/googletest/include)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} Threads::Threads)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_11)

target_link_libraries(${PROJECT_NAME} ${binary_dir}/googlemock/gtest/libgtest.a)
target_link_libraries(${PROJECT_NAME} ${binary_dir}/googlemock/gtest/libgtest_main.a)

add_custom_target(tests ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME} DEPENDS ${PROJECT_NAME})

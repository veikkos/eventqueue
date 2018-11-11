cmake_minimum_required(VERSION 3.13.0)

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

add_executable(eventqueue-tests eventqueue-tests.cpp)

add_dependencies(eventqueue-tests gtest)

include_directories(${source_dir}/googletest/include)
target_include_directories(eventqueue-tests PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(eventqueue-tests Threads::Threads)

target_compile_features(eventqueue-tests PUBLIC cxx_std_11)

target_link_libraries(eventqueue-tests ${binary_dir}/googlemock/gtest/libgtest.a)
target_link_libraries(eventqueue-tests ${binary_dir}/googlemock/gtest/libgtest_main.a)

cmake_minimum_required(VERSION 3.21)
project(Assignment_3 VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)

# Add sources
FILE(GLOB MY_SOURCES Source/**)

# Create executable
add_executable(Assignment_3 ${MY_SOURCES})

include_directories(/usr/local/include/)

#target_link_libraries(Assignment_3 gtest gtest_main pthread)



# Set warning level
if (MSVC)
    target_compile_options(Assignment_3 PRIVATE /W4)
else()
    target_compile_options(Assignment_3 PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Maintain folder structure in IDEs
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${MY_SOURCES})

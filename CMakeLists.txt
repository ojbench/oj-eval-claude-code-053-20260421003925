cmake_minimum_required(VERSION 3.10)
project(RegexEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Create a dummy executable just for compilation success
# The actual testing will be done with src.hpp
add_custom_target(code ALL
    COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_BINARY_DIR}/code
    COMMENT "Creating dummy code file"
)

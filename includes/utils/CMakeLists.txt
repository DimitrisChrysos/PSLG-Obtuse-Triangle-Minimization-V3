# Create the utils library
add_library(utils utils.cpp)

# Include the current directory for headers
target_include_directories(utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
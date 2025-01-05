# Create the utils library
add_library(input_categories input_categories.cpp)

# Include the current directory for headers
target_include_directories(input_categories PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
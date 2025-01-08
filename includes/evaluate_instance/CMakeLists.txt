# Create the utils library
add_library(evaluate_instance evaluate_instance.cpp)

# Include the current directory for headers
target_include_directories(evaluate_instance PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
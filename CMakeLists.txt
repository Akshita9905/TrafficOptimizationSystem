cmake_minimum_required(VERSION 3.15)
project(TrafficOptimization)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(TrafficOptimization
    backend/main.cpp
    backend/Graph.cpp
    backend/TrafficSensor.cpp
    backend/TrafficOptimizer.cpp
)

target_include_directories(TrafficOptimization PRIVATE ${CMAKE_SOURCE_DIR})

if(WIN32)
    target_link_libraries(TrafficOptimization ws2_32)
endif()

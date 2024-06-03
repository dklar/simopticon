cmake_minimum_required(VERSION 3.15)
project(StabilityAnalyzer VERSION 0.1.0 LANGUAGES C CXX)

set(SOURCES main.cpp FourierTransform.cpp
    platooning.cpp FrequencyResponseAnalyzer.cpp
    MagnitudeSpectrumBuilder.cpp
    )
message(STATUS "Quellen='${SOURCES}'")

find_package(Eigen3 3.3 REQUIRED)
find_package(OpenMP REQUIRED)
find_package(GTest REQUIRED)

include_directories(${EIGEN3_INCLUDE_DIR})
add_executable(${PROJECT_NAME} ${SOURCES})
#target_include_directories(${PROJECT_NAME} PRIVATE Inc)
target_link_libraries(${PROJECT_NAME} Eigen3::Eigen OpenMP::OpenMP_CXX)

add_subdirectory(Test)

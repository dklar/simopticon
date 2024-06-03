#project(gtest CXX C)
cmake_minimum_required(VERSION 3.15)
 

find_package(GTest REQUIRED)
find_package(OpenMP REQUIRED)



include_directories(${GTEST_INCLUDE_DIRS})

add_executable(simple_test test1.cpp helper.cpp 
            ../FourierTransform.cpp ../MagnitudeSpectrumBuilder.cpp
            ../FrequencyResponseAnalyzer.cpp
            )
            
target_link_libraries(simple_test ${GTEST_LIBRARIES} pthread OpenMP::OpenMP_CXX)
#add_test(simple_gtest test1)
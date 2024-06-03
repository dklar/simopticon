cmake_minimum_required(VERSION 3.24)
project(CrowRestServerSimopticon VERSION 0.1.0 LANGUAGES C CXX)

include(FetchContent)
FetchContent_Declare(
    crow
    GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
    GIT_TAG master  
)
FetchContent_MakeAvailable(crow)

include(FetchContent)
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz)
FetchContent_MakeAvailable(json)


add_subdirectory(simopticonLib)

add_executable(CrowRestServerSimopticon main.cpp)

target_link_libraries(CrowRestServerSimopticon 
    PRIVATE
    Crow::Crow
    nlohmann_json::nlohmann_json
    pthread stdc++fs
    simopticonLib
)

add_dependencies(CrowRestServerSimopticon simopticonLib)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
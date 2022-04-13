cmake_minimum_required(VERSION 3.0.0)
project(PlatformDataEngine VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 17)
include(CTest)
enable_testing()

add_executable(PlatformDataEngine "PlatformDataEngine/main.cpp")
set(CMAKE_PREFIX_PATH "vcpkg_installed/x64-linux/")
find_package(box2d CONFIG REQUIRED)
target_link_libraries(PlatformDataEngine PRIVATE box2d::box2d)


find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(PlatformDataEngine PRIVATE nlohmann_json::nlohmann_json)

find_package(SFML COMPONENTS system window graphics CONFIG REQUIRED)
target_link_libraries(PlatformDataEngine PRIVATE sfml-system sfml-network sfml-graphics sfml-window)

find_package(spdlog CONFIG REQUIRED)
target_link_libraries(PlatformDataEngine PRIVATE spdlog::spdlog spdlog::spdlog_header_only)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

add_custom_target(run
    COMMAND ${PROJECT_NAME}
    DEPENDS ${PROJECT_NAME}
    WORKING_DIRECTORY ${CMAKE_PROJECT_DIR}
)
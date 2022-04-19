cmake_minimum_required(VERSION 3.0.0)
project(PlatformDataEngine VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 17)

add_executable(PlatformDataEngine "PlatformDataEngine/main.cpp")

set(CMAKE_PREFIX_PATH "vcpkg_installed/x64-linux/include/")

find_package(box2d CONFIG REQUIRED)
target_link_libraries(PlatformDataEngine PRIVATE box2d::box2d)


find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(PlatformDataEngine PRIVATE nlohmann_json::nlohmann_json)

find_package(SFML COMPONENTS system window graphics CONFIG REQUIRED)
target_link_libraries(PlatformDataEngine PRIVATE sfml-system sfml-network sfml-graphics sfml-window)

find_package(spdlog CONFIG REQUIRED)
target_link_libraries(PlatformDataEngine PRIVATE spdlog::spdlog spdlog::spdlog_header_only)

target_include_directories(PlatformDataEngine
    PRIVATE
    "${PROJECT_BINARY_DIR}"
    "${CMAKE_PREFIX_PATH}/include"
)

add_custom_target(run
    COMMAND ${PROJECT_NAME}
    DEPENDS ${PROJECT_NAME}
    WORKING_DIRECTORY ${CMAKE_PROJECT_DIR}
)

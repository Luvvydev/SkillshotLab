cmake_minimum_required(VERSION 3.16)
project(SkillshotLab LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML 3 REQUIRED COMPONENTS Graphics Window System)


add_executable(SkillshotLab
    src/main.cpp
    src/app.cpp
    src/geometry.cpp
    src/ability.cpp
    src/target.cpp
    src/scenario.cpp
)

target_include_directories(SkillshotLab PRIVATE src)

target_link_libraries(SkillshotLab PRIVATE SFML::Graphics SFML::Window SFML::System)


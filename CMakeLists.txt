cmake_minimum_required(VERSION 3.22)

project(DEFENCE)

# Устанавливаем стандарт C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE DEFENCE_SOURCES "${CMAKE_CURRENT_LIST_DIR}/source/*cpp" )
add_executable(defence "${DEFENCE_SOURCES}")

 find_package(SFML COMPONENTS system window graphics audio CONFIG REQUIRED)
target_link_libraries(defence PRIVATE sfml-system sfml-network sfml-graphics sfml-window sfml-audio)

target_include_directories(defence PUBLIC headers)

find_package(glm CONFIG REQUIRED)
target_link_libraries(defence PRIVATE glm::glm)

find_package(TGUI CONFIG REQUIRED)
target_link_libraries(defence PRIVATE TGUI::TGUI)
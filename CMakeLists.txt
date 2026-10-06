cmake_minimum_required(VERSION 3.16)

project(TimerApp VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Multimedia)

qt_standard_project_setup()

qt_add_executable(TimerApp
    src/main.cpp
    resources.qrc
)

target_link_libraries(TimerApp PRIVATE
    Qt6::Widgets
    Qt6::Multimedia
)

install(TARGETS TimerApp
    RUNTIME DESTINATION bin
)
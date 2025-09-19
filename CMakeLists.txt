cmake_minimum_required(VERSION 3.16)
project(GiperbolaDesk LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
    GiperbolaDesk/Main.cpp
    GiperbolaDesk/src/Desk.cpp
    GiperbolaDesk/src/Network.cpp
    GiperbolaDesk/src/ScreenViewer.cpp
    GiperbolaDesk/src/Widgets.cpp
)

include_directories(GiperbolaDesk/include)

add_executable(${PROJECT_NAME} ${SOURCES})

find_package(SFML 2.6 COMPONENTS system window graphics network audio REQUIRED)

find_package(OpenCV REQUIRED)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        sfml-system
        sfml-window
        sfml-graphics
        sfml-audio
        sfml-network
        ${OpenCV_LIBS}
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/GiperbolaDesk/Graphics
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/Graphics
)
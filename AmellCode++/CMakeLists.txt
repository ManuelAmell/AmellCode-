cmake_minimum_required(VERSION 3.21)
project(AmellIDE VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# AUTOMOC es crucial para generar los archivos moc automáticamente
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.5 COMPONENTS Widgets REQUIRED)

set(APP_SOURCES
    main.cpp
    MainWindow.cpp
    Editor.cpp
    CppHighlighter.cpp
)

set(APP_HEADERS
    MainWindow.h
    Editor.h
    CppHighlighter.h
)

# Target sin guion
qt_add_executable(AmellIDE
    ${APP_SOURCES}
    ${APP_HEADERS}
)

# Ejecutable final con guion
set_target_properties(AmellIDE PROPERTIES OUTPUT_NAME "Amell-IDE")

target_link_libraries(AmellIDE PRIVATE Qt6::Widgets)

if (WIN32)
    set_target_properties(AmellIDE PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

install(TARGETS AmellIDE RUNTIME DESTINATION bin)
cmake_minimum_required(VERSION 3.16)
project(StudentGradeManager)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Charts)

qt_standard_project_setup()

set(SOURCES
    main.cpp
    MainWindow.cpp
    Student.cpp
    GradeManager.cpp
    ChartWidget.cpp
)

set(HEADERS
    MainWindow.h
    Student.h
    GradeManager.h
    ChartWidget.h
)

qt_add_executable(StudentGradeManager ${SOURCES} ${HEADERS})

target_link_libraries(StudentGradeManager Qt6::Core Qt6::Widgets Qt6::Charts)

qt_add_resources(StudentGradeManager "resources"
    PREFIX "/"
    FILES
    styles.qss
)
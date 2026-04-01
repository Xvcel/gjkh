cmake_minimum_required(VERSION 3.20)
project(WeWords VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui)

qt_standard_project_setup()

set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/FlashcardView.cpp
    src/BrowseView.cpp
    src/StatsView.cpp
    src/SettingsDialog.cpp
    src/WordDatabase.cpp
    src/UserProgress.cpp
    src/WordCard.cpp
)

set(HEADERS
    include/MainWindow.h
    include/FlashcardView.h
    include/BrowseView.h
    include/StatsView.h
    include/SettingsDialog.h
    include/WordDatabase.h
    include/UserProgress.h
    include/WordCard.h
    include/Word.h
    include/AppSettings.h
)

qt_add_executable(WeWords
    ${SOURCES}
    ${HEADERS}
    resources/resources.qrc
)

target_include_directories(WeWords PRIVATE include)

target_link_libraries(WeWords PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
)

if(WIN32)
    set_target_properties(WeWords PROPERTIES WIN32_EXECUTABLE TRUE)
    target_sources(WeWords PRIVATE resources/wewords.rc)
endif()

install(TARGETS WeWords
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

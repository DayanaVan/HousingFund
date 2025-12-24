cmake_minimum_required(VERSION 3.16)
project(HousingFund VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
find_package(PostgreSQL REQUIRED)

set(SOURCES
    src/main.cpp
    src/database/DatabaseManager.cpp
    src/ui/MainWindow.cpp
    src/ui/AuthDialog.cpp
    src/ui/AddEditDialog.cpp
    src/ui/ExportDialog.cpp 
    src/utils/HashUtils.cpp
)

set(HEADERS
    src/database/DatabaseManager.h
    src/models/House.h
    src/models/User.h
    src/ui/MainWindow.h
    src/ui/AuthDialog.h
    src/ui/AddEditDialog.h
    src/ui/ExportDialog.h 
    src/utils/HashUtils.h
    src/config/Config.h
)

set(UI_FILES
    src/ui/MainWindow.ui
    src/ui/AuthDialog.ui
)

add_executable(HousingFund
    ${SOURCES} 
    ${HEADERS}
    ${UI_FILES}
)

target_link_libraries(HousingFund
    Qt6::Core
    Qt6::Widgets
    ${PostgreSQL_LIBRARIES}
    pqxx
    ssl
    crypto
    pthread
)


target_include_directories(HousingFund PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${PostgreSQL_INCLUDE_DIRS}
)

install(TARGETS HousingFund DESTINATION bin)

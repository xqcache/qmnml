set(TARGET_NAME ${PROJECT_NAME})

find_package(QT NAMES Qt6 CONFIG REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} CONFIG REQUIRED COMPONENTS Core)

add_library(${TARGET_NAME} STATIC qmnml.h qmnml.cpp)
target_link_libraries(${TARGET_NAME} PUBLIC Qt${QT_VERSION_MAJOR}::Core)
target_include_directories(${TARGET_NAME} PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>")
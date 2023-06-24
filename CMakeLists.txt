cmake_minimum_required(VERSION 3.14)

project(LightingEffects)

add_definitions(-DUNICODE -D_UNICODE)
add_executable(${PROJECT_NAME} WIN32 Source/ClientCode/Main.cpp)

target_link_directories(${PROJECT_NAME} PRIVATE Build/)

file(GLOB ENGINE_SOURCE
    Source/*.cpp
    Source/*.h
)

add_library(EngineLib ${ENGINE_SOURCE})
target_link_libraries(${PROJECT_NAME} EngineLib)

target_include_directories(${PROJECT_NAME} PRIVATE Source/)
target_link_libraries(${PROJECT_NAME} d3d12.lib dxgi.lib d3dcompiler.lib)

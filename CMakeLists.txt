cmake_minimum_required(VERSION 3.14)

project(LightingEffects)

add_definitions(-DUNICODE -D_UNICODE)

# Main
add_executable(${PROJECT_NAME} WIN32 Source/Main.cpp)


# Engine lib
target_link_directories(${PROJECT_NAME} PRIVATE Build/)

file(GLOB ENGINE_SOURCE
    Source/*.cpp
    Source/*.h
    Source/*.inl
)
add_library(EngineLib ${ENGINE_SOURCE})
target_link_libraries(${PROJECT_NAME} EngineLib)

target_include_directories(${PROJECT_NAME} PRIVATE Source/)

# DirectX12
target_link_libraries(${PROJECT_NAME} d3d12.lib dxgi.lib d3dcompiler.lib)

# Test cases
add_executable(ModuleTest Source/TestCases/TestMain.cpp)
target_link_directories(ModuleTest PRIVATE Build/)
target_link_libraries(ModuleTest EngineLib)
target_include_directories(ModuleTest PRIVATE Source/)


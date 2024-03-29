cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

file(GLOB SHADER_SOURCE Source/*.hlsl)

# Exclude shader file from VS compilation
set_source_files_properties(${SHADER_SOURCE} PROPERTIES VS_TOOL_OVERRIDE "None")

add_library(EngineLib ${ENGINE_SOURCE} ${SHADER_SOURCE})
target_link_libraries(${PROJECT_NAME} EngineLib)

target_include_directories(${PROJECT_NAME} PRIVATE Source/)

# DirectX12
target_link_libraries(${PROJECT_NAME} d3d12.lib dxgi.lib d3dcompiler.lib)

# Test cases
add_executable(ModuleTest Source/TestCases/TestMain.cpp)
target_link_directories(ModuleTest PRIVATE Build/)
target_link_libraries(ModuleTest EngineLib)
target_include_directories(ModuleTest PRIVATE Source/)


cmake_minimum_required(VERSION 3.24)

project(D3D11Framework VERSION 0.1.0 LANGUAGES CXX)

if(NOT WIN32)
    message(FATAL_ERROR "D3D11Framework supports Windows only.")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

# Pinned so editor builds remain reproducible. Update this tag deliberately.
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.92.9
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(imgui)

set(IMGUIZMO_BUILD_EXAMPLE OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    imguizmo_source
    GIT_REPOSITORY https://github.com/CedricGuillemet/ImGuizmo.git
    GIT_TAG 1.10
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(imguizmo_source)

add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_dx11.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp
)

target_include_directories(imgui PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

target_compile_definitions(imgui PRIVATE UNICODE _UNICODE NOMINMAX WIN32_LEAN_AND_MEAN)
target_link_libraries(imguizmo PUBLIC imgui)
target_compile_definitions(imguizmo PRIVATE UNICODE _UNICODE NOMINMAX WIN32_LEAN_AND_MEAN)

add_executable(D3D11Framework WIN32
    Source/main.cpp
    Source/Core/Application.cpp
    Source/Core/Application.h
    Source/Core/Window.cpp
    Source/Core/Window.h
    Source/Graphics/D3D11Renderer.cpp
    Source/Graphics/D3D11Renderer.h
    Source/Graphics/SceneRenderer.cpp
    Source/Graphics/SceneRenderer.h
    Source/Editor/EditorLayer.cpp
    Source/Editor/EditorLayer.h
    Source/Editor/EditorCamera.cpp
    Source/Editor/EditorCamera.h
    Source/Scene/Component.cpp
    Source/Scene/Component.h
    Source/Scene/GameObject.cpp
    Source/Scene/GameObject.h
    Source/Scene/Scene.cpp
    Source/Scene/Scene.h
    Source/Scene/SceneManager.cpp
    Source/Scene/SceneManager.h
    Source/Components/RotatorComponent.cpp
    Source/Components/RotatorComponent.h
    Source/Components/TransformComponent.h
    Source/Components/MeshComponent.h
    Source/Scenes/SampleScene.cpp
    Source/Scenes/SampleScene.h
)

target_include_directories(D3D11Framework PRIVATE Source)
target_compile_definitions(D3D11Framework PRIVATE UNICODE _UNICODE NOMINMAX WIN32_LEAN_AND_MEAN)
target_compile_options(D3D11Framework PRIVATE /W4 /permissive- /utf-8)
target_link_libraries(D3D11Framework PRIVATE imgui imguizmo d3d11 dxgi d3dcompiler)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT D3D11Framework)
set_target_properties(D3D11Framework PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:D3D11Framework>"
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/Source PREFIX "Source" FILES
    Source/main.cpp
    Source/Core/Application.cpp Source/Core/Application.h
    Source/Core/Window.cpp Source/Core/Window.h
    Source/Graphics/D3D11Renderer.cpp Source/Graphics/D3D11Renderer.h
    Source/Graphics/SceneRenderer.cpp Source/Graphics/SceneRenderer.h
    Source/Editor/EditorLayer.cpp Source/Editor/EditorLayer.h
    Source/Editor/EditorCamera.cpp Source/Editor/EditorCamera.h
    Source/Scene/Component.cpp Source/Scene/Component.h
    Source/Scene/GameObject.cpp Source/Scene/GameObject.h
    Source/Scene/Scene.cpp Source/Scene/Scene.h
    Source/Scene/SceneManager.cpp Source/Scene/SceneManager.h
    Source/Components/RotatorComponent.cpp Source/Components/RotatorComponent.h
    Source/Components/TransformComponent.h
    Source/Components/MeshComponent.h
    Source/Scenes/SampleScene.cpp Source/Scenes/SampleScene.h
)

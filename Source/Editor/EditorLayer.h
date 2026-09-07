#pragma once

#include "Editor/EditorCamera.h"

#include <Windows.h>
#include <d3d11.h>
#include <array>

namespace Framework
{
    class D3D11Renderer;
    class GameObject;
    class Scene;
    class SceneManager;
    class SceneRenderer;

    class EditorLayer final
    {
    public:
        EditorLayer() = default;
        ~EditorLayer();

        EditorLayer(const EditorLayer&) = delete;
        EditorLayer& operator=(const EditorLayer&) = delete;

        bool Initialize(HWND window, ID3D11Device* device, ID3D11DeviceContext* context);
        void BeginFrame();
        void UpdateCamera(float deltaTime);
        void Draw(SceneManager& sceneManager, const D3D11Renderer& renderer, float deltaTime);
        void Render();
        bool HandleWindowMessage(HWND window, UINT message, WPARAM wParam, LPARAM lParam) const;

        [[nodiscard]] const float* GetClearColor() const { return m_clearColor; }
        [[nodiscard]] bool IsVSyncEnabled() const { return m_vsync; }
        [[nodiscard]] const EditorCamera& GetCamera() const { return m_camera; }

    private:
        void DrawHierarchy(Scene& scene);
        void DrawInspector();
        void DrawGizmo(const D3D11Renderer& renderer);

        enum class GizmoOperation
        {
            Translate,
            Rotate,
            Scale
        };

        bool m_initialized = false;
        EditorCamera m_camera;
        bool m_showDemoWindow = false;
        bool m_vsync = true;
        float m_clearColor[4]{ 0.08f, 0.10f, 0.14f, 1.0f };
        Scene* m_displayedScene = nullptr;
        GameObject* m_selectedObject = nullptr;
        GameObject* m_nameEditObject = nullptr;
        std::array<char, 128> m_nameBuffer{};
        unsigned int m_newObjectIndex = 1;
        GizmoOperation m_gizmoOperation = GizmoOperation::Translate;
    };
}

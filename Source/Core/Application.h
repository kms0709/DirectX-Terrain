#pragma once

#include "Core/Window.h"
#include "Editor/EditorLayer.h"
#include "Graphics/D3D11Renderer.h"
#include "Graphics/SceneRenderer.h"
#include "Scene/SceneManager.h"

#include <Windows.h>

namespace Framework
{
    class Application final
    {
    public:
        explicit Application(HINSTANCE instance);

        bool Initialize(int showCommand);
        int Run();

    private:
        Window m_window;
        D3D11Renderer m_renderer;
        SceneRenderer m_sceneRenderer;
        EditorLayer m_editor;
        SceneManager m_sceneManager;
    };
}

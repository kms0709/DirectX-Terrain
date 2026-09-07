#include "Core/Application.h"

#include "Scenes/SampleScene.h"

#include <chrono>
#include <stdexcept>

namespace Framework
{
    Application::Application(HINSTANCE instance)
        : m_window(instance)
    {
    }

    bool Application::Initialize(int showCommand)
    {
        m_window.Initialize(L"D3D11 Component Framework", 1280, 720, showCommand);
        m_renderer.Initialize(m_window.GetHandle(), m_window.GetClientWidth(), m_window.GetClientHeight());
        m_sceneRenderer.Initialize(m_renderer);

        if (!m_editor.Initialize(m_window.GetHandle(), m_renderer.GetDevice(), m_renderer.GetContext()))
        {
            throw std::runtime_error("Failed to initialize Dear ImGui.");
        }

        m_window.SetMessageHandler([this](HWND window, UINT message, WPARAM wParam, LPARAM lParam)
        {
            return m_editor.HandleWindowMessage(window, message, wParam, lParam);
        });
        m_window.SetResizeHandler([this](std::uint32_t width, std::uint32_t height)
        {
            m_renderer.Resize(width, height);
        });

        m_sceneManager.AddScene<SampleScene>();
        m_sceneManager.AddScene<ModelingScene>();
        return true;
    }

    int Application::Run()
    {
        using Clock = std::chrono::steady_clock;
        auto previousTime = Clock::now();

        while (m_window.ProcessMessages())
        {
            const auto currentTime = Clock::now();
            const float deltaTime = std::chrono::duration<float>(currentTime - previousTime).count();
            previousTime = currentTime;

            m_sceneManager.Update(deltaTime);

            m_editor.BeginFrame();
            m_editor.UpdateCamera(deltaTime);

            m_renderer.BeginFrame(m_editor.GetClearColor());
            if (const Scene* scene = m_sceneManager.GetActiveScene();
                scene != nullptr && m_renderer.GetWidth() > 0 && m_renderer.GetHeight() > 0)
            {
                const float aspectRatio = static_cast<float>(m_renderer.GetWidth()) /
                    static_cast<float>(m_renderer.GetHeight());
                m_sceneRenderer.Render(*scene, m_renderer,
                    m_editor.GetCamera().GetViewMatrix(),
                    m_editor.GetCamera().GetProjectionMatrix(aspectRatio));
            }
            m_editor.Draw(m_sceneManager, m_renderer, deltaTime);
            m_editor.Render();
            m_renderer.EndFrame(m_editor.IsVSyncEnabled());

            m_sceneManager.LateUpdate(deltaTime);
        }
        return EXIT_SUCCESS;
    }
}

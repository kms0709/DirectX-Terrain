#pragma once

#include <Windows.h>
#include <cstdint>
#include <functional>
#include <string>

namespace Framework
{
    class Window final
    {
    public:
        using MessageHandler = std::function<bool(HWND, UINT, WPARAM, LPARAM)>;
        using ResizeHandler = std::function<void(std::uint32_t, std::uint32_t)>;

        explicit Window(HINSTANCE instance);
        ~Window();

        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;

        bool Initialize(const std::wstring& title, std::uint32_t width, std::uint32_t height, int showCommand);
        bool ProcessMessages();

        void SetMessageHandler(MessageHandler handler) { m_messageHandler = std::move(handler); }
        void SetResizeHandler(ResizeHandler handler) { m_resizeHandler = std::move(handler); }

        [[nodiscard]] HWND GetHandle() const { return m_handle; }
        [[nodiscard]] std::uint32_t GetClientWidth() const { return m_clientWidth; }
        [[nodiscard]] std::uint32_t GetClientHeight() const { return m_clientHeight; }

    private:
        static LRESULT CALLBACK StaticWindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam);
        LRESULT WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam);

        HINSTANCE m_instance = nullptr;
        HWND m_handle = nullptr;
        std::wstring m_className = L"D3D11FrameworkWindow";
        std::uint32_t m_clientWidth = 0;
        std::uint32_t m_clientHeight = 0;
        bool m_quitRequested = false;
        MessageHandler m_messageHandler;
        ResizeHandler m_resizeHandler;
    };
}

#include "Core/Window.h"

#include <stdexcept>

namespace Framework
{
    Window::Window(HINSTANCE instance)
        : m_instance(instance)
    {
    }

    Window::~Window()
    {
        if (m_handle != nullptr)
        {
            DestroyWindow(m_handle);
        }
        UnregisterClassW(m_className.c_str(), m_instance);
    }

    bool Window::Initialize(const std::wstring& title, std::uint32_t width, std::uint32_t height, int showCommand)
    {
        WNDCLASSEXW windowClass{};
        windowClass.cbSize = sizeof(windowClass);
        windowClass.style = CS_HREDRAW | CS_VREDRAW;
        windowClass.lpfnWndProc = StaticWindowProc;
        windowClass.hInstance = m_instance;
        windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        windowClass.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
        windowClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
        windowClass.lpszClassName = m_className.c_str();

        if (RegisterClassExW(&windowClass) == 0)
        {
            throw std::runtime_error("Failed to register the Win32 window class.");
        }

        RECT windowRect{ 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };
        const DWORD style = WS_OVERLAPPEDWINDOW;
        AdjustWindowRect(&windowRect, style, FALSE);

        m_handle = CreateWindowExW(
            0,
            m_className.c_str(),
            title.c_str(),
            style,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            windowRect.right - windowRect.left,
            windowRect.bottom - windowRect.top,
            nullptr,
            nullptr,
            m_instance,
            this);

        if (m_handle == nullptr)
        {
            throw std::runtime_error("Failed to create the Win32 window.");
        }

        m_clientWidth = width;
        m_clientHeight = height;
        ShowWindow(m_handle, showCommand);
        UpdateWindow(m_handle);
        return true;
    }

    bool Window::ProcessMessages()
    {
        MSG message{};
        while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE))
        {
            if (message.message == WM_QUIT)
            {
                m_quitRequested = true;
            }

            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
        return !m_quitRequested;
    }

    LRESULT CALLBACK Window::StaticWindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
    {
        Window* owner = nullptr;
        if (message == WM_NCCREATE)
        {
            const auto* create = reinterpret_cast<CREATESTRUCTW*>(lParam);
            owner = static_cast<Window*>(create->lpCreateParams);
            SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(owner));
            owner->m_handle = window;
        }
        else
        {
            owner = reinterpret_cast<Window*>(GetWindowLongPtrW(window, GWLP_USERDATA));
        }

        return owner != nullptr
            ? owner->WindowProc(window, message, wParam, lParam)
            : DefWindowProcW(window, message, wParam, lParam);
    }

    LRESULT Window::WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
    {
        if (m_messageHandler && m_messageHandler(window, message, wParam, lParam))
        {
            return 1;
        }

        switch (message)
        {
        case WM_SIZE:
            m_clientWidth = static_cast<std::uint32_t>(LOWORD(lParam));
            m_clientHeight = static_cast<std::uint32_t>(HIWORD(lParam));
            if (wParam != SIZE_MINIMIZED && m_resizeHandler)
            {
                m_resizeHandler(m_clientWidth, m_clientHeight);
            }
            return 0;

        case WM_ERASEBKGND:
            return 1;

        case WM_CLOSE:
            DestroyWindow(window);
            return 0;

        case WM_DESTROY:
            m_handle = nullptr;
            PostQuitMessage(0);
            return 0;

        default:
            return DefWindowProcW(window, message, wParam, lParam);
        }
    }
}

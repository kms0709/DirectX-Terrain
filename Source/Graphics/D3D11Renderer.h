#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <wrl/client.h>
#include <cstdint>

namespace Framework
{
    class D3D11Renderer final
    {
    public:
        D3D11Renderer() = default;
        ~D3D11Renderer() = default;

        D3D11Renderer(const D3D11Renderer&) = delete;
        D3D11Renderer& operator=(const D3D11Renderer&) = delete;

        bool Initialize(HWND window, std::uint32_t width, std::uint32_t height);
        void Resize(std::uint32_t width, std::uint32_t height);
        void BeginFrame(const float clearColor[4]);
        void EndFrame(bool verticalSync);

        [[nodiscard]] ID3D11Device* GetDevice() const { return m_device.Get(); }
        [[nodiscard]] ID3D11DeviceContext* GetContext() const { return m_context.Get(); }
        [[nodiscard]] std::uint32_t GetWidth() const { return m_width; }
        [[nodiscard]] std::uint32_t GetHeight() const { return m_height; }

    private:
        void CreateRenderTarget();

        Microsoft::WRL::ComPtr<ID3D11Device> m_device;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_context;
        Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapChain;
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTarget;
        Microsoft::WRL::ComPtr<ID3D11Texture2D> m_depthBuffer;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_depthStencil;
        std::uint32_t m_width = 0;
        std::uint32_t m_height = 0;
    };
}

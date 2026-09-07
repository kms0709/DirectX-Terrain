#include "Graphics/D3D11Renderer.h"

#include <dxgi.h>
#include <iterator>
#include <stdexcept>

namespace Framework
{
    bool D3D11Renderer::Initialize(HWND window, std::uint32_t width, std::uint32_t height)
    {
        DXGI_SWAP_CHAIN_DESC swapChainDescription{};
        swapChainDescription.BufferDesc.Width = width;
        swapChainDescription.BufferDesc.Height = height;
        swapChainDescription.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDescription.SampleDesc.Count = 1;
        swapChainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDescription.BufferCount = 2;
        swapChainDescription.OutputWindow = window;
        swapChainDescription.Windowed = TRUE;
        swapChainDescription.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

        UINT flags = 0;
#if defined(_DEBUG)
        flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

        constexpr D3D_FEATURE_LEVEL requestedLevels[] = {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0
        };
        D3D_FEATURE_LEVEL createdLevel{};

        HRESULT result = D3D11CreateDeviceAndSwapChain(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            flags,
            requestedLevels,
            static_cast<UINT>(std::size(requestedLevels)),
            D3D11_SDK_VERSION,
            &swapChainDescription,
            m_swapChain.GetAddressOf(),
            m_device.GetAddressOf(),
            &createdLevel,
            m_context.GetAddressOf());

        // A Windows runtime without D3D_FEATURE_LEVEL_11_1 rejects the entire
        // level array with E_INVALIDARG, so retry with 11_0 only.
        if (result == E_INVALIDARG)
        {
            constexpr D3D_FEATURE_LEVEL fallbackLevel = D3D_FEATURE_LEVEL_11_0;
            result = D3D11CreateDeviceAndSwapChain(
                nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags,
                &fallbackLevel, 1, D3D11_SDK_VERSION, &swapChainDescription,
                m_swapChain.GetAddressOf(), m_device.GetAddressOf(),
                &createdLevel, m_context.GetAddressOf());
        }

#if defined(_DEBUG)
        // The Graphics Tools optional feature may be absent on a development PC.
        if (result == DXGI_ERROR_SDK_COMPONENT_MISSING)
        {
            flags &= ~D3D11_CREATE_DEVICE_DEBUG;
            result = D3D11CreateDeviceAndSwapChain(
                nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags,
                requestedLevels, static_cast<UINT>(std::size(requestedLevels)),
                D3D11_SDK_VERSION, &swapChainDescription,
                m_swapChain.GetAddressOf(), m_device.GetAddressOf(),
                &createdLevel, m_context.GetAddressOf());
            if (result == E_INVALIDARG)
            {
                constexpr D3D_FEATURE_LEVEL fallbackLevel = D3D_FEATURE_LEVEL_11_0;
                result = D3D11CreateDeviceAndSwapChain(
                    nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags,
                    &fallbackLevel, 1, D3D11_SDK_VERSION, &swapChainDescription,
                    m_swapChain.GetAddressOf(), m_device.GetAddressOf(),
                    &createdLevel, m_context.GetAddressOf());
            }
        }
#endif

        if (FAILED(result))
        {
            throw std::runtime_error("Failed to initialize Direct3D 11.");
        }

        m_width = width;
        m_height = height;
        CreateRenderTarget();
        return true;
    }

    void D3D11Renderer::Resize(std::uint32_t width, std::uint32_t height)
    {
        if (!m_swapChain || width == 0 || height == 0)
        {
            return;
        }

        m_context->OMSetRenderTargets(0, nullptr, nullptr);
        m_renderTarget.Reset();
        m_depthStencil.Reset();
        m_depthBuffer.Reset();

        const HRESULT result = m_swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
        if (FAILED(result))
        {
            throw std::runtime_error("Failed to resize the Direct3D 11 swap chain.");
        }
        m_width = width;
        m_height = height;
        CreateRenderTarget();
    }

    void D3D11Renderer::BeginFrame(const float clearColor[4])
    {
        ID3D11RenderTargetView* renderTarget = m_renderTarget.Get();
        m_context->OMSetRenderTargets(1, &renderTarget, m_depthStencil.Get());
        m_context->ClearRenderTargetView(renderTarget, clearColor);
        m_context->ClearDepthStencilView(m_depthStencil.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
    }

    void D3D11Renderer::EndFrame(bool verticalSync)
    {
        const HRESULT result = m_swapChain->Present(verticalSync ? 1U : 0U, 0);
        if (FAILED(result))
        {
            throw std::runtime_error("Failed to present the Direct3D 11 swap chain.");
        }
    }

    void D3D11Renderer::CreateRenderTarget()
    {
        Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
        if (FAILED(m_swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf()))) ||
            FAILED(m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, m_renderTarget.GetAddressOf())))
        {
            throw std::runtime_error("Failed to create the Direct3D 11 render target.");
        }

        D3D11_TEXTURE2D_DESC depthDescription{};
        depthDescription.Width = m_width;
        depthDescription.Height = m_height;
        depthDescription.MipLevels = 1;
        depthDescription.ArraySize = 1;
        depthDescription.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depthDescription.SampleDesc.Count = 1;
        depthDescription.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        if (FAILED(m_device->CreateTexture2D(&depthDescription, nullptr, m_depthBuffer.GetAddressOf())) ||
            FAILED(m_device->CreateDepthStencilView(m_depthBuffer.Get(), nullptr, m_depthStencil.GetAddressOf())))
        {
            throw std::runtime_error("Failed to create the Direct3D 11 depth buffer.");
        }

        D3D11_VIEWPORT viewport{};
        viewport.Width = static_cast<float>(m_width);
        viewport.Height = static_cast<float>(m_height);
        viewport.MaxDepth = 1.0f;
        m_context->RSSetViewports(1, &viewport);
    }
}

#pragma once

#include "Components/MeshComponent.h"

#include <DirectXMath.h>
#include <d3d11.h>
#include <wrl/client.h>
#include <array>
#include <cstdint>

namespace Framework
{
    class D3D11Renderer;
    class Scene;
    struct Vector3;

    class SceneRenderer final
    {
    public:
        void Initialize(D3D11Renderer& renderer);
        void Render(const Scene& scene, const D3D11Renderer& renderer,
            DirectX::FXMMATRIX view, DirectX::CXMMATRIX projection);
        [[nodiscard]] bool ProjectToScreen(
            const Vector3& worldPosition,
            const D3D11Renderer& renderer,
            float& screenX,
            float& screenY) const;

    public:
        struct PrimitiveMesh
        {
            Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
            Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
            std::uint32_t indexCount = 0;
        };

    private:
        void CreatePipeline(ID3D11Device* device);
        void CreatePrimitives(ID3D11Device* device);

        Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
        Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
        Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantBuffer;
        Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterizerState;
        std::array<PrimitiveMesh, static_cast<std::size_t>(PrimitiveModel::Count)> m_primitives;
        DirectX::XMMATRIX m_view = DirectX::XMMatrixIdentity();
        DirectX::XMMATRIX m_projection = DirectX::XMMatrixIdentity();
    };
}

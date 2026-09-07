#include "Graphics/SceneRenderer.h"

#include "Components/TransformComponent.h"
#include "Graphics/D3D11Renderer.h"
#include "Scene/GameObject.h"
#include "Scene/Scene.h"

#include <d3dcompiler.h>
#include <cmath>
#include <numbers>
#include <stdexcept>
#include <vector>

namespace Framework
{
    namespace
    {
        struct Vertex
        {
            DirectX::XMFLOAT3 position;
            DirectX::XMFLOAT3 normal;
        };

        struct SceneConstants
        {
            DirectX::XMFLOAT4X4 worldViewProjection;
            DirectX::XMFLOAT4X4 world;
            DirectX::XMFLOAT4 color;
        };

        using VertexList = std::vector<Vertex>;
        using IndexList = std::vector<std::uint32_t>;

        void CreateMeshBuffers(ID3D11Device* device, const VertexList& vertices,
            const IndexList& indices, SceneRenderer::PrimitiveMesh& output)
        {
            D3D11_BUFFER_DESC vertexDescription{};
            vertexDescription.ByteWidth = static_cast<UINT>(vertices.size() * sizeof(Vertex));
            vertexDescription.Usage = D3D11_USAGE_IMMUTABLE;
            vertexDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            D3D11_SUBRESOURCE_DATA vertexData{ vertices.data() };

            D3D11_BUFFER_DESC indexDescription{};
            indexDescription.ByteWidth = static_cast<UINT>(indices.size() * sizeof(std::uint32_t));
            indexDescription.Usage = D3D11_USAGE_IMMUTABLE;
            indexDescription.BindFlags = D3D11_BIND_INDEX_BUFFER;
            D3D11_SUBRESOURCE_DATA indexData{ indices.data() };

            if (FAILED(device->CreateBuffer(&vertexDescription, &vertexData, output.vertexBuffer.GetAddressOf())) ||
                FAILED(device->CreateBuffer(&indexDescription, &indexData, output.indexBuffer.GetAddressOf())))
            {
                throw std::runtime_error("Failed to create primitive mesh buffers.");
            }
            output.indexCount = static_cast<std::uint32_t>(indices.size());
        }

        std::pair<VertexList, IndexList> CreateBox()
        {
            VertexList vertices;
            IndexList indices;
            constexpr DirectX::XMFLOAT3 normals[] = {
                { 0, 0, -1 }, { 0, 0, 1 }, { -1, 0, 0 },
                { 1, 0, 0 }, { 0, 1, 0 }, { 0, -1, 0 }
            };
            constexpr DirectX::XMFLOAT3 faceVertices[][4] = {
                {{-.5f,-.5f,-.5f},{-.5f,.5f,-.5f},{.5f,.5f,-.5f},{.5f,-.5f,-.5f}},
                {{-.5f,-.5f,.5f},{.5f,-.5f,.5f},{.5f,.5f,.5f},{-.5f,.5f,.5f}},
                {{-.5f,-.5f,.5f},{-.5f,.5f,.5f},{-.5f,.5f,-.5f},{-.5f,-.5f,-.5f}},
                {{.5f,-.5f,-.5f},{.5f,.5f,-.5f},{.5f,.5f,.5f},{.5f,-.5f,.5f}},
                {{-.5f,.5f,-.5f},{-.5f,.5f,.5f},{.5f,.5f,.5f},{.5f,.5f,-.5f}},
                {{-.5f,-.5f,.5f},{-.5f,-.5f,-.5f},{.5f,-.5f,-.5f},{.5f,-.5f,.5f}}
            };
            for (std::uint32_t face = 0; face < 6; ++face)
            {
                const std::uint32_t base = static_cast<std::uint32_t>(vertices.size());
                for (const auto& position : faceVertices[face])
                    vertices.push_back({ position, normals[face] });
                indices.insert(indices.end(), { base, base + 1, base + 2, base, base + 2, base + 3 });
            }
            return { std::move(vertices), std::move(indices) };
        }

        std::pair<VertexList, IndexList> CreatePlane()
        {
            VertexList vertices = {
                {{-.5f,0,-.5f},{0,1,0}}, {{-.5f,0,.5f},{0,1,0}},
                {{.5f,0,.5f},{0,1,0}}, {{.5f,0,-.5f},{0,1,0}}
            };
            return { std::move(vertices), { 0, 1, 2, 0, 2, 3 } };
        }

        std::pair<VertexList, IndexList> CreateSphere()
        {
            constexpr std::uint32_t slices = 24;
            constexpr std::uint32_t stacks = 16;
            VertexList vertices;
            IndexList indices;
            for (std::uint32_t stack = 0; stack <= stacks; ++stack)
            {
                const float phi = std::numbers::pi_v<float> * static_cast<float>(stack) / stacks;
                for (std::uint32_t slice = 0; slice <= slices; ++slice)
                {
                    const float theta = 2.0f * std::numbers::pi_v<float> * static_cast<float>(slice) / slices;
                    const DirectX::XMFLOAT3 normal{
                        std::sin(phi) * std::cos(theta), std::cos(phi), std::sin(phi) * std::sin(theta) };
                    vertices.push_back({ { normal.x * .5f, normal.y * .5f, normal.z * .5f }, normal });
                }
            }
            for (std::uint32_t stack = 0; stack < stacks; ++stack)
            {
                for (std::uint32_t slice = 0; slice < slices; ++slice)
                {
                    const std::uint32_t a = stack * (slices + 1) + slice;
                    const std::uint32_t b = a + slices + 1;
                    indices.insert(indices.end(), { a, b, a + 1, a + 1, b, b + 1 });
                }
            }
            return { std::move(vertices), std::move(indices) };
        }

        std::pair<VertexList, IndexList> CreateCylinder()
        {
            constexpr std::uint32_t slices = 24;
            VertexList vertices;
            IndexList indices;
            for (std::uint32_t slice = 0; slice <= slices; ++slice)
            {
                const float angle = 2.0f * std::numbers::pi_v<float> * static_cast<float>(slice) / slices;
                const float x = std::cos(angle) * .5f;
                const float z = std::sin(angle) * .5f;
                const DirectX::XMFLOAT3 normal{ std::cos(angle), 0, std::sin(angle) };
                vertices.push_back({ { x, -.5f, z }, normal });
                vertices.push_back({ { x, .5f, z }, normal });
            }
            for (std::uint32_t slice = 0; slice < slices; ++slice)
            {
                const std::uint32_t base = slice * 2;
                indices.insert(indices.end(), { base, base + 1, base + 2, base + 2, base + 1, base + 3 });
            }
            for (const float y : { -.5f, .5f })
            {
                const std::uint32_t center = static_cast<std::uint32_t>(vertices.size());
                vertices.push_back({ {0, y, 0}, {0, y > 0 ? 1.0f : -1.0f, 0} });
                const std::uint32_t ring = static_cast<std::uint32_t>(vertices.size());
                for (std::uint32_t slice = 0; slice <= slices; ++slice)
                {
                    const float angle = 2.0f * std::numbers::pi_v<float> * static_cast<float>(slice) / slices;
                    vertices.push_back({ {std::cos(angle)*.5f, y, std::sin(angle)*.5f}, {0, y > 0 ? 1.0f : -1.0f, 0} });
                }
                for (std::uint32_t slice = 0; slice < slices; ++slice)
                {
                    if (y > 0)
                        indices.insert(indices.end(), { center, ring + slice, ring + slice + 1 });
                    else
                        indices.insert(indices.end(), { center, ring + slice + 1, ring + slice });
                }
            }
            return { std::move(vertices), std::move(indices) };
        }
    }

    void SceneRenderer::Initialize(D3D11Renderer& renderer)
    {
        CreatePipeline(renderer.GetDevice());
        CreatePrimitives(renderer.GetDevice());
    }

    void SceneRenderer::CreatePipeline(ID3D11Device* device)
    {
        constexpr char shader[] = R"(
cbuffer SceneConstants : register(b0)
{
    matrix WorldViewProjection;
    matrix World;
    float4 ObjectColor;
};
struct VSInput { float3 position : POSITION; float3 normal : NORMAL; };
struct VSOutput { float4 position : SV_POSITION; float3 normal : NORMAL; };
VSOutput VSMain(VSInput input)
{
    VSOutput output;
    output.position = mul(float4(input.position, 1.0), WorldViewProjection);
    output.normal = normalize(mul(float4(input.normal, 0.0), World).xyz);
    return output;
}
float4 PSMain(VSOutput input) : SV_TARGET
{
    float lighting = 0.25 + 0.75 * saturate(dot(normalize(input.normal), normalize(float3(-0.4, 0.8, -0.3))));
    return float4(ObjectColor.rgb * lighting, ObjectColor.a);
})";

        Microsoft::WRL::ComPtr<ID3DBlob> vertexBlob;
        Microsoft::WRL::ComPtr<ID3DBlob> pixelBlob;
        Microsoft::WRL::ComPtr<ID3DBlob> errors;
        UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(_DEBUG)
        flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
        if (FAILED(D3DCompile(shader, sizeof(shader), nullptr, nullptr, nullptr, "VSMain", "vs_5_0", flags, 0,
            vertexBlob.GetAddressOf(), errors.GetAddressOf())) ||
            FAILED(D3DCompile(shader, sizeof(shader), nullptr, nullptr, nullptr, "PSMain", "ps_5_0", flags, 0,
            pixelBlob.GetAddressOf(), errors.ReleaseAndGetAddressOf())))
        {
            throw std::runtime_error("Failed to compile the built-in scene shader.");
        }
        if (FAILED(device->CreateVertexShader(vertexBlob->GetBufferPointer(), vertexBlob->GetBufferSize(), nullptr, m_vertexShader.GetAddressOf())) ||
            FAILED(device->CreatePixelShader(pixelBlob->GetBufferPointer(), pixelBlob->GetBufferSize(), nullptr, m_pixelShader.GetAddressOf())))
        {
            throw std::runtime_error("Failed to create scene shaders.");
        }

        constexpr D3D11_INPUT_ELEMENT_DESC layout[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
        };
        if (FAILED(device->CreateInputLayout(layout, 2, vertexBlob->GetBufferPointer(), vertexBlob->GetBufferSize(), m_inputLayout.GetAddressOf())))
        {
            throw std::runtime_error("Failed to create the scene input layout.");
        }

        D3D11_BUFFER_DESC constantDescription{};
        constantDescription.ByteWidth = sizeof(SceneConstants);
        constantDescription.Usage = D3D11_USAGE_DEFAULT;
        constantDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        if (FAILED(device->CreateBuffer(&constantDescription, nullptr, m_constantBuffer.GetAddressOf())))
        {
            throw std::runtime_error("Failed to create the scene constant buffer.");
        }

        D3D11_RASTERIZER_DESC rasterizerDescription{};
        rasterizerDescription.FillMode = D3D11_FILL_SOLID;
        rasterizerDescription.CullMode = D3D11_CULL_NONE;
        rasterizerDescription.DepthClipEnable = TRUE;
        if (FAILED(device->CreateRasterizerState(&rasterizerDescription, m_rasterizerState.GetAddressOf())))
        {
            throw std::runtime_error("Failed to create the scene rasterizer state.");
        }
    }

    void SceneRenderer::CreatePrimitives(ID3D11Device* device)
    {
        auto [boxVertices, boxIndices] = CreateBox();
        auto [sphereVertices, sphereIndices] = CreateSphere();
        auto [cylinderVertices, cylinderIndices] = CreateCylinder();
        auto [planeVertices, planeIndices] = CreatePlane();
        CreateMeshBuffers(device, boxVertices, boxIndices, m_primitives[static_cast<std::size_t>(PrimitiveModel::Box)]);
        CreateMeshBuffers(device, sphereVertices, sphereIndices, m_primitives[static_cast<std::size_t>(PrimitiveModel::Sphere)]);
        CreateMeshBuffers(device, cylinderVertices, cylinderIndices, m_primitives[static_cast<std::size_t>(PrimitiveModel::Cylinder)]);
        CreateMeshBuffers(device, planeVertices, planeIndices, m_primitives[static_cast<std::size_t>(PrimitiveModel::Plane)]);
    }

    void SceneRenderer::Render(const Scene& scene, const D3D11Renderer& renderer,
        DirectX::FXMMATRIX view, DirectX::CXMMATRIX projection)
    {
        if (renderer.GetWidth() == 0 || renderer.GetHeight() == 0)
            return;

        using namespace DirectX;
        m_view = view;
        m_projection = projection;

        ID3D11DeviceContext* context = renderer.GetContext();
        context->IASetInputLayout(m_inputLayout.Get());
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
        context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
        context->VSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());
        context->PSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());
        context->RSSetState(m_rasterizerState.Get());

        constexpr UINT stride = sizeof(Vertex);
        constexpr UINT offset = 0;
        for (const auto& object : scene.GetGameObjects())
        {
            const auto* meshComponent = object->GetComponent<MeshComponent>();
            if (!object->IsActive() || meshComponent == nullptr || !meshComponent->IsEnabled())
                continue;

            const TransformComponent& transform = object->GetTransform();
            const XMMATRIX world =
                XMMatrixScaling(transform.scale.x, transform.scale.y, transform.scale.z) *
                XMMatrixRotationX(XMConvertToRadians(transform.rotation.x)) *
                XMMatrixRotationY(XMConvertToRadians(transform.rotation.y)) *
                XMMatrixRotationZ(XMConvertToRadians(transform.rotation.z)) *
                XMMatrixTranslation(transform.position.x, transform.position.y, transform.position.z);
            SceneConstants constants{};
            XMStoreFloat4x4(&constants.worldViewProjection, XMMatrixTranspose(world * m_view * m_projection));
            XMStoreFloat4x4(&constants.world, XMMatrixTranspose(world));
            constants.color = { meshComponent->color[0], meshComponent->color[1], meshComponent->color[2], meshComponent->color[3] };
            context->UpdateSubresource(m_constantBuffer.Get(), 0, nullptr, &constants, 0, 0);

            const PrimitiveMesh& mesh = m_primitives[static_cast<std::size_t>(meshComponent->GetModel())];
            ID3D11Buffer* vertexBuffer = mesh.vertexBuffer.Get();
            context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
            context->IASetIndexBuffer(mesh.indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
            context->DrawIndexed(mesh.indexCount, 0, 0);
        }
    }

    bool SceneRenderer::ProjectToScreen(const Vector3& position, const D3D11Renderer& renderer,
        float& screenX, float& screenY) const
    {
        using namespace DirectX;
        if (renderer.GetWidth() == 0 || renderer.GetHeight() == 0)
            return false;
        const XMVECTOR projected = XMVector3Project(
            XMVectorSet(position.x, position.y, position.z, 1.0f),
            0.0f, 0.0f, static_cast<float>(renderer.GetWidth()), static_cast<float>(renderer.GetHeight()),
            0.0f, 1.0f, m_projection, m_view, XMMatrixIdentity());
        XMFLOAT3 result{};
        XMStoreFloat3(&result, projected);
        screenX = result.x;
        screenY = result.y;
        return result.z >= 0.0f && result.z <= 1.0f;
    }
}

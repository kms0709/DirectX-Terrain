#include "Editor/EditorCamera.h"

#include <imgui.h>

#include <algorithm>
#include <cmath>

namespace Framework
{
    EditorCamera::EditorCamera()
    {
        Reset();
    }

    EditorCamera::~EditorCamera()
    {
        if (m_navigating && GetCapture() == m_window)
            ReleaseCapture();
    }

    void EditorCamera::Initialize(HWND window)
    {
        m_window = window;
    }

    void EditorCamera::Reset()
    {
        m_position = { 7.0f, 5.0f, -9.0f };
        const DirectX::XMFLOAT3 direction{ -7.0f, -5.0f, 9.0f };
        const float length = std::sqrt(
            direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);
        m_yaw = std::atan2(direction.x, direction.z);
        m_pitch = std::asin(direction.y / length);
        m_fieldOfView = 55.0f;
    }

    void EditorCamera::Update(float deltaTime)
    {
        ImGuiIO& io = ImGui::GetIO();
        const bool rightMouseDown = ImGui::IsMouseDown(ImGuiMouseButton_Right);

        if (!m_navigating && ImGui::IsMouseClicked(ImGuiMouseButton_Right) && !io.WantCaptureMouse)
        {
            m_navigating = true;
            if (m_window != nullptr)
                SetCapture(m_window);
        }
        if (m_navigating && !rightMouseDown)
        {
            m_navigating = false;
            if (GetCapture() == m_window)
                ReleaseCapture();
        }

        if (m_navigating)
        {
            ImGui::SetMouseCursor(ImGuiMouseCursor_None);
            m_yaw += io.MouseDelta.x * m_mouseSensitivity;
            m_pitch -= io.MouseDelta.y * m_mouseSensitivity;
            m_pitch = std::clamp(m_pitch, -1.5533f, 1.5533f);

            const float safeDeltaTime = std::min(deltaTime, 0.1f);
            const float speedMultiplier = ImGui::IsKeyDown(ImGuiKey_LeftShift) ? 3.0f : 1.0f;
            const float movement = m_moveSpeed * speedMultiplier * safeDeltaTime;
            DirectX::XMVECTOR position = DirectX::XMLoadFloat3(&m_position);
            const DirectX::XMVECTOR forward = GetForwardVector();
            const DirectX::XMVECTOR right = GetRightVector();

            if (ImGui::IsKeyDown(ImGuiKey_W))
                position = DirectX::XMVectorMultiplyAdd(forward, DirectX::XMVectorReplicate(movement), position);
            if (ImGui::IsKeyDown(ImGuiKey_S))
                position = DirectX::XMVectorMultiplyAdd(forward, DirectX::XMVectorReplicate(-movement), position);
            if (ImGui::IsKeyDown(ImGuiKey_D))
                position = DirectX::XMVectorMultiplyAdd(right, DirectX::XMVectorReplicate(movement), position);
            if (ImGui::IsKeyDown(ImGuiKey_A))
                position = DirectX::XMVectorMultiplyAdd(right, DirectX::XMVectorReplicate(-movement), position);
            DirectX::XMStoreFloat3(&m_position, position);
        }

        if (!io.WantCaptureMouse && io.MouseWheel != 0.0f)
        {
            DirectX::XMVECTOR position = DirectX::XMLoadFloat3(&m_position);
            position = DirectX::XMVectorMultiplyAdd(
                GetForwardVector(), DirectX::XMVectorReplicate(io.MouseWheel * m_zoomSpeed), position);
            DirectX::XMStoreFloat3(&m_position, position);
        }
    }

    DirectX::XMVECTOR EditorCamera::GetForwardVector() const
    {
        const float cosPitch = std::cos(m_pitch);
        return DirectX::XMVector3Normalize(DirectX::XMVectorSet(
            cosPitch * std::sin(m_yaw),
            std::sin(m_pitch),
            cosPitch * std::cos(m_yaw),
            0.0f));
    }

    DirectX::XMVECTOR EditorCamera::GetRightVector() const
    {
        return DirectX::XMVector3Normalize(DirectX::XMVector3Cross(
            DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), GetForwardVector()));
    }

    DirectX::XMMATRIX EditorCamera::GetViewMatrix() const
    {
        return DirectX::XMMatrixLookToLH(
            DirectX::XMLoadFloat3(&m_position),
            GetForwardVector(),
            DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
    }

    DirectX::XMMATRIX EditorCamera::GetProjectionMatrix(float aspectRatio) const
    {
        return DirectX::XMMatrixPerspectiveFovLH(
            DirectX::XMConvertToRadians(m_fieldOfView),
            std::max(aspectRatio, 0.001f),
            0.1f,
            1000.0f);
    }
}

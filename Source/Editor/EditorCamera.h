#pragma once

#include <DirectXMath.h>
#include <Windows.h>

namespace Framework
{
    class EditorCamera final
    {
    public:
        EditorCamera();
        ~EditorCamera();

        EditorCamera(const EditorCamera&) = delete;
        EditorCamera& operator=(const EditorCamera&) = delete;

        void Initialize(HWND window);
        void Update(float deltaTime);
        void Reset();

        [[nodiscard]] DirectX::XMMATRIX GetViewMatrix() const;
        [[nodiscard]] DirectX::XMMATRIX GetProjectionMatrix(float aspectRatio) const;
        [[nodiscard]] const DirectX::XMFLOAT3& GetPosition() const { return m_position; }
        [[nodiscard]] bool IsNavigating() const { return m_navigating; }

    private:
        [[nodiscard]] DirectX::XMVECTOR GetForwardVector() const;
        [[nodiscard]] DirectX::XMVECTOR GetRightVector() const;

        HWND m_window = nullptr;
        DirectX::XMFLOAT3 m_position{};
        float m_yaw = 0.0f;
        float m_pitch = 0.0f;
        float m_fieldOfView = 55.0f;
        float m_moveSpeed = 5.0f;
        float m_mouseSensitivity = 0.0035f;
        float m_zoomSpeed = 1.25f;
        bool m_navigating = false;
    };
}

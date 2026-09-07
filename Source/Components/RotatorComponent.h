#pragma once

#include "Scene/Component.h"

namespace Framework
{
    class TransformComponent;

    class RotatorComponent final : public Component
    {
    public:
        explicit RotatorComponent(float degreesPerSecond = 30.0f)
            : m_degreesPerSecond(degreesPerSecond)
        {
        }

        [[nodiscard]] const char* GetTypeName() const override { return "Rotator"; }

    protected:
        void Start() override;
        void Update(float deltaTime) override;

    private:
        float m_degreesPerSecond = 30.0f;
        TransformComponent* m_transform = nullptr;
    };
}

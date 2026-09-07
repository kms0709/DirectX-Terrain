#pragma once

#include "Scene/Component.h"

namespace Framework
{
    struct Vector3
    {
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
    };

    class TransformComponent final : public Component
    {
    public:
        [[nodiscard]] const char* GetTypeName() const override { return "Transform"; }

        Vector3 position{};
        Vector3 rotation{};
        Vector3 scale{ 1.0f, 1.0f, 1.0f };

        float mm;
    };
}

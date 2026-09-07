#pragma once

#include "Scene/Component.h"

#include <cstdint>

namespace Framework
{
    enum class PrimitiveModel : std::uint8_t
    {
        Box,
        Sphere,
        Cylinder,
        Plane,
        Count
    };

    class MeshComponent final : public Component
    {
    public:
        explicit MeshComponent(PrimitiveModel model = PrimitiveModel::Box)
            : m_model(model)
        {
        }

        [[nodiscard]] const char* GetTypeName() const override { return "Mesh"; }
        [[nodiscard]] PrimitiveModel GetModel() const { return m_model; }
        void SetModel(PrimitiveModel model) { m_model = model; }

        float color[4]{ 0.30f, 0.65f, 0.95f, 1.0f };

    private:
        PrimitiveModel m_model = PrimitiveModel::Box;
    };
}

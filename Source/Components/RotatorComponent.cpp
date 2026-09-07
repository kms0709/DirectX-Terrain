#include "Components/RotatorComponent.h"
#include "Components/TransformComponent.h"
#include "Scene/GameObject.h"

namespace Framework
{
    void RotatorComponent::Start()
    {
        m_transform = GetGameObject().GetComponent<TransformComponent>();
    }

    void RotatorComponent::Update(float deltaTime)
    {
        if (m_transform != nullptr)
        {
            m_transform->rotation.y += m_degreesPerSecond * deltaTime;
            if (m_transform->rotation.y >= 360.0f)
            {
                m_transform->rotation.y -= 360.0f;
            }
        }
    }
}

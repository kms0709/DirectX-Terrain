#include "Scene/GameObject.h"
#include "Components/TransformComponent.h"

namespace Framework
{
    GameObject::GameObject(std::string name)
        : m_name(std::move(name))
    {
        AddComponent<TransformComponent>();
    }

    GameObject::~GameObject()
    {
        for (const auto& component : m_components)
        {
            component->Destroy();
        }
    }

    void GameObject::Update(float deltaTime)
    {
        if (!IsActive())
        {
            return;
        }
        for (const auto& component : m_components)
        {
            component->Tick(deltaTime);
        }
    }

    void GameObject::LateUpdate(float deltaTime)
    {
        if (!IsActive())
        {
            return;
        }
        for (const auto& component : m_components)
        {
            component->LateTick(deltaTime);
        }
    }

    void GameObject::SetActive(bool active)
    {
        if (m_activeSelf == active)
        {
            return;
        }
        m_activeSelf = active;
        for (const auto& component : m_components)
        {
            component->NotifyOwnerActiveChanged(IsActive());
        }
    }

    void GameObject::SetSceneActive(bool active)
    {
        if (m_sceneActive == active)
            return;
        m_sceneActive = active;
        for (const auto& component : m_components)
            component->NotifyOwnerActiveChanged(IsActive());
    }

    TransformComponent& GameObject::GetTransform() const
    {
        return *GetComponent<TransformComponent>();
    }
}

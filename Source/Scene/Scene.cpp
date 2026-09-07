#include "Scene/Scene.h"

namespace Framework
{
    void Scene::Initialize()
    {
        if (!m_initialized)
        {
            m_initialized = true;
            OnCreate();
        }
    }

    GameObject& Scene::CreateEditorGameObject(std::string name)
    {
        return CreateGameObject(std::move(name));
    }

    void Scene::SetActive(bool active)
    {
        if (m_active == active)
            return;
        m_active = active;
        for (const auto& gameObject : m_gameObjects)
            gameObject->SetSceneActive(active);
    }

    GameObject& Scene::CreateGameObject(std::string name)
    {
        auto gameObject = std::make_unique<GameObject>(std::move(name));
        GameObject& reference = *gameObject;
        m_gameObjects.push_back(std::move(gameObject));
        reference.SetSceneActive(m_active);
        return reference;
    }

    void Scene::Update(float deltaTime)
    {
        for (const auto& gameObject : m_gameObjects)
        {
            gameObject->Update(deltaTime);
        }
    }

    void Scene::LateUpdate(float deltaTime)
    {
        for (const auto& gameObject : m_gameObjects)
        {
            gameObject->LateUpdate(deltaTime);
        }
    }
}

#include "Scene/SceneManager.h"

namespace Framework
{
    bool SceneManager::SetActiveScene(std::size_t index)
    {
        if (index >= m_scenes.size())
        {
            return false;
        }
        if (index == m_activeSceneIndex && m_scenes[index]->IsActive())
            return true;
        if (Scene* current = GetActiveScene())
            current->SetActive(false);
        m_activeSceneIndex = index;
        m_scenes[m_activeSceneIndex]->SetActive(true);
        return true;
    }

    Scene* SceneManager::GetActiveScene() const
    {
        return m_activeSceneIndex < m_scenes.size() ? m_scenes[m_activeSceneIndex].get() : nullptr;
    }

    void SceneManager::Update(float deltaTime)
    {
        if (Scene* scene = GetActiveScene())
        {
            scene->Update(deltaTime);
        }
    }

    void SceneManager::LateUpdate(float deltaTime)
    {
        if (Scene* scene = GetActiveScene())
        {
            scene->LateUpdate(deltaTime);
        }
    }
}

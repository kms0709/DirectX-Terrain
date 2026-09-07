#pragma once

#include "Scene/Scene.h"

#include <concepts>
#include <cstddef>
#include <memory>
#include <utility>
#include <vector>

namespace Framework
{
    class SceneManager final
    {
    public:
        template <typename T, typename... Args>
            requires std::derived_from<T, Scene>
        T& AddScene(Args&&... args)
        {
            auto scene = std::make_unique<T>(std::forward<Args>(args)...);
            T& reference = *scene;
            scene->Initialize();
            m_scenes.push_back(std::move(scene));
            if (m_scenes.size() == 1)
                SetActiveScene(0);
            return reference;
        }

        bool SetActiveScene(std::size_t index);
        void Update(float deltaTime);
        void LateUpdate(float deltaTime);

        [[nodiscard]] Scene* GetActiveScene() const;
        [[nodiscard]] std::size_t GetActiveSceneIndex() const { return m_activeSceneIndex; }
        [[nodiscard]] const std::vector<std::unique_ptr<Scene>>& GetScenes() const { return m_scenes; }

    private:
        std::vector<std::unique_ptr<Scene>> m_scenes;
        std::size_t m_activeSceneIndex = 0;
    };
}

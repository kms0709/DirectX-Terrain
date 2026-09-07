#pragma once

#include "Scene/GameObject.h"

#include <memory>
#include <string>
#include <vector>

namespace Framework
{
    class Scene
    {
    public:
        explicit Scene(std::string name) : m_name(std::move(name)) {}
        virtual ~Scene() = default;

        Scene(const Scene&) = delete;
        Scene& operator=(const Scene&) = delete;

        void Initialize();
        void SetActive(bool active);
        GameObject& CreateEditorGameObject(std::string name);
        void Update(float deltaTime);
        void LateUpdate(float deltaTime);

        [[nodiscard]] const std::string& GetName() const { return m_name; }
        [[nodiscard]] bool IsActive() const { return m_active; }
        [[nodiscard]] const std::vector<std::unique_ptr<GameObject>>& GetGameObjects() const { return m_gameObjects; }

    protected:
        virtual void OnCreate() = 0;
        GameObject& CreateGameObject(std::string name);

    private:
        std::string m_name;
        bool m_initialized = false;
        bool m_active = false;
        std::vector<std::unique_ptr<GameObject>> m_gameObjects;
    };
}

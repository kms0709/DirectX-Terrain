#pragma once

#include "Scene/Component.h"

#include <concepts>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace Framework
{
    class Scene;
    class TransformComponent;

    class GameObject final
    {
    public:
        explicit GameObject(std::string name);
        ~GameObject();

        GameObject(const GameObject&) = delete;
        GameObject& operator=(const GameObject&) = delete;

        template <typename T, typename... Args>
            requires std::derived_from<T, Component>
        T& AddComponent(Args&&... args)
        {
            if (auto* existing = GetComponent<T>())
            {
                return *existing;
            }
            auto component = std::make_unique<T>(std::forward<Args>(args)...);
            T& reference = *component;
            m_components.push_back(std::move(component));
            reference.Attach(*this);
            return reference;
        }

        template <typename T>
            requires std::derived_from<T, Component>
        [[nodiscard]] T* GetComponent() const
        {
            for (const auto& component : m_components)
            {
                if (auto* match = dynamic_cast<T*>(component.get()))
                {
                    return match;
                }
            }
            return nullptr;
        }

        void Update(float deltaTime);
        void LateUpdate(float deltaTime);
        void SetActive(bool active);
        void SetName(std::string name) { m_name = std::move(name); }

        [[nodiscard]] const std::string& GetName() const { return m_name; }
        [[nodiscard]] bool IsActive() const { return m_activeSelf && m_sceneActive; }
        [[nodiscard]] bool IsActiveSelf() const { return m_activeSelf; }
        [[nodiscard]] TransformComponent& GetTransform() const;
        [[nodiscard]] const std::vector<std::unique_ptr<Component>>& GetComponents() const { return m_components; }

    private:
        friend class Scene;
        void SetSceneActive(bool active);

        std::string m_name;
        bool m_activeSelf = true;
        bool m_sceneActive = false;
        std::vector<std::unique_ptr<Component>> m_components;
    };
}

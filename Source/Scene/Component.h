#pragma once

namespace Framework
{
    class GameObject;

    class Component
    {
    public:
        virtual ~Component() = default;

        Component(const Component&) = delete;
        Component& operator=(const Component&) = delete;

        [[nodiscard]] GameObject& GetGameObject() const { return *m_owner; }
        [[nodiscard]] bool IsEnabled() const { return m_enabled; }
        [[nodiscard]] virtual const char* GetTypeName() const { return "Component"; }
        void SetEnabled(bool enabled);

    protected:
        Component() = default;

        virtual void Awake() {}
        virtual void Start() {}
        virtual void Update(float deltaTime) { (void)deltaTime; }
        virtual void LateUpdate(float deltaTime) { (void)deltaTime; }
        virtual void OnEnable() {}
        virtual void OnDisable() {}
        virtual void OnDestroy() {}

    private:
        friend class GameObject;

        void Attach(GameObject& owner);
        void Tick(float deltaTime);
        void LateTick(float deltaTime);
        void NotifyOwnerActiveChanged(bool active);
        void Destroy();

        GameObject* m_owner = nullptr;
        bool m_enabled = true;
        bool m_awakeCalled = false;
        bool m_startCalled = false;
        bool m_effectivelyEnabled = false;
        bool m_destroyCalled = false;
    };
}

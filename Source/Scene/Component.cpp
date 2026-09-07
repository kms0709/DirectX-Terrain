#include "Scene/Component.h"
#include "Scene/GameObject.h"

namespace Framework
{
    void Component::SetEnabled(bool enabled)
    {
        if (m_enabled == enabled)
        {
            return;
        }

        m_enabled = enabled;
        NotifyOwnerActiveChanged(m_owner != nullptr && m_owner->IsActive());
    }

    void Component::Attach(GameObject& owner)
    {
        m_owner = &owner;
        if (!m_awakeCalled)
        {
            m_awakeCalled = true;
            Awake();
        }
        NotifyOwnerActiveChanged(owner.IsActive());
    }

    void Component::Tick(float deltaTime)
    {
        if (!m_effectivelyEnabled)
        {
            return;
        }
        if (!m_startCalled)
        {
            m_startCalled = true;
            Start();
        }
        Update(deltaTime);
    }

    void Component::LateTick(float deltaTime)
    {
        if (m_effectivelyEnabled)
        {
            LateUpdate(deltaTime);
        }
    }

    void Component::NotifyOwnerActiveChanged(bool active)
    {
        const bool shouldBeEnabled = active && m_enabled;
        if (shouldBeEnabled == m_effectivelyEnabled)
        {
            return;
        }

        m_effectivelyEnabled = shouldBeEnabled;
        if (m_effectivelyEnabled)
        {
            OnEnable();
        }
        else
        {
            OnDisable();
        }
    }

    void Component::Destroy()
    {
        if (m_destroyCalled)
        {
            return;
        }
        if (m_effectivelyEnabled)
        {
            m_effectivelyEnabled = false;
            OnDisable();
        }
        m_destroyCalled = true;
        OnDestroy();
    }
}

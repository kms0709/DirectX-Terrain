#include "Editor/EditorLayer.h"

#include "Components/MeshComponent.h"
#include "Components/RotatorComponent.h"
#include "Components/TransformComponent.h"
#include "Graphics/D3D11Renderer.h"
#include "Scene/GameObject.h"
#include "Scene/Scene.h"
#include "Scene/SceneManager.h"

#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include <ImGuizmo.h>

#include <DirectXMath.h>
#include <cmath>
#include <string>

// imgui_impl_win32.h intentionally does not expose this declaration to avoid
// requiring every includer to pull in <Windows.h>. This file already does.
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
    HWND window, UINT message, WPARAM wParam, LPARAM lParam);

namespace Framework
{
    EditorLayer::~EditorLayer()
    {
        if (m_initialized)
        {
            ImGui_ImplDX11_Shutdown();
            ImGui_ImplWin32_Shutdown();
            ImGui::DestroyContext();
        }
    }

    bool EditorLayer::Initialize(HWND window, ID3D11Device* device, ID3D11DeviceContext* context)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        ImGui::StyleColorsDark();
        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowRounding = 5.0f;
        style.FrameRounding = 3.0f;

        if (!ImGui_ImplWin32_Init(window) || !ImGui_ImplDX11_Init(device, context))
        {
            ImGui::DestroyContext();
            return false;
        }

        m_initialized = true;
        m_camera.Initialize(window);
        return true;
    }

    void EditorLayer::BeginFrame()
    {
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        ImGuizmo::BeginFrame();
    }

    void EditorLayer::UpdateCamera(float deltaTime)
    {
        m_camera.Update(deltaTime);
    }

    void EditorLayer::Draw(SceneManager& sceneManager, const D3D11Renderer& renderer, float deltaTime)
    {
        ImGui::SetNextWindowPos(ImVec2(12.0f, 12.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(390.0f, 190.0f), ImGuiCond_FirstUseEver);
        ImGui::Begin("Framework Editor");
        ImGui::Text("Direct3D 11 / Dear ImGui %s", IMGUI_VERSION);
        ImGui::Text("Frame: %.3f ms (%.1f FPS)", deltaTime * 1000.0f,
            deltaTime > 0.0f ? 1.0f / deltaTime : 0.0f);
        ImGui::ColorEdit3("Clear color", m_clearColor);
        ImGui::Checkbox("VSync", &m_vsync);
        ImGui::SameLine();
        ImGui::Checkbox("ImGui demo", &m_showDemoWindow);
        const auto& cameraPosition = m_camera.GetPosition();
        ImGui::Text("Camera: %.1f, %.1f, %.1f", cameraPosition.x, cameraPosition.y, cameraPosition.z);
        ImGui::SameLine();
        if (ImGui::SmallButton("Reset"))
            m_camera.Reset();
        ImGui::TextDisabled("Viewport: RMB + WASD / drag, Wheel: dolly");
        const char* gizmoMode = m_gizmoOperation == GizmoOperation::Translate ? "Move (W)" :
            m_gizmoOperation == GizmoOperation::Rotate ? "Rotate (E)" : "Scale (R)";
        ImGui::Text("Gizmo: %s", gizmoMode);
        ImGui::End();

        const auto& scenes = sceneManager.GetScenes();
        int activeSceneIndex = static_cast<int>(sceneManager.GetActiveSceneIndex());
        ImGui::SetNextWindowPos(ImVec2(12.0f, 212.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(300.0f, 500.0f), ImGuiCond_FirstUseEver);
        ImGui::Begin("Hierarchy");
        const char* activeSceneName = sceneManager.GetActiveScene() ? sceneManager.GetActiveScene()->GetName().c_str() : "None";
        if (ImGui::BeginCombo("Scene", activeSceneName))
        {
            for (std::size_t index = 0; index < scenes.size(); ++index)
            {
                const bool selected = activeSceneIndex == static_cast<int>(index);
                if (ImGui::Selectable(scenes[index]->GetName().c_str(), selected))
                {
                    sceneManager.SetActiveScene(index);
                    activeSceneIndex = static_cast<int>(index);
                }
            }
            ImGui::EndCombo();
        }

        Scene* scene = sceneManager.GetActiveScene();
        if (scene != m_displayedScene)
        {
            m_displayedScene = scene;
            m_selectedObject = nullptr;
        }
        if (scene != nullptr)
            DrawHierarchy(*scene);
        ImGui::End();

        DrawInspector();
        DrawGizmo(renderer);

        if (m_showDemoWindow)
        {
            ImGui::ShowDemoWindow(&m_showDemoWindow);
        }
    }

    void EditorLayer::DrawHierarchy(Scene& scene)
    {
        if (ImGui::Button("+ Add GameObject", ImVec2(-1.0f, 0.0f)))
        {
            const std::string name = "GameObject " + std::to_string(m_newObjectIndex++);
            m_selectedObject = &scene.CreateEditorGameObject(name);
        }
        ImGui::Separator();

        for (const auto& object : scene.GetGameObjects())
        {
            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_SpanAvailWidth |
                ImGuiTreeNodeFlags_NoTreePushOnOpen;
            if (m_selectedObject == object.get())
                flags |= ImGuiTreeNodeFlags_Selected;
            ImGui::TreeNodeEx(object.get(), flags, "%s", object->GetName().c_str());
            if (ImGui::IsItemClicked())
                m_selectedObject = object.get();
        }
    }

    void EditorLayer::DrawInspector()
    {
        ImGui::SetNextWindowPos(ImVec2(930.0f, 12.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(338.0f, 660.0f), ImGuiCond_FirstUseEver);
        ImGui::Begin("Inspector");
        if (m_selectedObject == nullptr)
        {
            ImGui::TextDisabled("Select a GameObject in Hierarchy.");
            ImGui::End();
            return;
        }

        bool active = m_selectedObject->IsActiveSelf();
        if (ImGui::Checkbox("##Active", &active))
            m_selectedObject->SetActive(active);
        ImGui::SameLine();
        if (m_nameEditObject != m_selectedObject)
        {
            m_nameEditObject = m_selectedObject;
            strncpy_s(m_nameBuffer.data(), m_nameBuffer.size(), m_selectedObject->GetName().c_str(), _TRUNCATE);
        }
        ImGui::SetNextItemWidth(-1.0f);
        if (ImGui::InputText("##Name", m_nameBuffer.data(), m_nameBuffer.size()))
            m_selectedObject->SetName(m_nameBuffer.data());
        ImGui::Separator();

        TransformComponent& transform = m_selectedObject->GetTransform();
        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::DragFloat3("Position", &transform.position.x, 0.05f);
            ImGui::DragFloat3("Rotation", &transform.rotation.x, 0.5f);
            ImGui::DragFloat3("Scale", &transform.scale.x, 0.05f, 0.001f, 1000.0f);
        }

        if (auto* mesh = m_selectedObject->GetComponent<MeshComponent>())
        {
            if (ImGui::CollapsingHeader("Mesh", ImGuiTreeNodeFlags_DefaultOpen))
            {
                bool enabled = mesh->IsEnabled();
                if (ImGui::Checkbox("Enabled##Mesh", &enabled))
                    mesh->SetEnabled(enabled);
                constexpr const char* modelNames[] = { "Box", "Sphere", "Cylinder", "Plane" };
                int model = static_cast<int>(mesh->GetModel());
                if (ImGui::Combo("Model", &model, modelNames, IM_ARRAYSIZE(modelNames)))
                    mesh->SetModel(static_cast<PrimitiveModel>(model));
                ImGui::ColorEdit4("Color", mesh->color);
            }
        }

        if (auto* rotator = m_selectedObject->GetComponent<RotatorComponent>())
        {
            if (ImGui::CollapsingHeader("Rotator", ImGuiTreeNodeFlags_DefaultOpen))
            {
                bool enabled = rotator->IsEnabled();
                if (ImGui::Checkbox("Enabled##Rotator", &enabled))
                    rotator->SetEnabled(enabled);
                ImGui::TextDisabled("Rotates around Y at runtime");
            }
        }

        ImGui::Spacing();
        if (ImGui::Button("Add Component", ImVec2(-1.0f, 0.0f)))
            ImGui::OpenPopup("AddComponentPopup");
        if (ImGui::BeginPopup("AddComponentPopup"))
        {
            const bool hasMesh = m_selectedObject->GetComponent<MeshComponent>() != nullptr;
            const bool hasRotator = m_selectedObject->GetComponent<RotatorComponent>() != nullptr;
            if (ImGui::MenuItem("Mesh", nullptr, false, !hasMesh))
                m_selectedObject->AddComponent<MeshComponent>();
            if (ImGui::MenuItem("Rotator", nullptr, false, !hasRotator))
                m_selectedObject->AddComponent<RotatorComponent>();
            ImGui::EndPopup();
        }
        ImGui::End();
    }

    void EditorLayer::DrawGizmo(const D3D11Renderer& renderer)
    {
        if (m_selectedObject == nullptr || !m_selectedObject->IsActive())
            return;

        ImGuiIO& io = ImGui::GetIO();
        if (!m_camera.IsNavigating() && !io.WantTextInput)
        {
            if (ImGui::IsKeyPressed(ImGuiKey_W, false))
                m_gizmoOperation = GizmoOperation::Translate;
            else if (ImGui::IsKeyPressed(ImGuiKey_E, false))
                m_gizmoOperation = GizmoOperation::Rotate;
            else if (ImGui::IsKeyPressed(ImGuiKey_R, false))
                m_gizmoOperation = GizmoOperation::Scale;
        }

        if (renderer.GetWidth() == 0 || renderer.GetHeight() == 0)
            return;

        using namespace DirectX;
        const float aspectRatio = static_cast<float>(renderer.GetWidth()) / renderer.GetHeight();
        XMFLOAT4X4 view{};
        XMFLOAT4X4 projection{};
        XMStoreFloat4x4(&view, m_camera.GetViewMatrix());
        XMStoreFloat4x4(&projection, m_camera.GetProjectionMatrix(aspectRatio));

        TransformComponent& transform = m_selectedObject->GetTransform();
        XMFLOAT4X4 world{};
        const XMMATRIX worldMatrix =
            XMMatrixScaling(transform.scale.x, transform.scale.y, transform.scale.z) *
            XMMatrixRotationX(XMConvertToRadians(transform.rotation.x)) *
            XMMatrixRotationY(XMConvertToRadians(transform.rotation.y)) *
            XMMatrixRotationZ(XMConvertToRadians(transform.rotation.z)) *
            XMMatrixTranslation(transform.position.x, transform.position.y, transform.position.z);
        XMStoreFloat4x4(&world, worldMatrix);

        ImGuizmo::OPERATION operation = ImGuizmo::TRANSLATE;
        if (m_gizmoOperation == GizmoOperation::Rotate)
            operation = ImGuizmo::ROTATE;
        else if (m_gizmoOperation == GizmoOperation::Scale)
            operation = ImGuizmo::SCALE;

        ImGuizmo::SetOrthographic(false);
        ImGuizmo::SetRect(0.0f, 0.0f,
            static_cast<float>(renderer.GetWidth()), static_cast<float>(renderer.GetHeight()));
        ImGuizmo::Enable(!io.WantCaptureMouse || ImGuizmo::IsUsing());

        if (ImGuizmo::Manipulate(&view._11, &projection._11, operation,
            ImGuizmo::LOCAL, &world._11))
        {
            float position[3]{};
            float rotation[3]{};
            float scale[3]{};
            ImGuizmo::DecomposeMatrixToComponents(&world._11, position, rotation, scale);

            transform.position = { position[0], position[1], position[2] };
            transform.rotation = { rotation[0], rotation[1], rotation[2] };
            if (std::abs(scale[0]) > 0.0001f && std::abs(scale[1]) > 0.0001f && std::abs(scale[2]) > 0.0001f)
                transform.scale = { scale[0], scale[1], scale[2] };
        }
    }

    void EditorLayer::Render()
    {
        ImGui::Render();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    }

    bool EditorLayer::HandleWindowMessage(HWND window, UINT message, WPARAM wParam, LPARAM lParam) const
    {
        return m_initialized && ImGui_ImplWin32_WndProcHandler(window, message, wParam, lParam) != 0;
    }
}

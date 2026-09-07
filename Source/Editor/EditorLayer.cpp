#include "Editor/EditorLayer.h"

#include "Components/MeshComponent.h"
#include "Components/RotatorComponent.h"
#include "Components/TransformComponent.h"
#include "Graphics/D3D11Renderer.h"
#include "Graphics/SceneRenderer.h"
#include "Scene/GameObject.h"
#include "Scene/Scene.h"
#include "Scene/SceneManager.h"

#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

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
        return true;
    }

    void EditorLayer::BeginFrame()
    {
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
    }

    void EditorLayer::Draw(SceneManager& sceneManager, const SceneRenderer& sceneRenderer,
        const D3D11Renderer& renderer, float deltaTime)
    {
        ImGui::SetNextWindowPos(ImVec2(12.0f, 12.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(360.0f, 150.0f), ImGuiCond_FirstUseEver);
        ImGui::Begin("Framework Editor");
        ImGui::Text("Direct3D 11 / Dear ImGui %s", IMGUI_VERSION);
        ImGui::Text("Frame: %.3f ms (%.1f FPS)", deltaTime * 1000.0f,
            deltaTime > 0.0f ? 1.0f / deltaTime : 0.0f);
        ImGui::ColorEdit3("Clear color", m_clearColor);
        ImGui::Checkbox("VSync", &m_vsync);
        ImGui::SameLine();
        ImGui::Checkbox("ImGui demo", &m_showDemoWindow);
        ImGui::End();

        const auto& scenes = sceneManager.GetScenes();
        int activeSceneIndex = static_cast<int>(sceneManager.GetActiveSceneIndex());
        ImGui::SetNextWindowPos(ImVec2(12.0f, 172.0f), ImGuiCond_FirstUseEver);
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
        DrawGizmo(sceneRenderer, renderer);

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

    void EditorLayer::DrawGizmo(const SceneRenderer& sceneRenderer, const D3D11Renderer& renderer)
    {
        if (m_selectedObject == nullptr || !m_selectedObject->IsActive())
            return;

        float x = 0.0f;
        float y = 0.0f;
        if (!sceneRenderer.ProjectToScreen(m_selectedObject->GetTransform().position, renderer, x, y))
            return;

        ImDrawList* drawList = ImGui::GetForegroundDrawList();
        const ImVec2 origin(x, y);
        constexpr float axisLength = 48.0f;
        drawList->AddCircleFilled(origin, 4.0f, IM_COL32(255, 255, 255, 230));
        drawList->AddLine(origin, ImVec2(x + axisLength, y), IM_COL32(235, 70, 70, 255), 3.0f);
        drawList->AddTriangleFilled(ImVec2(x + axisLength + 7, y), ImVec2(x + axisLength - 2, y - 5),
            ImVec2(x + axisLength - 2, y + 5), IM_COL32(235, 70, 70, 255));
        drawList->AddText(ImVec2(x + axisLength + 8, y - 8), IM_COL32(255, 100, 100, 255), "X");
        drawList->AddLine(origin, ImVec2(x, y - axisLength), IM_COL32(80, 220, 100, 255), 3.0f);
        drawList->AddTriangleFilled(ImVec2(x, y - axisLength - 7), ImVec2(x - 5, y - axisLength + 2),
            ImVec2(x + 5, y - axisLength + 2), IM_COL32(80, 220, 100, 255));
        drawList->AddText(ImVec2(x + 5, y - axisLength - 14), IM_COL32(100, 255, 120, 255), "Y");
        drawList->AddLine(origin, ImVec2(x - 32.0f, y + 32.0f), IM_COL32(70, 130, 245, 255), 3.0f);
        drawList->AddText(ImVec2(x - 45.0f, y + 30.0f), IM_COL32(100, 150, 255, 255), "Z");
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

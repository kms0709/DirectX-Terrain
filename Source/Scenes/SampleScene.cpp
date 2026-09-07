#include "Scenes/SampleScene.h"

#include "Components/MeshComponent.h"
#include "Components/RotatorComponent.h"
#include "Components/TransformComponent.h"

namespace Framework
{
    void SampleScene::OnCreate()
    {
        GameObject& box = CreateGameObject("Rotating Box");
        box.GetTransform().position = { 0.0f, 0.5f, 0.0f };
        box.AddComponent<MeshComponent>(PrimitiveModel::Box);
        box.AddComponent<RotatorComponent>(30.0f);

        GameObject& ground = CreateGameObject("Ground");
        ground.GetTransform().scale = { 6.0f, 1.0f, 6.0f };
        auto& mesh = ground.AddComponent<MeshComponent>(PrimitiveModel::Plane);
        mesh.color[0] = 0.25f;
        mesh.color[1] = 0.32f;
        mesh.color[2] = 0.25f;
    }

    void ModelingScene::OnCreate()
    {
        constexpr PrimitiveModel models[] = {
            PrimitiveModel::Box, PrimitiveModel::Sphere,
            PrimitiveModel::Cylinder, PrimitiveModel::Plane
        };
        constexpr const char* names[] = { "Box", "Sphere", "Cylinder", "Plane" };

        for (int index = 0; index < 4; ++index)
        {
            GameObject& object = CreateGameObject(names[index]);
            object.GetTransform().position = { (static_cast<float>(index) - 1.5f) * 2.2f, 0.5f, 0.0f };
            object.AddComponent<MeshComponent>(models[index]);
        }
    }
}

#pragma once

#include "Scene/Scene.h"

namespace Framework
{
    class SampleScene final : public Scene
    {
    public:
        SampleScene() : Scene("Sample Scene") {}

    protected:
        void OnCreate() override;
    };

    class ModelingScene final : public Scene
    {
    public:
        ModelingScene() : Scene("Model Gallery") {}

    protected:
        void OnCreate() override;
    };
}

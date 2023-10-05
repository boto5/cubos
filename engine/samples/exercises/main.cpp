#include <cubos/engine/cubos.hpp>
#include <cubos/engine/renderer/plugin.hpp>
#include <cubos/engine/renderer/point_light.hpp>
#include <cubos/engine/renderer/environment.hpp>
#include <cubos/engine/transform/plugin.hpp>
#include <cubos/engine/settings/settings.hpp>

#include <iostream>
#include <random>

using cubos::core::ecs::Commands;
using cubos::core::ecs::Write;

using namespace cubos::engine;

typedef struct 
{
    VoxelMaterial rbgValues = {{1.2f, 1.2f, 1.2f, 1.0f}};
    int ntFrame;
} PaletteInfo;

PaletteInfo info;

static void setPaletteSystem(Write<Renderer> renderer)
{

    if (info.ntFrame%1000==0){
        info.rbgValues.color[0] = 1.2f;
        info.rbgValues.color[1] = 1.2f;
        info.rbgValues.color[2] = 1.2f;
    }

    info.rbgValues.color[rand()%3]+=0.01f;
    info.rbgValues.color[rand()%3]-=0.002f;

    // Create a simple palette with 3 materials (red, green and blue).
    (*renderer)->setPalette(VoxelPalette{{
        {info.rbgValues}
    }});

    info.ntFrame++;
}

static void spawnVoxelGridSystem(Commands commands, Write<Assets> assets)
{
    // Create a 2x2x2 grid whose voxels alternate between the materials defined in the palette.
    auto gridAsset = assets->create(VoxelGrid{{2, 2, 2}, {1, 0, 0, 0, 0, 0, 0, 0}}); // just one voxel

    // Spawn an entity with a renderable grid component and a identity transform.
    commands.create(RenderableGrid{gridAsset, {-1.0F, 0.0F, -1.0F}}, LocalToWorld{});
}

static void spawnLightSystem(Commands commands)
{
    commands.create()
        .add(PointLight{.color = {1.0F, 1.0F, 1.0F}, .intensity = 1.0F, .range = 10.0F})
        .add(Position{{1.0F, 3.0F, -2.0F}});
}

static void setEnvironmentSystem(Write<RendererEnvironment> env)
{
    env->ambient = {0.2F, 0.2F, 0.2F};
    env->skyGradient[0] = {0.1F, 0.2F, 0.4F};
    env->skyGradient[1] = {0.6F, 0.6F, 0.8F};
}

static void spawnCameraSystem(Commands commands, Write<ActiveCameras> camera)
{
    camera->entities[0] =
        commands.create()
            .add(Camera{.fovY = 60.0F, .zNear = 0.1F, .zFar = 100.0F})
            .add(Position{{-3.0, 1.0F, -3.0F}})
            .add(Rotation{glm::quatLookAt(glm::normalize(glm::vec3{1.0F, 0.0F, 1.0F}), glm::vec3{0.0F, 1.0F, 0.0F})})
            .entity();

}

static void settingsSystem(Write<Settings> settings)
{
    // We don't load assets in this sample and we don't even have an assets folder, so we should
    // disable assets IO.
    settings->setBool("assets.io.enabled", false);
}

int main()
{
    Cubos cubos{};

    cubos.addPlugin(rendererPlugin);

    cubos.startupSystem(settingsSystem).tagged("cubos.settings");

    cubos.system(setPaletteSystem).after("cubos.renderer.init");
    cubos.startupSystem(spawnVoxelGridSystem);
    cubos.startupSystem(spawnLightSystem);
    cubos.startupSystem(setEnvironmentSystem);
    cubos.startupSystem(spawnCameraSystem);

    cubos.run();
}

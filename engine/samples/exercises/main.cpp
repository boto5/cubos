#include <cubos/engine/cubos.hpp>
#include <cubos/engine/renderer/plugin.hpp>
#include <cubos/engine/renderer/directional_light.hpp>
#include <cubos/engine/renderer/environment.hpp>
#include <cubos/engine/transform/plugin.hpp>
#include <cubos/engine/settings/settings.hpp>
#include <cubos/engine/voxels/plugin.hpp>

#include <random>

using cubos::core::ecs::Commands;
using cubos::core::ecs::Write;
using cubos::core::ecs::Read;

using namespace cubos::engine;

static const Asset<VoxelGrid> CastleAsset = AnyAsset("79a75078-6d19-4652-873f-4b87e6826bcb");
static const Asset<VoxelPalette> PaletteAsset = AnyAsset("11dd2819-112c-4cb5-bf2b-5b94ff400cda");


static void setPaletteSystem(Read<Assets> assets, Write<Renderer> renderer)
{

    auto palette = assets->read(PaletteAsset);
    (*renderer)->setPalette(*palette);
}

static void spawnCastelSystem(Commands commands, Write<Assets> assets)
{
    auto castle = assets->read(CastleAsset);
    glm::vec3 offset = glm::vec3(castle->size().x, 0.0F, castle->size().z) / -2.0F;

    commands.create().add(RenderableGrid{CastleAsset, offset}).add(LocalToWorld{});
}

static void spawnLightSystem(Commands commands)
{
    commands.create()
        .add(DirectionalLight{glm::vec3(1.0F), 1.0F})
        .add(Rotation{glm::quat(glm::vec3(glm::radians(45.0F), glm::radians(45.0F), 0))});
}

static void spawnCameraSystem(Commands commands, Write<ActiveCameras> camera)
{
    camera->entities[0] =
        commands.create()
            .add(Camera{.fovY = 60.0F, .zNear = 0.1F, .zFar = 100.0F})
            .add(Position{{50.0F, 50.0F, 50.0F}})
            .add(Rotation{glm::quatLookAt(glm::normalize(glm::vec3{-1.0F, -1.0F, -1.0F}), glm::vec3{0.0F, 1.0F, 0.0F})})
            .entity();
}

static void settingsSystem(Write<Settings> settings)
{
    settings->setString("assets.io.path", SAMPLE_ASSETS_FOLDER);
}

static void setEnvironmentSystem(Write<RendererEnvironment> env)
{
    env->ambient = {0.2F, 0.2F, 0.2F};
    env->skyGradient[0] = {0.1F, 0.2F, 0.4F};
    env->skyGradient[1] = {0.6F, 0.6F, 0.8F};
}

int main()
{
    Cubos cubos{};

    cubos.addPlugin(rendererPlugin);
    cubos.addPlugin(voxelsPlugin);
    cubos.addPlugin(assetsPlugin);

    cubos.startupSystem(settingsSystem).tagged("cubos.settings");

    cubos.startupSystem(spawnCameraSystem);
    cubos.startupSystem(spawnLightSystem);
    cubos.startupSystem(setEnvironmentSystem);

    cubos.startupSystem(setPaletteSystem).after("cubos.renderer.init");
    cubos.startupSystem(spawnCastelSystem).tagged("cubos.assets");

    cubos.run();
}

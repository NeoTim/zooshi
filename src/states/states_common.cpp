// Copyright 2015 Google Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "fplbase/input.h"
#include "states/states_common.h"

#ifdef ANDROID_CARDBOARD
#include "fplbase/renderer_hmd.h"
#endif

namespace fpl {
namespace fpl_project {

static const vec4 kGreenishColor(0.05f, 0.2f, 0.1f, 1.0f);

static void RenderStereoscopic(Renderer& renderer, World* world, Camera& camera,
                               Camera* cardboard_camera,
                               InputSystem* input_system) {
#ifdef ANDROID_CARDBOARD
  auto render_callback = [&renderer, world, &camera, cardboard_camera](
      const mat4& hmd_viewport_transform) {
    // Update the Cardboard camera with the translation changes from the given
    // transform, which contains the shifts for the eyes.
    const vec3 hmd_translation =
        (hmd_viewport_transform * mathfu::kAxisW4f * hmd_viewport_transform)
            .xyz();
    const vec3 corrected_translation =
        vec3(hmd_translation.x(), -hmd_translation.z(), hmd_translation.y());
    cardboard_camera->set_position(camera.position() + corrected_translation);
    cardboard_camera->set_facing(camera.facing());
    cardboard_camera->set_up(camera.up());

    auto camera_transform = cardboard_camera->GetTransformMatrix();
    renderer.model_view_projection() = camera_transform;
    world->world_renderer->RenderWorld(camera, renderer, world);
  };

  HeadMountedDisplayRender(input_system, &renderer, kGreenishColor,
                           render_callback);
#else
  (void)renderer;
  (void)world;
  (void)camera;
  (void)cardboard_camera;
  (void)input_system;
#endif  // ANDROID_CARDBOARD
}

void RenderWorld(Renderer& renderer, World* world, Camera& camera,
                 Camera* cardboard_camera, InputSystem* input_system) {
  world->world_renderer->RenderPrep(camera, renderer, world);
  if (world->is_in_cardboard) {
    RenderStereoscopic(renderer, world, camera, cardboard_camera, input_system);
  } else {
    world->world_renderer->RenderWorld(camera, renderer, world);
  }
}

void UpdateMainCamera(Camera* main_camera, World* world) {
  auto player = world->player_component.begin()->entity;
  auto transform_component = &world->transform_component;
  main_camera->set_position(transform_component->WorldPosition(player));
  main_camera->set_facing(
      transform_component->WorldOrientation(player).Inverse() *
      mathfu::kAxisY3f);
  auto player_data = world->entity_manager.GetComponentData<PlayerData>(player);
  auto raft_orientation = transform_component->WorldOrientation(
      world->entity_manager.GetComponent<ServicesComponent>()->raft_entity());
  main_camera->set_up(raft_orientation.Inverse() * player_data->GetUp());
}

gui::Event TextButton(const char *text, float size, const char *id) {
  gui::StartGroup(gui::kLayoutVerticalLeft, size, id);
  gui::SetMargin(gui::Margin(10));
  auto event = gui::CheckEvent();
  if (event & gui::kEventIsDown) {
    gui::ColorBackground(vec4(1.0f, 1.0f, 1.0f, 0.5f));
  } else if (event & gui::kEventHover) {
    gui::ColorBackground(vec4(0.5f, 0.5f, 0.5f, 0.5f));
  }
  gui::Label(text, size);
  gui::EndGroup();
  return event;
}

}  // fpl_project
}  // fpl
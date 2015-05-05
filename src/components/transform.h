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

#ifndef COMPONENTS_TRANSFORM_H_
#define COMPONENTS_TRANSFORM_H_

#include "components_generated.h"
#include "entity/component.h"
#include "mathfu/constants.h"
#include "mathfu/glsl_mappings.h"
#include "mathfu/matrix_4x4.h"

namespace fpl {

// Data for scene object components.
struct TransformData {
  TransformData()
    : position(mathfu::kZeros3f),
      scale(mathfu::kOnes3f),
      orientation(mathfu::quat::identity) {}

  mathfu::vec3 position;
  mathfu::vec3 scale;
  mathfu::quat orientation;

  // We construct the matrix by hand here, because we know that it will
  // always be a composition of rotation, scale, and translation, so we
  // can do things a bit more cleanly than 3 4x4 matrix multiplications.
  mathfu::mat4 GetTransformMatrix() {
    // Start with rotation:
    mathfu::mat3 rot = orientation.ToMatrix();

    // Break it up into columns:
    mathfu::vec4 c0 = mathfu::vec4(rot[0], rot[3], rot[6], 0);
    mathfu::vec4 c1 = mathfu::vec4(rot[1], rot[4], rot[7], 0);
    mathfu::vec4 c2 = mathfu::vec4(rot[2], rot[5], rot[8], 0);
    mathfu::vec4 c3 = mathfu::vec4(0, 0, 0, 1);

    // Apply scale:
    c0 *= scale.x();
    c1 *= scale.y();
    c2 *= scale.z();

    // Apply translation:
    c3[0] = position.x();
    c3[1] = position.y();
    c3[2] = position.z();

    // Compose and return result:
    return mathfu::mat4(c0, c1, c2, c3);
  }

};

class TransformComponent : public entity::Component<TransformData> {
 public:
  TransformComponent() {}

  virtual void AddFromRawData(entity::EntityRef& /*entity*/,
                              const void* /*raw_data*/) {}

  virtual void InitEntity(entity::EntityRef& /*entity*/) {}
};

}  // fpl

FPL_ENTITY_REGISTER_COMPONENT(TransformComponent, TransformData,
                              ComponentDataUnion_TransformDef)

#endif  // COMPONENTS_TRANSFORM_H_
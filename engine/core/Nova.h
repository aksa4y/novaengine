// Nova Engine public umbrella header.
// SPDX-License-Identifier: MIT
#pragma once

#include "Doriax.h"

namespace Nova {

// Transitional public aliases. These keep the existing implementation binary
// stable while callers move from the upstream Doriax namespace to Nova.
using Engine = ::doriax::Engine;
using Scaling = ::doriax::Scaling;
using TextureStrategy = ::doriax::TextureStrategy;
using Platform = ::doriax::Platform;
using GraphicBackend = ::doriax::GraphicBackend;
using BodyType = ::doriax::BodyType;
using ResourceLoadState = ::doriax::ResourceLoadState;
using Scene = ::doriax::Scene;

} // namespace Nova

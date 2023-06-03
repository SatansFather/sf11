#pragma once

#include "src/instance.h"
#include "src/adapter.h"
#include "src/context.h"
#include "src/sampler.h"
#include "src/input_layout.h"
#include "src/rasterizer.h"
#include "src/texture.h"
#include "src/color.h"
#include <memory>

// TODO 
// load shaders from data
// associate texture2d with window
// document

namespace sf11
{
// creates an instance which provides the initial d3d11 interface
std::unique_ptr<SfInstance> CreateInstance(const InstanceCreationParams& params);

// returns a vector containing info for each graphics devices in this machine
std::vector<SfAdapter> EnumerateAdapters();
}
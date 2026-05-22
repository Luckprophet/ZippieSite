#pragma once

#include <cstdint>

namespace ggdoll::domain {

struct Slice {
    std::uint64_t startFrame = 0;
    std::uint64_t endFrame = 0;
    bool reversed = false;
    float pitchSemitones = 0.0F;
};

} // namespace ggdoll::domain

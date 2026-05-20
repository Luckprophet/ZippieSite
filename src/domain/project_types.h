#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ggdoll::domain {

struct AudioClipRef {
    std::string relativePath;
    std::uint64_t frames = 0;
    double sampleRate = 0.0;
};

struct Track {
    std::string id;
    std::vector<AudioClipRef> clips;
};

struct Project {
    std::string id;
    std::string name;
    std::vector<Track> tracks;
};

} // namespace ggdoll::domain

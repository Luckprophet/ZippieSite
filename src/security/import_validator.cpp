#include "security/import_validator.h"

#include <algorithm>
#include <array>

namespace ggdoll::security {

namespace {
constexpr std::array<const char*, 3> kAllowedExtensions{ ".wav", ".mp3", ".flac" };
}

ImportValidator::ImportValidator(std::filesystem::path workspaceRoot)
    : workspaceRoot_(std::filesystem::weakly_canonical(std::move(workspaceRoot))) {}

std::optional<ValidationError> ImportValidator::validateExtension(const std::filesystem::path& candidate) const {
    std::string ext = candidate.extension().string();
    std::ranges::transform(ext, ext.begin(), [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });

    const auto isAllowed = std::ranges::find(kAllowedExtensions, ext) != kAllowedExtensions.end();
    if (!isAllowed) {
        return ValidationError{ "ext_not_allowed", "Only .wav, .mp3, and .flac are accepted in phase 1." };
    }

    return std::nullopt;
}

std::optional<ValidationError> ImportValidator::validateCanonicalInsideWorkspace(const std::filesystem::path& candidate) const {
    const auto canonical = std::filesystem::weakly_canonical(candidate);
    const auto mismatch = std::mismatch(
        workspaceRoot_.begin(), workspaceRoot_.end(), canonical.begin(), canonical.end());

    if (mismatch.first != workspaceRoot_.end()) {
        return ValidationError{ "path_traversal", "Import path escapes workspace root." };
    }

    return std::nullopt;
}

} // namespace ggdoll::security

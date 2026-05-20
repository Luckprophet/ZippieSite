#include "app/import_audio_use_case.h"

namespace ggdoll::app {

ImportAudioUseCase::ImportAudioUseCase(security::ImportValidator validator)
    : validator_(std::move(validator)) {}

ImportAudioResult ImportAudioUseCase::execute(const std::filesystem::path& candidatePath) const {
    if (const auto extError = validator_.validateExtension(candidatePath); extError.has_value()) {
        return { false, {}, extError };
    }

    if (const auto pathError = validator_.validateCanonicalInsideWorkspace(candidatePath); pathError.has_value()) {
        return { false, {}, pathError };
    }

    return { true, candidatePath.lexically_normal().string(), std::nullopt };
}

} // namespace ggdoll::app

#pragma once

#include "security/import_validator.h"

#include <filesystem>
#include <optional>
#include <string>

namespace ggdoll::app {

struct ImportAudioResult {
    bool ok = false;
    std::string normalizedPath;
    std::optional<security::ValidationError> error;
};

class ImportAudioUseCase {
public:
    explicit ImportAudioUseCase(security::ImportValidator validator);

    ImportAudioResult execute(const std::filesystem::path& candidatePath) const;

private:
    security::ImportValidator validator_;
};

} // namespace ggdoll::app

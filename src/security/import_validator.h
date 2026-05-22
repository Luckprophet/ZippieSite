#pragma once

#include <filesystem>
#include <optional>
#include <string>

namespace ggdoll::security {

struct ValidationError {
    std::string code;
    std::string message;
};

class ImportValidator {
public:
    explicit ImportValidator(std::filesystem::path workspaceRoot);

    std::optional<ValidationError> validateExtension(const std::filesystem::path& candidate) const;
    std::optional<ValidationError> validateCanonicalInsideWorkspace(const std::filesystem::path& candidate) const;

private:
    std::filesystem::path workspaceRoot_;
};

} // namespace ggdoll::security

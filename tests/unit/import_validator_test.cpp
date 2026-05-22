#include "app/import_audio_use_case.h"

#include <cassert>
#include <filesystem>

int main() {
    namespace fs = std::filesystem;
    const fs::path workspace = fs::current_path();

    ggdoll::security::ImportValidator validator(workspace);
    ggdoll::app::ImportAudioUseCase importUseCase(validator);

    {
        const auto result = importUseCase.execute(workspace / "beat.wav");
        assert(result.ok);
    }

    {
        const auto result = importUseCase.execute(workspace / "beat.exe");
        assert(!result.ok);
        assert(result.error.has_value());
        assert(result.error->code == "ext_not_allowed");
    }

    return 0;
}

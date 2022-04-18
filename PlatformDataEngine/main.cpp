#include "PlatformDataEngineWrapper.h"
#include <spdlog/spdlog.h>

int main(int argc, char** argv)
{
    PlatformDataEngine::ApplicationMode appMode = PlatformDataEngine::SERVER;
    char* clientArg = argv[1];
    if (argc > 1 && std::strcmp(clientArg, "--client") == 0) {
        appMode = PlatformDataEngine::CLIENT;
    }

    if (argc > 1 && std::strcmp(clientArg, "--dedicated") == 0) {
        appMode = PlatformDataEngine::DEDICATED;
    }

    srand(time(NULL));

    spdlog::set_level(spdlog::level::info);

    PlatformDataEngine::PlatformDataEngineWrapper engine;
    engine.run(appMode);

    return 0;
}
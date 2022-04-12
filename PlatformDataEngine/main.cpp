#include "PlatformDataEngineWrapper.h"
#include <spdlog/spdlog.h>

int main(int argc, char** argv)
{
    bool isClient = false;
    char* clientArg = argv[1];
    if (argc > 1 && std::strcmp(clientArg, "--client") == 0) {
        isClient = true;
    }

    srand(time(NULL));

    spdlog::set_level(spdlog::level::info);

    PlatformDataEngine::PlatformDataEngineWrapper engine;
    engine.run(isClient);

    return 0;
}
#include "PlatformDataEngineWrapper.h"
#include <spdlog/spdlog.h>

int main()
{
    spdlog::set_level(spdlog::level::debug);

    PlatformDataEngine::PlatformDataEngineWrapper engine;
    engine.run();

    return 0;
}
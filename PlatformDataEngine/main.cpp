#include "PlatformDataEngineWrapper.h"
#include <spdlog/spdlog.h>

int main()
{
    srand(time(NULL));

    spdlog::set_level(spdlog::level::info);

    PlatformDataEngine::PlatformDataEngineWrapper engine;
    engine.run();

    return 0;
}
#include "Application.h"
#include "scenes/TitleScene.h"

// ═══════════════════════════════════════════════
//   FI — 下落式音游演示
//   SFML 3.1 + C++17 + Visual Studio 2022
// ═══════════════════════════════════════════════

int main()
{
    // 1920×1080 全高清，正式录制用这个分辨率
    Application app("FI - Rhythm Game", 1920, 1080);

    // 初始场景：标题画面
    app.getSceneManager().pushScene(std::make_unique<TitleScene>());

    // 进入主循环
    app.run();

    return 0;
}

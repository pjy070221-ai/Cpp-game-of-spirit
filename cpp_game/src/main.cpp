#include "Application.h"
#include "SceneManager.h"
#include "TitleScene.h"
#include <memory>
#include <windows.h>
#include <direct.h>
#include <filesystem>

int main() {
    //
    char exePath[MAX_PATH];
    if (GetModuleFileNameA(NULL, exePath, MAX_PATH) == 0) return 1;
    std::filesystem::path exeDir = std::filesystem::path(exePath).parent_path();
    // 从 exe 目录向上搜索，找到包含 assets/ 的目录作为工作目录
    for (auto p = exeDir; !p.empty() && p != p.root_path(); p = p.parent_path()) {
        if (std::filesystem::exists(p / "assets")) {
            _chdir(p.string().c_str());
            break;
        }
    }

    Application app(L"\u6307\u5C16\u632F\u5F8B", 1280, 720);
    app.getSceneManager().pushScene(std::make_unique<TitleScene>());
    app.run();
    return 0;
}

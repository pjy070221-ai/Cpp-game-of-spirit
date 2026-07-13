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
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    std::filesystem::path exeDir(exePath);
    exeDir = exeDir.parent_path().parent_path().parent_path(); // x64/Debug/../../ = cpp_game/
    _chdir(exeDir.string().c_str());

    Application app(L"\u6307\u5C16\u632F\u5F8B", 1280, 720);
    app.getSceneManager().pushScene(std::make_unique<TitleScene>());
    app.run();
    return 0;
}

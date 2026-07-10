#include "Application.h"
#include "SceneManager.h"
#include "TitleScene.h"
#include <memory>

int main() {
    Application app(L"\u6307\u5C16\u632F\u5F8B", 1280, 720);
    app.getSceneManager().pushScene(std::make_unique<TitleScene>());
    app.run();
    return 0;
}

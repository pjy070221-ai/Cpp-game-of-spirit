#include "MusicPlayer.h"
#include <iostream>

MusicPlayer::MusicPlayer()
    : offset(0.0f)
    , loaded(false)
    , title("") {
}

MusicPlayer::~MusicPlayer() {
    stop();
}

bool MusicPlayer::load(const std::string& filePath) {
    if (!music.openFromFile(filePath)) {
        std::cout << "[MusicPlayer] Failed to load: " << filePath << std::endl;
        return false;
    }

    loaded = true;
    title = filePath;
    std::cout << "[MusicPlayer] Loaded: " << filePath << std::endl;
    std::cout << "[MusicPlayer] Duration: " << music.getDuration().asSeconds() << "s" << std::endl;
    return true;
}

void MusicPlayer::play() {
    if (!loaded) return;
    music.play();
    std::cout << "[MusicPlayer] Playing..." << std::endl;
}

void MusicPlayer::pause() {
    music.pause();
    std::cout << "[MusicPlayer] Paused" << std::endl;
}

void MusicPlayer::stop() {
    music.stop();
    std::cout << "[MusicPlayer] Stopped" << std::endl;
}

void MusicPlayer::setVolume(float volume) {
    music.setVolume(volume * 100.0f);
}

void MusicPlayer::setOffset(float offsetMs) {
    offset = offsetMs / 1000.0f;
}

void MusicPlayer::setLoop(bool loop) {
    music.setLooping(loop);
}

float MusicPlayer::getCurrentTime() const {
    if (!loaded) return 0.0f;
    return music.getPlayingOffset().asSeconds() + offset;
}

float MusicPlayer::getTotalTime() const {
    if (!loaded) return 0.0f;
    return music.getDuration().asSeconds();
}

// ============================================================
// SFML 3.1.0: 使用数字值判断播放状态
// 状态值: 0 = Stopped, 1 = Paused, 2 = Playing
// ============================================================
bool MusicPlayer::isPlaying() const {
    if (!loaded) return false;
    return static_cast<int>(music.getStatus()) == 2;
}

bool MusicPlayer::isLoaded() const {
    return loaded;
}

std::string MusicPlayer::getTitle() const {
    return title;
}

void MusicPlayer::reset() {
    stop();
    music.setPlayingOffset(sf::seconds(0));
}
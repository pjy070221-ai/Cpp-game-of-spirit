#include "MusicPlayer.h"

bool MusicPlayer::load(const std::string& filePath) {
    stop();
    m_loaded = m_music.openFromFile(filePath);
    return m_loaded;
}

void MusicPlayer::play() {
    if (m_loaded) m_music.play();
}

void MusicPlayer::pause() {
    if (m_loaded) m_music.pause();
}

void MusicPlayer::stop() {
    if (m_loaded) m_music.stop();
}

void MusicPlayer::reset() {
    stop();
    m_loaded = false;
}

void MusicPlayer::setVolume(float v) {
    m_music.setVolume(v * 100.0f);
}

void MusicPlayer::setOffset(float ms) {
    m_offset = ms / 1000.0f;
}

void MusicPlayer::setLoop(bool l) {
    m_music.setLooping(l);
}

float MusicPlayer::getCurrentTime() const {
    if (!m_loaded) return 0.0f;
    return m_music.getPlayingOffset().asSeconds() - m_offset;
}

float MusicPlayer::getTotalTime() const {
    if (!m_loaded) return 0.0f;
    return m_music.getDuration().asSeconds();
}

bool MusicPlayer::isPlaying() const {
    return m_loaded && m_music.getStatus() == sf::SoundSource::Status::Playing;
}

#pragma once

#include <SFML/Audio.hpp>
#include <string>

class MusicPlayer {
public:
    MusicPlayer() = default;
    ~MusicPlayer() { stop(); }

    bool load(const std::string& filePath);
    void play();
    void pause();
    void stop();
    void reset();

    void setVolume(float v);
    void setOffset(float ms);
    void setLoop(bool l);

    float getCurrentTime() const;
    float getTotalTime() const;
    bool  isPlaying() const;
    bool  isLoaded() const { return m_loaded; }

private:
    sf::Music m_music;
    float     m_offset = 0.0f;
    bool      m_loaded = false;
};

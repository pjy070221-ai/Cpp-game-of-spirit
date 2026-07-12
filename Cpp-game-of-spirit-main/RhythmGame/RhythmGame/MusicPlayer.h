#pragma once

#include <SFML/Audio.hpp>
#include <string>

class MusicPlayer {
public:
    MusicPlayer();
    ~MusicPlayer();

    bool load(const std::string& filePath);
    void play();
    void pause();
    void stop();
    void setVolume(float volume);
    void setOffset(float offsetMs);
    void setLoop(bool loop);  // ÉùÃ÷²»±ä

    float getCurrentTime() const;
    float getTotalTime() const;
    bool isPlaying() const;
    bool isLoaded() const;
    std::string getTitle() const;

    void reset();

private:
    sf::Music music;
    float offset;
    bool loaded;
    std::string title;
};
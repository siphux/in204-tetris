#pragma once
#include <SFML/Audio.hpp>
#include <memory>

enum class MusicTrack {
    MENU,
    GAME
};

// Manages background music for Tetris
class MusicManager {
public:
    MusicManager();
    ~MusicManager();
    
    // Initialize and load music from file
    bool initialize();
    
    // Switch between menu and game music
    void playTrack(MusicTrack track);
    
    // Control playback
    void play();
    void pause();
    void stop();
    void setVolume(float volume);  // 0-100
    
    // Update - call this every frame to handle looping
    void update();
    
    // Check if music is playing
    bool isPlaying() const;
    
private:
    std::unique_ptr<sf::Music> m_menuMusic;
    std::unique_ptr<sf::Music> m_gameMusic;
    sf::Music* m_currentMusic;
    MusicTrack m_currentTrack;
    bool m_initialized;
    bool m_menuMusicLoaded;
    bool m_gameMusicLoaded;
    float m_volume;
};

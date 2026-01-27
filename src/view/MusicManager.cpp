#include "MusicManager.h"
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

// Helper function to find a resource file by trying multiple paths
static std::string findResourcePath(const std::string& filename) {
    // Paths to try (in order of preference)
    std::vector<std::string> pathsToTry = {
        filename,                                    // Current directory
        "../" + filename,                            // Up one level
        "../../" + filename,                         // Up two levels
        "../data/" + filename,                       // Up one level + data/
    };
    
    for (const auto& path : pathsToTry) {
        if (fs::exists(path)) {
            return path;
        }
    }
    
    // Return original filename if not found
    return filename;
}

MusicManager::MusicManager() : m_initialized(false), m_currentMusic(nullptr), m_currentTrack(MusicTrack::MENU), m_volume(30.0f), m_menuMusicLoaded(false), m_gameMusicLoaded(false) {
    m_menuMusic = std::make_unique<sf::Music>();
    m_gameMusic = std::make_unique<sf::Music>();
}

MusicManager::~MusicManager() {
    if (m_menuMusic) {
        m_menuMusic->stop();
    }
    if (m_gameMusic) {
        m_gameMusic->stop();
    }
}

bool MusicManager::initialize() {
    // Find and load menu music
    std::string menuPath = findResourcePath("data/menu_music.mp3");
    if (m_menuMusic->openFromFile(menuPath)) {
        m_menuMusic->setVolume(m_volume);
        m_menuMusic->setLooping(true);
        std::cout << "Loaded menu music from: " << menuPath << std::endl;
        m_menuMusicLoaded = true;
    } else {
        std::cout << "Menu music not found (optional)" << std::endl;
        m_menuMusicLoaded = false;
    }
    
    // Find and load game music
    std::string gamePath = findResourcePath("data/tetris_theme_music.ogg");
    if (m_gameMusic->openFromFile(gamePath)) {
        m_gameMusic->setVolume(m_volume);
        m_gameMusic->setLooping(true);
        std::cout << "Loaded game music from: " << gamePath << std::endl;
        m_gameMusicLoaded = true;
    } else {
        std::cerr << "Failed to load game music file" << std::endl;
        m_gameMusicLoaded = false;
    }
    
    m_initialized = m_gameMusicLoaded;  // Only need game music at minimum
    if (m_initialized) {
        // Start with menu music if available, otherwise game music
        if (m_menuMusicLoaded) {
            m_currentMusic = m_menuMusic.get();
            m_currentTrack = MusicTrack::MENU;
        } else {
            m_currentMusic = m_gameMusic.get();
            m_currentTrack = MusicTrack::GAME;
        }
    }
    
    return m_initialized;
}

void MusicManager::playTrack(MusicTrack track) {
    if (!m_initialized) return;
    
    sf::Music* newMusic = nullptr;
    MusicTrack fallbackTrack = track;
    
    if (track == MusicTrack::MENU) {
        if (m_menuMusicLoaded) {
            // Menu music exists and is loaded
            newMusic = m_menuMusic.get();
        } else {
            // Menu music not available, fall back to game music
            newMusic = m_gameMusic.get();
            fallbackTrack = MusicTrack::GAME;
        }
    } else {  // track == MusicTrack::GAME
        if (m_gameMusicLoaded) {
            // Game music exists and is loaded
            newMusic = m_gameMusic.get();
        } else {
            // Game music not available, fall back to menu music if available
            if (m_menuMusicLoaded) {
                newMusic = m_menuMusic.get();
                fallbackTrack = MusicTrack::MENU;
            }
        }
    }
    
    // Only switch if we're changing tracks and the new music exists
    if (newMusic && newMusic != m_currentMusic) {
        // Stop current music
        if (m_currentMusic) {
            m_currentMusic->stop();
        }
        
        // Switch to new music
        m_currentMusic = newMusic;
        m_currentTrack = fallbackTrack;
        m_currentMusic->setVolume(m_volume);
        m_currentMusic->play();
    }
}

void MusicManager::play() {
    if (m_initialized && m_currentMusic) {
        if (m_currentMusic->getStatus() != sf::Music::Status::Playing) {
            m_currentMusic->play();
        }
    }
}

void MusicManager::pause() {
    if (m_currentMusic) {
        m_currentMusic->pause();
    }
}

void MusicManager::stop() {
    if (m_currentMusic) {
        m_currentMusic->stop();
    }
}

void MusicManager::setVolume(float volume) {
    m_volume = volume;
    if (m_currentMusic) {
        m_currentMusic->setVolume(volume);
    }
}

bool MusicManager::isPlaying() const {
    return m_currentMusic && m_currentMusic->getStatus() == sf::Music::Status::Playing;
}

void MusicManager::update() {
    // Check if music stopped unexpectedly and restart it
    if (m_initialized && m_currentMusic) {
        auto status = m_currentMusic->getStatus();
        if (status == sf::Music::Status::Stopped) {
            // Restart if it stopped (though it shouldn't with looping enabled)
            m_currentMusic->play();
        }
    }
}
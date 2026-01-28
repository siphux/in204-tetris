#include "MusicManager.h"
#include <iostream>
#include <filesystem>
#include <stdexcept>

namespace fs = std::filesystem;

// Helper function to find a resource file by trying multiple paths
static std::string findResourcePath(const std::string& filename) {
    std::vector<std::string> pathsToTry = {
        filename,
        "../" + filename,                         
        "../../" + filename,                 
        "../data/" + filename,                    
    };
    
    for (const auto& path : pathsToTry) {
        try {
            if (fs::exists(path)) {
                return path;
            }
        } catch (const std::exception& e) {
            std::cerr << "Error checking path " << path << ": " << e.what() << std::endl;
        }
    }
    
    // Return original filename if not found
    return filename;
}

MusicManager::MusicManager() : m_initialized(false), m_currentMusic(nullptr), m_currentTrack(MusicTrack::MENU), m_volume(30.0f), m_menuMusicLoaded(false), m_gameMusicLoaded(false) {
    try {
        m_menuMusic = std::make_unique<sf::Music>();
        m_gameMusic = std::make_unique<sf::Music>();
    } catch (const std::exception& e) {
        std::cerr << "Error initializing music objects: " << e.what() << std::endl;
    }
}

MusicManager::~MusicManager() {
    try {
        if (m_menuMusic) {
            m_menuMusic->stop();
        }
        if (m_gameMusic) {
            m_gameMusic->stop();
        }
    } catch (const std::exception& e) {
        std::cerr << "Error stopping music: " << e.what() << std::endl;
    }
}

bool MusicManager::initialize() {
    try {
        // Find and load menu music
        std::string menuPath = findResourcePath("data/menu_music.mp3");
        if (m_menuMusic->openFromFile(menuPath)) {
            m_menuMusic->setVolume(m_volume);
            m_menuMusic->setLooping(true);
            std::cout << "Loaded menu music from: " << menuPath << std::endl;
            m_menuMusicLoaded = true;
        } else {
            std::cerr << "Warning: Menu music not found or failed to load (optional)" << std::endl;
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
            std::cerr << "Warning: Game music not found or failed to load (optional)" << std::endl;
            m_gameMusicLoaded = false;
        }
        
        m_initialized = m_gameMusicLoaded;
        return m_initialized;
    } catch (const std::exception& e) {
        std::cerr << "Exception initializing music: " << e.what() << std::endl;
        m_initialized = false;
        return false;
    }
    if (m_initialized) {
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
            newMusic = m_menuMusic.get();
        } else {
            newMusic = m_gameMusic.get();
            fallbackTrack = MusicTrack::GAME;
        }
    } else {
        if (m_gameMusicLoaded) {
            newMusic = m_gameMusic.get();
        } else {
            if (m_menuMusicLoaded) {
                newMusic = m_menuMusic.get();
                fallbackTrack = MusicTrack::MENU;
            }
        }
    }
    
    if (newMusic && newMusic != m_currentMusic) {
        if (m_currentMusic) {
            m_currentMusic->stop();
        }
        
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
    if (m_initialized && m_currentMusic) {
        auto status = m_currentMusic->getStatus();
        if (status == sf::Music::Status::Stopped) {
            m_currentMusic->play();
        }
    }
}
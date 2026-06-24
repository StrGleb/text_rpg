#pragma once

#include <SFML/Graphics.hpp>
#include <optional>
#include <string>
#include <vector>

#include "entities/Enemy.h"
#include "entities/Player.h"
#include "ui/Menu.h"
#include "world/Map.h"

class Game {
    struct EnemySpawn {
        EnemyType type;
        sf::Vector2f position;
    };

    enum class State { // состояние игры
        Menu,
        Playing,
        LevelComplete,
        Victory
    };

    sf::RenderWindow mWindow;
    Player mPlayer;
    Map mMap;
    Menu mMenu;
    sf::View mView; // камера следования
    sf::Texture mCityTexture;
    std::optional<sf::Sprite> mCitySprite;
    std::vector<EnemySpawn> mEnemySpawns;
    std::vector<Enemy> mEnemies;
    float mParallaxFactor = 0.3f; // коэффициент смещения фона (параллакс)
    sf::Font mFont;               // шрифт
    sf::RectangleShape mOverlayBottomFill;
    sf::RectangleShape mOverlayBottomBorder;
    sf::RectangleShape mOverlayPanelShadow;
    sf::RectangleShape mOverlayPanel;
    sf::RectangleShape mOverlayDivider;
    sf::Text mOverlayTitle;
    sf::Text mOverlayHint;
    std::vector<std::string> mLevelFiles;
    size_t mCurrentLevelIndex;
    State mState;

    void loadEnemySpawns();
    void resetEnemies();
    void startNewGame();
    void loadCurrentLevel();
    void showLevelCompleteScreen();
    void showVictoryScreen();
    void clearPlayerInput();
    void updateParallax();
    bool hasReachedLevelExit() const;
    bool isHomelanderDefeated() const;
    void configureOverlayText(const sf::String& title, const sf::String& hint, sf::Color titleColor);

public:
    Game();
    void run();
    void processEvents();
    void update();
    void render();
};    

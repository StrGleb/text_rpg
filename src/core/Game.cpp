#include "core/Game.h"

#include <filesystem>
#include <optional>
#include <stdexcept>
#include <vector>

namespace fs = std::filesystem;

namespace {
constexpr float kStompTolerance = 12.f;
constexpr float kOverlayPanelWidth = 620.f;
constexpr float kOverlayPanelHeight = 220.f;
constexpr float kOverlayPanelLeft = 90.f;
constexpr float kOverlayPanelTop = 150.f;
constexpr sf::Color kBottomFillColor(9, 12, 42);
constexpr sf::Color kBottomBorderColor(29, 40, 92);
constexpr sf::Color kOverlayPanelFill(7, 12, 32, 232);
constexpr sf::Color kOverlayPanelOutline(185, 56, 56);
constexpr sf::Color kOverlayPanelShadow(0, 0, 0, 95);
constexpr sf::Color kOverlayHintColor(232, 232, 232);

bool isPlayerStompingEnemy(const Player& player, const Enemy& enemy) {
    const sf::FloatRect playerBounds = player.getBounds();
    const sf::FloatRect enemyBounds = enemy.getBounds();

    const float playerBottom = playerBounds.position.y + playerBounds.size.y;
    const float enemyTop = enemyBounds.position.y;

    return player.getVelocityY() > 0.f && playerBottom <= enemyTop + kStompTolerance;
}

bool loadGameFont(sf::Font& font) {
    const std::vector<fs::path> fontCandidates = {
        fs::path("assets") / "font.ttf",
        fs::path("../assets") / "font.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"
    };

    for (const fs::path& candidate : fontCandidates) {
        if (fs::exists(candidate) && font.openFromFile(candidate.string())) {
            return true;
        }
    }

    return false;
}

void centerTextHorizontally(sf::Text& text, float centerX) {
    const sf::FloatRect bounds = text.getLocalBounds();
    text.setOrigin({
        bounds.position.x + (bounds.size.x / 2.f),
        0.f
    });
    text.setPosition({centerX, text.getPosition().y});
}

void fitTextToWidth(sf::Text& text, float maxWidth, unsigned int maxCharacterSize, unsigned int minCharacterSize) {
    for (unsigned int size = maxCharacterSize; size >= minCharacterSize; --size) {
        text.setCharacterSize(size);
        if (text.getLocalBounds().size.x <= maxWidth || size == minCharacterSize) {
            return;
        }
    }
}

void configurePanel(
    sf::RectangleShape& panel,
    sf::Vector2f position,
    sf::Vector2f size,
    sf::Color fillColor,
    float outlineThickness = 0.f,
    sf::Color outlineColor = sf::Color::Transparent
) {
    panel.setPosition(position);
    panel.setSize(size);
    panel.setFillColor(fillColor);
    panel.setOutlineThickness(outlineThickness);
    panel.setOutlineColor(outlineColor);
}
}

Game::Game()
    : mOverlayBottomFill(),
    mOverlayBottomBorder(),
    mOverlayPanelShadow(),
    mOverlayPanel(),
    mOverlayDivider(),
    mOverlayTitle(mFont),
    mOverlayHint(mFont),
    mCurrentLevelIndex(0),
    mState(State::Menu) {
    mWindow.create(sf::VideoMode({800, 600}), "The Boys");
    mWindow.setFramerateLimit(60); 
    mLevelFiles = {"levels/level1.txt", "levels/level2.txt", "levels/level3.txt"};
    mView.setSize({800.f, 600.f});
    mView.setCenter({400.f, 300.f});

    if (!mCityTexture.loadFromFile("assets/gorod.png")) {
        throw std::runtime_error("Missing gorod.png");
    }

    mCityTexture.setRepeated(true);
    mCitySprite.emplace(mCityTexture);
    mCitySprite->setPosition({0.f, 0.f});
    mCitySprite->setTextureRect(sf::IntRect({0, 0}, {800, 259}));
    mCitySprite->setScale({1.85f, 1.85f});

    if (!loadGameFont(mFont)) {
        throw std::runtime_error("Missing menu/message font");
    }

    mOverlayTitle.setCharacterSize(42);
    mOverlayHint.setCharacterSize(24);
    mOverlayHint.setFillColor(kOverlayHintColor);
    configurePanel(mOverlayBottomFill, {0.f, 468.f}, {800.f, 132.f}, kBottomFillColor);
    configurePanel(mOverlayBottomBorder, {0.f, 468.f}, {800.f, 4.f}, kBottomBorderColor);
    configurePanel(
        mOverlayPanelShadow,
        {kOverlayPanelLeft + 8.f, kOverlayPanelTop + 8.f},
        {kOverlayPanelWidth, kOverlayPanelHeight},
        kOverlayPanelShadow
    );
    configurePanel(
        mOverlayPanel,
        {kOverlayPanelLeft, kOverlayPanelTop},
        {kOverlayPanelWidth, kOverlayPanelHeight},
        kOverlayPanelFill,
        3.f,
        kOverlayPanelOutline
    );
    configurePanel(
        mOverlayDivider,
        {kOverlayPanelLeft + 44.f, kOverlayPanelTop + 112.f},
        {kOverlayPanelWidth - 88.f, 3.f},
        kOverlayPanelOutline
    );

    loadCurrentLevel();
}

void Game::loadEnemySpawns() {
    mEnemySpawns.clear();

    for (const sf::Vector2f& position : mMap.findMarkerGroundPositions('h')) {
        mEnemySpawns.push_back(EnemySpawn{EnemyType::Homelander, position});
    }
    for (const sf::Vector2f& position : mMap.findMarkerGroundPositions('H')) {
        mEnemySpawns.push_back(EnemySpawn{EnemyType::Homelander, position});
    }

    for (const sf::Vector2f& position : mMap.findMarkerGroundPositions('e')) {
        mEnemySpawns.push_back(EnemySpawn{EnemyType::VoughtAgent, position});
    }
    for (const sf::Vector2f& position : mMap.findMarkerGroundPositions('E')) {
        mEnemySpawns.push_back(EnemySpawn{EnemyType::VoughtAgent, position});
    }

    mEnemies.clear();
    mEnemies.reserve(mEnemySpawns.size());
    for (const EnemySpawn& spawn : mEnemySpawns) {
        mEnemies.emplace_back(spawn.type, spawn.position);
    }
}

void Game::startNewGame() {
    mCurrentLevelIndex = 0;
    clearPlayerInput();
    loadCurrentLevel();
    mState = State::Playing;
}

void Game::loadCurrentLevel() {
    mMap.loadFromFile(mLevelFiles[mCurrentLevelIndex]);
    mPlayer.respawn();
    loadEnemySpawns();
    mView.setCenter({400.f, 300.f});
    updateParallax();
}

void Game::showLevelCompleteScreen() {
    clearPlayerInput();
    configureOverlayText(
        "LEVEL " + std::to_string(mCurrentLevelIndex + 1) + " COMPLETE",
        "Left click to continue",
        sf::Color::Green
    );
    mState = State::LevelComplete;
}

void Game::showVictoryScreen() {
    clearPlayerInput();
    configureOverlayText(
        "VICTORY! HOMELANDER DEFEATED!",
        "Left click to return to menu",
        sf::Color(120, 255, 120)
    );
    mState = State::Victory;
}

void Game::clearPlayerInput() {
    mPlayer.setMovingLeft(false);
    mPlayer.setMovingRight(false);
    mPlayer.setJumping(false);
}

void Game::updateParallax() {
    const float viewWidth = mView.getSize().x;
    const float viewLeft = mView.getCenter().x - (viewWidth / 2.f);
    const int textureOffsetX = static_cast<int>(viewLeft * mParallaxFactor);

    if (mCitySprite) {
        mCitySprite->setPosition({0.f, 0.f});
        mCitySprite->setTextureRect(
            sf::IntRect({textureOffsetX, 0}, {static_cast<int>(viewWidth) + 1, 259})
        );
    }
}

bool Game::hasReachedLevelExit() const {
    const std::vector<sf::Vector2f> exits = mMap.findMarkerGroundPositions('3');
    if (exits.empty()) {
        return false;
    }

    const float tileSize = mMap.getTileSize();
    const sf::FloatRect playerBounds = mPlayer.getBounds();

    for (const sf::Vector2f& exitPosition : exits) {
        const sf::FloatRect exitBounds(
            {exitPosition.x - (tileSize * 0.5f), exitPosition.y - (tileSize * 1.5f)},
            {tileSize, tileSize * 1.5f}
        );

        if (playerBounds.findIntersection(exitBounds)) {
            return true;
        }
    }

    return false;
}

bool Game::isHomelanderDefeated() const {
    bool hasHomelander = false;

    for (const Enemy& enemy : mEnemies) {
        if (enemy.getType() != EnemyType::Homelander) {
            continue;
        }

        hasHomelander = true;
        if (enemy.isActive() && !enemy.isDead()) {
            return false;
        }
    }

    return hasHomelander;
}

void Game::configureOverlayText(const sf::String& title, const sf::String& hint, sf::Color titleColor) {
    mOverlayTitle.setString(title);
    mOverlayTitle.setFillColor(titleColor);
    fitTextToWidth(mOverlayTitle, kOverlayPanelWidth - 56.f, 42, 28);
    mOverlayTitle.setPosition({400.f, kOverlayPanelTop + 34.f});
    centerTextHorizontally(mOverlayTitle, 400.f);

    mOverlayHint.setString(hint);
    fitTextToWidth(mOverlayHint, kOverlayPanelWidth - 80.f, 24, 18);
    mOverlayHint.setPosition({400.f, kOverlayPanelTop + 144.f});
    centerTextHorizontally(mOverlayHint, 400.f);
}

void Game::resetEnemies() {
    for (Enemy& enemy : mEnemies) {
        enemy.reset();
    }
}

void Game::run() {
    while (mWindow.isOpen()) {
        processEvents();
        if (mState == State::Menu) {
            mMenu.update(mWindow);
        } else if (mState == State::Playing) {
            update();
        }
        render();
    }
}

void Game::processEvents() {
    while (const std::optional event = mWindow.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            mWindow.close();
        }

        if (mState == State::Menu) {
            if (const auto* mousePressed = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (mousePressed->button == sf::Mouse::Button::Left) {
                    const sf::Vector2i mousePos = sf::Mouse::getPosition(mWindow);
                    if (mMenu.checkPlayClick(mousePos)) {
                        startNewGame();
                    } else if (mMenu.checkExitClick(mousePos)) {
                        mWindow.close();
                    }
                }
            }
            continue;
        }

        if (mState == State::Playing) {
            if (const auto* mousePressed = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (mousePressed->button == sf::Mouse::Button::Left) {
                    mPlayer.setMovingLeft(true);
                }
                if (mousePressed->button == sf::Mouse::Button::Right) {
                    mPlayer.setMovingRight(true);
                }
                if (mousePressed->button == sf::Mouse::Button::Middle) {
                    mPlayer.setJumping(true);
                }
            }

            if (const auto* mouseReleased = event->getIf<sf::Event::MouseButtonReleased>()) {
                if (mouseReleased->button == sf::Mouse::Button::Left) {
                    mPlayer.setMovingLeft(false);
                }
                if (mouseReleased->button == sf::Mouse::Button::Right) {
                    mPlayer.setMovingRight(false);
                }
                if (mouseReleased->button == sf::Mouse::Button::Middle) {
                    mPlayer.setJumping(false);
                }
            }

            continue;
        }

        if (const auto* mousePressed = event->getIf<sf::Event::MouseButtonPressed>()) {
            if (mousePressed->button != sf::Mouse::Button::Left) {
                continue;
            }

            if (mState == State::LevelComplete) {
                ++mCurrentLevelIndex;
                if (mCurrentLevelIndex < mLevelFiles.size()) {
                    loadCurrentLevel();
                    mState = State::Playing;
                } else {
                    showVictoryScreen();
                }
            } else if (mState == State::Victory) {
                mState = State::Menu;
            }
        }
    }
}

void Game::update() {
    const bool playerRespawned = mPlayer.update(mMap);

    if (playerRespawned) {
        resetEnemies();
    } else {
        for (Enemy& enemy : mEnemies) {
            enemy.update(mMap);
        }

        for (Enemy& enemy : mEnemies) {
            if (!enemy.isActive() || enemy.isDead()) {
                continue;
            }

            if (!mPlayer.getBounds().findIntersection(enemy.getBounds())) {
                continue;
            }

            if (isPlayerStompingEnemy(mPlayer, enemy)) {
                enemy.takeDamage();
                mPlayer.bounceFromEnemy();
            } else {
                mPlayer.respawn();
                resetEnemies();
            }

            break;
        }
    }

    if (mState != State::Playing) {
        return;
    }

    const bool isLastLevel = (mCurrentLevelIndex + 1 == mLevelFiles.size());
    if (isLastLevel) {
        if (hasReachedLevelExit() && isHomelanderDefeated()) {
            showVictoryScreen();
            return;
        }
    } else if (hasReachedLevelExit()) {
        showLevelCompleteScreen();
        return;
    }
    
    sf::Vector2f playerPos = mPlayer.getPosition();
    mView.setCenter({playerPos.x, 300.f}); 
    mWindow.setView(mView);
    updateParallax();
}

void Game::render() {
    mWindow.clear(sf::Color::Black);
    mWindow.setView(mWindow.getDefaultView());
    if (mCitySprite) {
        mWindow.draw(*mCitySprite);
    }

    if (mState == State::Menu) {
        mMenu.draw(mWindow);
    } else if (mState == State::Playing) {
        mWindow.setView(mView);
        mMap.draw(mWindow);
        mPlayer.draw(mWindow);
        for (Enemy& enemy : mEnemies) {
            enemy.draw(mWindow);
        }
    } else {
        mWindow.draw(mOverlayBottomFill);
        mWindow.draw(mOverlayBottomBorder);
        mWindow.draw(mOverlayPanelShadow);
        mWindow.draw(mOverlayPanel);
        mWindow.draw(mOverlayDivider);
        mWindow.draw(mOverlayTitle);
        mWindow.draw(mOverlayHint);
    }

    mWindow.display();
}

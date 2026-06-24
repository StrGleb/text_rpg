#include "entities/Player.h"
#include "world/Map.h"
#include <stdexcept>
#include <array>
#include <filesystem>

namespace fs = std::filesystem;

namespace {
constexpr float kGravity = 0.5f;
constexpr float kJumpVelocity = -12.f;
constexpr float kRespawnY = 230.f;
constexpr float kGroundProbeHeight = 2.f;

bool isSolidTile(char tile) {
    return tile == '1' || tile == '2';
}

bool isHazardTile(char tile) {
    return tile == '4';
}

sf::FloatRect getPlayerHitbox(const sf::Sprite& sprite) {
    sf::Vector2f position = sprite.getPosition();

    return sf::FloatRect({position.x - 18.f, position.y - 25.f}, {37.f, 52.f}); // хитбокс персонажа
}

sf::FloatRect getSolidTileBounds(size_t row, size_t col, float tileSize) {
    return sf::FloatRect({col * tileSize, row * tileSize}, {tileSize, tileSize});
}

std::array<sf::FloatRect, 4> getSpikeHitboxes(size_t row, size_t col, float tileSize) {
    const float left = col * tileSize;
    const float top = row * tileSize;

    return {
        sf::FloatRect({left + 17.f, top + 3.f}, {6.f, 7.f}),
        sf::FloatRect({left + 14.f, top + 10.f}, {12.f, 8.f}),
        sf::FloatRect({left + 10.f, top + 18.f}, {20.f, 9.f}),
        sf::FloatRect({left + 6.f, top + 27.f}, {28.f, 11.f})
    };
}

bool touchesSpike(const sf::FloatRect& playerBounds, const Map& map) {
    const float tileSize = map.getTileSize();

    for (size_t row = 0; row < map.getRowCount(); ++row) {
        for (size_t col = 0; col < map.getColCount(); ++col) {
            if (!isHazardTile(map.getTileAt(row, col))) {
                continue;
            }

            for (const sf::FloatRect& spikeHitbox : getSpikeHitboxes(row, col, tileSize)) {
                if (playerBounds.findIntersection(spikeHitbox)) {
                    return true;
                }
            }
        }
    }

    return false;
}

void respawnPlayer(sf::Sprite& sprite, float& velocityY, bool& isGrounded) {
    sprite.setPosition({480.f, kRespawnY});
    velocityY = 0.f;
    isGrounded = false;
}

bool hasGroundUnderPlayer(const sf::Sprite& sprite, const Map& map) {
    const float tileSize = map.getTileSize();
    sf::FloatRect hitbox = getPlayerHitbox(sprite);
    sf::FloatRect probe(
        {hitbox.position.x, hitbox.position.y + hitbox.size.y},
        {hitbox.size.x, kGroundProbeHeight}
    );

    for (size_t row = 0; row < map.getRowCount(); ++row) {
        for (size_t col = 0; col < map.getColCount(); ++col) {
            if (!isSolidTile(map.getTileAt(row, col))) {
                continue;
            }

            sf::FloatRect tileBounds = getSolidTileBounds(row, col, tileSize);
            if (probe.findIntersection(tileBounds)) {
                return true;
            }
        }
    }

    return false;
}
}

Player::Player() : mSprite(mIdleTexture) { 
    fs::path assetsPath = "assets";

    if (!fs::exists(assetsPath)) {
        if (fs::exists("../assets")) {
            assetsPath = "../assets";
        }
    }

    // Загрузка файлов
    std::string idlePath = (assetsPath / "butcher_idle.png").string();
    if (!mIdleTexture.loadFromFile(idlePath)) throw std::runtime_error("Missing butcher_idle.png");

    std::string jumpPath = (assetsPath / "butcher_jump.png").string();
    if (!mJumpTexture.loadFromFile(jumpPath)) throw std::runtime_error("Missing butcher_jump.png");

    mRunTextures.resize(6);
    for (int i = 0; i < 6; ++i) {
        std::string filename = "butcher" + std::to_string(i) + ".png";
        std::string runPath = (assetsPath / filename).string();
        if (!mRunTextures[i].loadFromFile(runPath)) throw std::runtime_error("Missing animation frame");
    }

    mSprite.setTexture(mIdleTexture, true); 

    // Геометрия
    mSprite.setOrigin({20.f, 26.f});
    mSprite.setPosition({140.f, 230.f});

    mCurrentFrame = 0;
    mSpeed = 5.f;
    mVelocityY = 0.f;
    mIsGrounded = false;

    mSprite.setColor(sf::Color::White);
    mSprite.setScale({1.2f, 1.2f});
}

bool Player::update(const Map& map) {
    bool didRespawn = false;

    // ГОРИЗОНТАЛЬНОЕ ДВИЖЕНИЕ
    float moveX = 0.f;
    if (mIsMovingLeft)  moveX -= mSpeed;
    if (mIsMovingRight) moveX += mSpeed;

    mSprite.move({moveX, 0.f});

    float tileSize = map.getTileSize();
    sf::FloatRect playerBounds = getPlayerHitbox(mSprite);

    for (size_t row = 0; row < map.getRowCount(); ++row) {
        for (size_t col = 0; col < map.getColCount(); ++col) {
            if (isSolidTile(map.getTileAt(row, col))) {
                sf::FloatRect tileBounds = getSolidTileBounds(row, col, tileSize);
                if (auto intersection = playerBounds.findIntersection(tileBounds)) {
                    if (moveX > 0.f) mSprite.move({-intersection->size.x, 0.f});
                    if (moveX < 0.f) mSprite.move({intersection->size.x, 0.f});

                    playerBounds = getPlayerHitbox(mSprite);
                }
            }
        }
    }

    // ВЕРТИКАЛЬНОЕ ДВИЖЕНИЕ
    if (mIsJumping && mIsGrounded) {
        mVelocityY = kJumpVelocity;
        mIsGrounded = false;
    }

    if (!mIsGrounded) {
        mVelocityY += kGravity;
    } else {
        mVelocityY = 0.f;
    }

    mSprite.move({0.f, mVelocityY}); 
    playerBounds = getPlayerHitbox(mSprite);
    bool landedThisFrame = false;

    for (size_t row = 0; row < map.getRowCount(); ++row) {
        for (size_t col = 0; col < map.getColCount(); ++col) {
            if (isSolidTile(map.getTileAt(row, col))) {
                sf::FloatRect tileBounds = getSolidTileBounds(row, col, tileSize);
                if (auto intersection = playerBounds.findIntersection(tileBounds)) {
                    if (mVelocityY > 0.f) {
                        mSprite.move({0.f, -intersection->size.y});
                        landedThisFrame = true;
                        mVelocityY = 0.f;
                    } else if (mVelocityY < 0.f) {
                        mSprite.move({0.f, intersection->size.y});
                        mVelocityY = 0.f;
                    }
                    playerBounds = getPlayerHitbox(mSprite);
                }
            }
        }
    }

    mIsGrounded = landedThisFrame || hasGroundUnderPlayer(mSprite, map);

    if (mIsGrounded) {
        mVelocityY = 0.f;
    }

    playerBounds = getPlayerHitbox(mSprite);
    if (touchesSpike(playerBounds, map)) {
        respawnPlayer(mSprite, mVelocityY, mIsGrounded);
        didRespawn = true;
    }

    // Смерть в яме
    if (mSprite.getPosition().y > 560.f) {
        respawnPlayer(mSprite, mVelocityY, mIsGrounded);
        didRespawn = true;
    }

    if (!mIsGrounded) {
        mSprite.setTexture(mJumpTexture); 
    } 
    else if (mIsMovingLeft || mIsMovingRight) {
        if (mAnimationTimer.getElapsedTime().asSeconds() > 0.12f) {
            mCurrentFrame = (mCurrentFrame + 1) % static_cast<int>(mRunTextures.size());
            mSprite.setTexture(mRunTextures[mCurrentFrame]);
            mAnimationTimer.restart();
        }
    } 
    else {
        mSprite.setTexture(mIdleTexture);
        mCurrentFrame = -1;
    }

    // Поворот
    if (mIsMovingLeft)  mSprite.setScale({-1.2f, 1.2f});
    if (mIsMovingRight) mSprite.setScale({1.2f, 1.2f});

    return didRespawn;
}

void Player::draw(sf::RenderWindow& window) {
    window.draw(mSprite);
}

void Player::respawn() {
    respawnPlayer(mSprite, mVelocityY, mIsGrounded);
}

void Player::bounceFromEnemy() {
    mVelocityY = kJumpVelocity * 0.75f;
    mIsGrounded = false;
    mSprite.move({0.f, -6.f});
}

sf::FloatRect Player::getBounds() const {
    return getPlayerHitbox(mSprite);
}

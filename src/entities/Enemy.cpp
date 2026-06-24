#include "entities/Enemy.h"
#include "world/Map.h"

#include <cmath>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

namespace {
bool isSolidTile(char tile) {
    return tile == '1' || tile == '2';
}

bool isDangerousTile(char tile) {
    return tile == '4';
}

struct EnemyConfig {
    const char* textureName;
    int maxHealth;
    float baseSpeed;
    float scale;
    float speedGainOnHit;
    bool reverseOnHit;
};

EnemyConfig getEnemyConfig(EnemyType type) {
    switch (type) {
        case EnemyType::VoughtAgent:
            return EnemyConfig{
                "vought.png",
                2,
                2.f,
                1.f,
                0.f,
                false
            };
        case EnemyType::Homelander:
        default:
            return EnemyConfig{
                "homelander_idle.png",
                10,
                3.f,
                1.2f,
                0.5f,
                true
            };
    }
}

sf::FloatRect getEnemyHitbox(const sf::Sprite& sprite) {
    sf::Vector2f position = sprite.getPosition();
    const float scaleX = std::abs(sprite.getScale().x);
    const float scaleY = std::abs(sprite.getScale().y);
    return sf::FloatRect(
        {position.x - (18.f * scaleX), position.y - (60.f * scaleY)},
        {36.f * scaleX, 60.f * scaleY}
    );
}
}

Enemy::Enemy(EnemyType type, sf::Vector2f startPos)
    : mType(type)
    , mStartPos(startPos)
    , mHealth(0)
    , mIsActive(false)
    , mIsDead(false)
    , mSpeed(0.f)
    , mDirection(-1.f) {
    const EnemyConfig config = getEnemyConfig(mType);

    fs::path assetsPath = "assets";
    if (!fs::exists(assetsPath) && fs::exists("../assets")) {
        assetsPath = "../assets";
    }

    if (!mTexture.loadFromFile((assetsPath / config.textureName).string())) {
        std::cerr << "Текстуры не загружены!\n";
    }

    reset();
}

void Enemy::reset() {
    const EnemyConfig config = getEnemyConfig(mType);

    if (!mSprite) {
        mSprite.emplace(mTexture);
    }

    mSprite->setOrigin({20.f, 60.f});
    mSprite->setPosition(mStartPos);
    mSprite->setScale({-config.scale, config.scale});
    mSprite->setColor(sf::Color::White);

    mHealth = config.maxHealth;
    mIsActive = true;
    mIsDead = false;
    mSpeed = config.baseSpeed;
    mDirection = -1.f;
}

void Enemy::update(const Map& map) {
    if (!mIsActive || mIsDead || !mSprite) {
        return;
    }

    const float moveX = mSpeed * mDirection;
    mSprite->move({moveX, 0.f});

    const float tileSize = map.getTileSize();
    sf::FloatRect bounds = getEnemyHitbox(*mSprite);
    const float scale = getEnemyConfig(mType).scale;

    for (size_t row = 0; row < map.getRowCount(); ++row) {
        for (size_t col = 0; col < map.getColCount(); ++col) {
            const char tile = map.getTileAt(row, col);
            if (isSolidTile(tile) || isDangerousTile(tile)) {
                sf::FloatRect tileBounds(
                    {static_cast<float>(col) * tileSize, static_cast<float>(row) * tileSize},
                    {tileSize, tileSize}
                );
                if (bounds.findIntersection(tileBounds)) {
                    mSprite->move({-moveX, 0.f});
                    mDirection *= -1.f;
                    mSprite->setScale({scale * mDirection, scale});
                    return;
                }
            }
        }
    }

    const float checkX = mSprite->getPosition().x + (mDirection * 24.f);
    const float checkY = mSprite->getPosition().y + 2.f;
    if (checkX < 0.f || checkY < 0.f) {
        mDirection *= -1.f;
        mSprite->setScale({scale * mDirection, scale});
        return;
    }

    const size_t nextCol = static_cast<size_t>(checkX / tileSize);
    const size_t nextRow = static_cast<size_t>(checkY / tileSize);

    const char tileUnderAhead = map.getTileAt(nextRow, nextCol);
    if (!isSolidTile(tileUnderAhead) || isDangerousTile(tileUnderAhead)) {
        mDirection *= -1.f;
        mSprite->setScale({scale * mDirection, scale});
    }
}

void Enemy::takeDamage() {
    if (!mIsActive || mIsDead || !mSprite) {
        return;
    }

    const EnemyConfig config = getEnemyConfig(mType);

    --mHealth;

    if (config.speedGainOnHit > 0.f) {
        mSpeed += config.speedGainOnHit;
    }

    if (config.reverseOnHit) {
        mDirection *= -1.f;
        mSprite->setScale({config.scale * mDirection, config.scale});
    }

    mSprite->setColor(sf::Color(255, 100, 100));

    if (mHealth < 1) {
        mIsDead = true;
    }
}

void Enemy::draw(sf::RenderWindow& window) {
    if (!mIsActive || mIsDead || !mSprite) {
        return;
    }

    if (mSprite->getColor() != sf::Color::White) {
        mSprite->setColor(sf::Color::White);
    }
    window.draw(*mSprite);
}

sf::FloatRect Enemy::getBounds() const {
    if (!mSprite) {
        return sf::FloatRect();
    }

    return getEnemyHitbox(*mSprite);
}

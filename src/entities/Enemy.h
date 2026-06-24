#pragma once

#include <SFML/Graphics.hpp>
#include <optional>

class Map;

enum class EnemyType {
    Homelander,
    VoughtAgent
};

class Enemy {
    sf::Texture mTexture;
    std::optional<sf::Sprite> mSprite;
    EnemyType mType;
    sf::Vector2f mStartPos;

    int mHealth;
    bool mIsActive;
    bool mIsDead;
    float mSpeed;
    float mDirection;

public:
    Enemy(EnemyType type, sf::Vector2f startPos);

    void reset();
    void update(const Map& map);
    void draw(sf::RenderWindow& window);

    void takeDamage();
    EnemyType getType() const { return mType; }
    int getHealth() const { return mHealth; }
    bool isActive() const { return mIsActive; }
    bool isDead() const { return mIsDead; }
    sf::FloatRect getBounds() const;
};

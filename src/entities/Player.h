#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

class Map;

class Player {
    std::vector<sf::Texture> mRunTextures; 
    sf::Texture mIdleTexture;              
    sf::Texture mJumpTexture;              
    sf::Sprite mSprite;                    

    sf::Clock mAnimationTimer; 
    int mCurrentFrame;         

    float mSpeed;
    float mVelocityY;
    bool mIsGrounded;

    bool mIsMovingLeft = false;
    bool mIsMovingRight = false;
    bool mIsJumping = false;

public:
    Player();
    bool update(const Map& map);
    void draw(sf::RenderWindow& window);
    void respawn();
    void bounceFromEnemy();

    void setMovingLeft(bool move) { mIsMovingLeft = move; }
    void setMovingRight(bool move) { mIsMovingRight = move; }
    void setJumping(bool jump) { mIsJumping = jump; }

    sf::Vector2f getPosition() const { return mSprite.getPosition(); }
    sf::FloatRect getBounds() const;
    float getVelocityY() const { return mVelocityY; }
};

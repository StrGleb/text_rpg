#include "Player.h"
#include "Map.h"
#include <iostream>
#include <stdexcept>
#include <filesystem>

namespace fs = std::filesystem;

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

    mRunTextures.resize(4);
    for (int i = 0; i < 4; ++i) {
        std::string filename = "butcher" + std::to_string(i) + ".png";
        std::string runPath = (assetsPath / filename).string();
        if (!mRunTextures[i].loadFromFile(runPath)) throw std::runtime_error("Missing animation frame");
    }

    mSprite.setTexture(mIdleTexture, true); 

    // Геометрия
    mSprite.setOrigin({20.f, 30.f}); 
    mSprite.setPosition({140.f, 230.f});

    mCurrentFrame = 0;
    mSpeed = 5.f;
    mVelocityY = 0.f;
    mIsGrounded = false;
    mWasMovingLastFrame = true; // ИСПРАВЛЕНИЕ: Принудительно ставим true при старте!

    mSprite.setColor(sf::Color::White);
    mSprite.setScale({1.f, 1.f});
}


void Player::update(const Map& map) {
    // 1. ГОРИЗОНТАЛЬНОЕ ДВИЖЕНИЕ
    float moveX = 0.f;
    if (mIsMovingLeft)  moveX -= mSpeed;
    if (mIsMovingRight) moveX += mSpeed;

    mSprite.move({moveX, 0.f});

    float tileSize = map.getTileSize();
    
    // ИСПРАВЛЕНИЕ: Получаем РЕАЛЬНЫЕ автоматические границы спрайта из файла картинки
    sf::FloatRect playerBounds = mSprite.getGlobalBounds();

    for (size_t row = 0; row < map.getRowCount(); ++row) {
        for (size_t col = 0; col < map.getColCount(); ++col) {
            char tile = map.getTileAt(row, col);
            if (tile == '1' || tile == '2' || tile == '4') {
                sf::FloatRect tileBounds({col * tileSize, row * tileSize}, {tileSize, tileSize});
                if (auto intersection = playerBounds.findIntersection(tileBounds)) {
                    if (moveX > 0.f) mSprite.move({-intersection->size.x, 0.f});
                    if (moveX < 0.f) mSprite.move({intersection->size.x, 0.f});
                    
                    playerBounds = mSprite.getGlobalBounds();
                }
            }
        }
    }

    // 2. ВЕРТИКАЛЬНОЕ ДВИЖЕНИЕ И ГРАВИТАЦИЯ
        // 2. ВЕРТИКАЛЬНОЕ ДВИЖЕНИЕ И ГРАВИТАЦИЯ
    if (!mIsGrounded) {
        mVelocityY += 0.5f;
    } else {
        mVelocityY = 0.f;
    }

    if (mIsJumping && mIsGrounded) {
        mVelocityY = -12.f;        
        mIsGrounded = false;
        mIsJumping = false;
    }

    mSprite.move({0.f, mVelocityY}); 
    playerBounds = mSprite.getGlobalBounds();
    mIsGrounded = false;

    for (size_t row = 0; row < map.getRowCount(); ++row) {
        for (size_t col = 0; col < map.getColCount(); ++col) {
            char tile = map.getTileAt(row, col);
            if (tile == '1' || tile == '2' || tile == '4') {
                sf::FloatRect tileBounds({col * tileSize, row * tileSize}, {tileSize, tileSize});
                if (auto intersection = playerBounds.findIntersection(tileBounds)) {
                    if (mVelocityY > 0.f) {
                        // ИСПРАВЛЕНИЕ: Вместо относительного .move() ставим позицию по Y жестко.
                        // Верхняя грань блока земли минус 30 пикселей (половина высоты Бутчера от центра до ботинок)
                        float groundY = row * tileSize;
                        mSprite.setPosition({mSprite.getPosition().x, groundY - 30.f});
                        
                        mIsGrounded = true;
                        mVelocityY = 0.f;
                    } else if (mVelocityY < 0.f) {
                        mSprite.move({0.f, intersection->size.y});
                        mVelocityY = 0.f;
                    }
                    playerBounds = mSprite.getGlobalBounds();
                }
            }
        }
    }

    if (mIsGrounded) {
        mVelocityY = 0.f;
    }

    // Смерть в яме
    if (mSprite.getPosition().y > 560.f) {
        mSprite.setPosition({140.f, 230.f});
        mVelocityY = 0.f; 
        mIsGrounded = false;
    }

    // Финиш
    size_t playerRow = static_cast<size_t>((mSprite.getPosition().y) / tileSize);
    size_t playerCol = static_cast<size_t>((mSprite.getPosition().x) / tileSize);
    if (map.getTileAt(playerRow, playerCol) == '3') {
        mSprite.setPosition({140.f, 230.f});
        mVelocityY = 0.f;
        mIsGrounded = false;
    }

    // ==========================================
    // СТАБИЛЬНЫЙ БЛОК АНИМАЦИИ
    // ==========================================
    if (!mIsGrounded) {
        mSprite.setTexture(mJumpTexture); 
    } 
    else if (mIsMovingLeft || mIsMovingRight) {
        if (mAnimationTimer.getElapsedTime().asSeconds() > 0.12f) {
            mCurrentFrame = (mCurrentFrame + 1) % 4; 
            mSprite.setTexture(mRunTextures[mCurrentFrame]);
            mAnimationTimer.restart();
        }
    } 
    else {
        if (mCurrentFrame != -1) {
            mSprite.setTexture(mIdleTexture);
            mCurrentFrame = -1; 
        }
    }

    // Поворот спрайта
    if (mIsMovingLeft)  mSprite.setScale({-1.f, 1.f});
    if (mIsMovingRight) mSprite.setScale({1.f, 1.f});
}

void Player::draw(sf::RenderWindow& window) {
    window.draw(mSprite);
}

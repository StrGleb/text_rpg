#include "world/Map.h"

#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

namespace {
bool isGroundTile(char tile) {
    return tile == '1' || tile == '2';
}

float findGroundY(const std::vector<std::string>& grid, size_t startRow, size_t col, float tileSize) {
    for (size_t row = startRow; row < grid.size(); ++row) {
        if (col >= grid[row].size()) {
            continue;
        }

        if (isGroundTile(grid[row][col])) {
            return static_cast<float>(row) * tileSize;
        }
    }

    return static_cast<float>(startRow) * tileSize;
}
}

Map::Map() {
    mTileSize = 40.f;

    fs::path assetsPath = "assets";

    if (!fs::exists(assetsPath)) {
        if (fs::exists("../assets")) {
            assetsPath = "../assets";
        } 
        else {
            std::cerr << "Папка assets не найдена!\n";
        }
    }

    if (!mTextures['1'].loadFromFile((assetsPath / "asfalt.png").string())) {
        std::cerr << "Не удалось загрузить asfalt.png\n";
    }
    if (!mTextures['2'].loadFromFile((assetsPath / "kamen_kirpich.png").string())) {
        std::cerr << "Не удалось загрузить kamen_kirpich.png\n";
    }
    if (!mTextures['3'].loadFromFile((assetsPath / "furgon.png").string())){
        std::cerr << "Не удалось загрузить furgon.png\n";
    }
    if (!mTextures['4'].loadFromFile((assetsPath / "ship.png").string())) {
        std::cerr << "Не удалось загрузить ship.png\n";
    }
    if (!mTextures['5'].loadFromFile((assetsPath / "gorod.png").string())) {
        std::cerr << "Не удалось загрузить gorod.png\n";
    }
}

void Map::loadFromFile(const std::string& filename) {
    mGrid.clear();
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Не удалось открыть файл карты: " << filename << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        if (!line.empty() && line.back() == '\n') {
            line.pop_back();
        }

        mGrid.push_back(line);
    }
    
    file.close();
}

void Map::draw(sf::RenderWindow& window) {
    for (size_t row = 0; row < mGrid.size(); ++row) {
        for (size_t col = 0; col < mGrid[row].size(); ++col) {
            char tileType = mGrid[row][col];

            if (!mTextures.count(tileType)) {
                continue;
            }

            const sf::Texture& texture = mTextures.at(tileType);
            sf::Sprite tileSprite(texture);

            if (tileType == '3') {
                sf::Vector2u textureSize = texture.getSize();
                if (textureSize.x == 0 || textureSize.y == 0) {
                    continue;
                }

                const float targetHeight = mTileSize * 1.5f;
                const float scale = targetHeight / static_cast<float>(textureSize.y);

                tileSprite.setOrigin({
                    static_cast<float>(textureSize.x) / 2.f,
                    static_cast<float>(textureSize.y)
                });
                tileSprite.setScale({scale, scale});
                const float groundY = findGroundY(mGrid, row + 1, col, mTileSize);

                tileSprite.setPosition({
                    (static_cast<float>(col) + 0.5f) * mTileSize,
                    groundY
                });
            } else {
                tileSprite.setPosition({
                    static_cast<float>(col) * mTileSize,
                    static_cast<float>(row) * mTileSize
                });
            }

            window.draw(tileSprite);
        }
    }
}

std::optional<sf::Vector2f> Map::findMarkerGroundPosition(char marker) const {
    const std::vector<sf::Vector2f> positions = findMarkerGroundPositions(marker);
    if (!positions.empty()) {
        return positions.front();
    }

    return std::nullopt;
}

std::vector<sf::Vector2f> Map::findMarkerGroundPositions(char marker) const {
    std::vector<sf::Vector2f> positions;

    for (size_t row = 0; row < mGrid.size(); ++row) {
        for (size_t col = 0; col < mGrid[row].size(); ++col) {
            if (mGrid[row][col] != marker) {
                continue;
            }

            const float groundY = findGroundY(mGrid, row + 1, col, mTileSize);
            positions.push_back(sf::Vector2f{
                (static_cast<float>(col) + 0.5f) * mTileSize,
                groundY
            });
        }
    }

    return positions;
}

size_t Map::getRowCount() const { return mGrid.size(); }
size_t Map::getColCount() const { return mGrid.empty() ? 0 : mGrid[0].size(); }
float Map::getTileSize() const { return mTileSize; }

char Map::getTileAt(size_t row, size_t col) const {
    if (row < mGrid.size() && col < mGrid[row].size()) {
        return mGrid[row][col];
    }
    return '0';
}

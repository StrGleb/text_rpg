#include "Map.h"

#include <fstream>
#include <iostream>

Map::Map() {
    mTileSize = 40.f;
    mTile.setSize({mTileSize, mTileSize});
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

            if (tileType == '1') {
                mTile.setFillColor(sf::Color(100, 100, 100));
                mTile.setPosition({col * mTileSize, row * mTileSize});
                window.draw(mTile);
            } else if (tileType == '2') {
                mTile.setFillColor(sf::Color::Green);
                mTile.setPosition({col * mTileSize, row * mTileSize});
                window.draw(mTile);
            } else if (tileType == '3') { // Наш финиш
                mTile.setFillColor(sf::Color::Yellow);
                mTile.setPosition({col * mTileSize, row * mTileSize});
                window.draw(mTile);
            } else if (tileType == '4') { 
                mTile.setFillColor(sf::Color(150, 50, 50)); 
                mTile.setPosition({col * mTileSize, row * mTileSize});
                window.draw(mTile);
            }
        }
    }
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

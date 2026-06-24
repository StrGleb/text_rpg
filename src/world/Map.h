#pragma once
#include <SFML/Graphics.hpp>
#include <optional>
#include <vector>
#include <string>
#include <map>

class Map {
    std::vector<std::string> mGrid;
    float mTileSize;

    std::map<char, sf::Texture> mTextures; 

public:
    Map();
    void loadFromFile(const std::string& filename);
    void draw(sf::RenderWindow& window);
    std::optional<sf::Vector2f> findMarkerGroundPosition(char marker) const;
    std::vector<sf::Vector2f> findMarkerGroundPositions(char marker) const;

    char getTileAt(size_t row, size_t col) const;
    size_t getRowCount() const;
    size_t getColCount() const;
    float getTileSize() const;
};

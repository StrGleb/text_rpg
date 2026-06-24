#pragma once
#include <SFML/Graphics.hpp>

class Menu {
private:
    sf::Font mFont;
    sf::RectangleShape mBottomFill;
    sf::RectangleShape mBottomBorder;
    sf::RectangleShape mTitlePanelShadow;
    sf::RectangleShape mTitlePanel;
    sf::RectangleShape mPlayPanelShadow;
    sf::RectangleShape mPlayPanel;
    sf::RectangleShape mExitPanelShadow;
    sf::RectangleShape mExitPanel;
    sf::Text mTitleText;
    sf::Text mPlayButton;
    sf::Text mExitButton;

public:
    Menu();
    void init();
    void update(const sf::RenderWindow& window);
    void draw(sf::RenderWindow& window);

    // Методы проверки кликов по кнопкам
    bool checkPlayClick(const sf::Vector2i& mousePos) const;
    bool checkExitClick(const sf::Vector2i& mousePos) const;
};

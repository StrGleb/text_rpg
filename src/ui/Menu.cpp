#include "ui/Menu.h"
#include <filesystem>
#include <iostream>
#include <vector>

namespace fs = std::filesystem;

namespace {
constexpr sf::Color kBottomFillColor(9, 12, 42);
constexpr sf::Color kBottomBorderColor(29, 40, 92);
constexpr sf::Color kPanelFillColor(7, 12, 32, 228);
constexpr sf::Color kPanelOutlineColor(185, 56, 56);
constexpr sf::Color kPanelShadowColor(0, 0, 0, 90);
constexpr sf::Color kTitleTextColor(245, 232, 212);
constexpr sf::Color kButtonTextColor(235, 235, 235);
constexpr sf::Color kButtonHoverTextColor(255, 215, 120);
constexpr sf::Color kButtonHoverFillColor(20, 28, 58, 245);

bool loadMenuFont(sf::Font& font) {
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

void centerTextInPanel(sf::Text& text, const sf::RectangleShape& panel, float topPadding) {
    const sf::FloatRect textBounds = text.getLocalBounds();
    const sf::Vector2f panelPos = panel.getPosition();
    const sf::Vector2f panelSize = panel.getSize();

    text.setPosition({
        panelPos.x + (panelSize.x - textBounds.size.x) * 0.5f - textBounds.position.x,
        panelPos.y + topPadding - textBounds.position.y
    });
}
}

Menu::Menu(): 
    mBottomFill(),
    mBottomBorder(),
    mTitlePanelShadow(),
    mTitlePanel(),
    mPlayPanelShadow(),
    mPlayPanel(),
    mExitPanelShadow(),
    mExitPanel(),
    mTitleText(mFont),
    mPlayButton(mFont),
    mExitButton(mFont) {
    init();
}

void Menu::init() {
    if (!loadMenuFont(mFont)) {
        std::cerr << "Не удалось загрузить шрифт для меню\n";
    }

    configurePanel(mBottomFill, {0.f, 468.f}, {800.f, 132.f}, kBottomFillColor);
    configurePanel(mBottomBorder, {0.f, 468.f}, {800.f, 4.f}, kBottomBorderColor);

    configurePanel(mTitlePanelShadow, {156.f, 100.f}, {488.f, 104.f}, kPanelShadowColor);
    configurePanel(mTitlePanel, {148.f, 92.f}, {488.f, 104.f}, kPanelFillColor, 3.f, kPanelOutlineColor);

    configurePanel(mPlayPanelShadow, {292.f, 263.f}, {216.f, 76.f}, kPanelShadowColor);
    configurePanel(mPlayPanel, {284.f, 255.f}, {216.f, 76.f}, kPanelFillColor, 2.f, kPanelOutlineColor);

    configurePanel(mExitPanelShadow, {292.f, 363.f}, {216.f, 76.f}, kPanelShadowColor);
    configurePanel(mExitPanel, {284.f, 355.f}, {216.f, 76.f}, kPanelFillColor, 2.f, kPanelOutlineColor);

    mTitleText.setString("THE BOYS");
    mTitleText.setCharacterSize(52);
    mTitleText.setFillColor(kTitleTextColor);
    centerTextInPanel(mTitleText, mTitlePanel, 18.f);

    mPlayButton.setString("PLAY");
    mPlayButton.setCharacterSize(34);
    mPlayButton.setFillColor(kButtonTextColor);
    centerTextInPanel(mPlayButton, mPlayPanel, 14.f);

    mExitButton.setString("EXIT");
    mExitButton.setCharacterSize(34);
    mExitButton.setFillColor(kButtonTextColor);
    centerTextInPanel(mExitButton, mExitPanel, 14.f);
}

void Menu::update(const sf::RenderWindow& window) {
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    sf::Vector2f mousePosF(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));

    if (mPlayPanel.getGlobalBounds().contains(mousePosF)) {
        mPlayButton.setFillColor(kButtonHoverTextColor);
        mPlayPanel.setFillColor(kButtonHoverFillColor);
    } else {
        mPlayButton.setFillColor(kButtonTextColor);
        mPlayPanel.setFillColor(kPanelFillColor);
    }

    if (mExitPanel.getGlobalBounds().contains(mousePosF)) {
        mExitButton.setFillColor(kButtonHoverTextColor);
        mExitPanel.setFillColor(kButtonHoverFillColor);
    } else {
        mExitButton.setFillColor(kButtonTextColor);
        mExitPanel.setFillColor(kPanelFillColor);
    }
}

void Menu::draw(sf::RenderWindow& window) {
    window.draw(mBottomFill);
    window.draw(mBottomBorder);
    window.draw(mTitlePanelShadow);
    window.draw(mTitlePanel);
    window.draw(mPlayPanelShadow);
    window.draw(mPlayPanel);
    window.draw(mExitPanelShadow);
    window.draw(mExitPanel);
    window.draw(mTitleText);
    window.draw(mPlayButton);
    window.draw(mExitButton);
}

bool Menu::checkPlayClick(const sf::Vector2i& mousePos) const {
    sf::Vector2f mousePosF(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
    return mPlayPanel.getGlobalBounds().contains(mousePosF);
}

bool Menu::checkExitClick(const sf::Vector2i& mousePos) const {
    sf::Vector2f mousePosF(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
    return mExitPanel.getGlobalBounds().contains(mousePosF);
}
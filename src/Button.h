#ifndef BUTTON_H
#define BUTTON_H

#include <SFML/Graphics.hpp>
#include <string>

class Button : public sf::Drawable
{
    float x, y;
    sf::Text text;
    sf::RectangleShape rect;

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    public:
    Button(float x, float y, sf::Text text, sf::RectangleShape rect);
    Button(float x, float y, float width, float height, std::string str, sf::Font &font, unsigned int char_size, sf::Color fg_color, sf::Color bg_color);
    sf::Text getText();
    sf::RectangleShape getRect();
    bool clicked(sf::RenderWindow &window);
};
#endif
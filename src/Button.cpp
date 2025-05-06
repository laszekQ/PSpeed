#include <SFML/Graphics.hpp>
#include "Button.h"

Button::Button(float x, float y, sf::Text text, sf::RectangleShape rect) : text{text}
{
    this->x = x;
    this->y = y;
    this->text = text;
    this->rect = rect;

    this->text.setOrigin(this->text.getLocalBounds().getCenter());
    this->rect.setOrigin(this->rect.getLocalBounds().getCenter());
    
    this->rect.setPosition({x, y});
    this->text.setPosition({x, y});
}

Button::Button(float x, float y, float width, float height, std::string str, sf::Font font, unsigned int char_size, sf::Color fg_color, sf::Color bg_color) : text{font, str, char_size}
{
    this->x = x;
    this->y = y;

    sf::RectangleShape _rect({width, height});
    _rect.setFillColor(bg_color);
    _rect.setOrigin(_rect.getLocalBounds().getCenter());
    _rect.setPosition({x, y});
    rect = _rect;

    this->text.setOrigin(this->text.getLocalBounds().getCenter());
    this->text.setPosition({x, y});
    this->text.setFillColor(fg_color);
}

void Button::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    target.draw(rect);
    target.draw(text);
}

sf::Text Button::getText()
{
    return text;
}

sf::RectangleShape Button::getRect()
{
    return rect;
}

bool Button::clicked(sf::RenderWindow &window)
{
    if(sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
    {
        sf::Vector2f mousePos = (sf::Vector2f)sf::Mouse::getPosition();
        sf::FloatRect button_bounds = rect.getGlobalBounds();

        return button_bounds.contains(mousePos);
    }
    return false;
}
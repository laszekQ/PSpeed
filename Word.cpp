#include "Word.h"
#include <SFML/Graphics.hpp>
#include <string>

Word::Word(const std::string &str, const sf::Font &font, int char_size, const sf::Color &color, float speed)
{
	text.setString(str);
	text.setFont(font);
	text.setFillColor(color);
	text.setCharacterSize(char_size);
	this->speed = speed;
}

sf::Text Word::getText() const
{
	return text;
}

sf::Vector2f Word::getPosition() const
{
	return text.getPosition();
}

void Word::setPosition(float x, float y)
{
	text.setPosition(x, y);
}

void Word::moveRight()
{
	sf::Vector2f pos = text.getPosition();
	text.setPosition(pos.x + speed, pos.y);
}

std::string Word::getString() const
{
	return text.getString();
}

sf::Font Word::getFont() const
{
	return *text.getFont();
}

void Word::setFont(sf::Font font)
{
	text.setFont(font);
}

int Word::getSize()
{
	return text.getCharacterSize();
}

void Word::setSize(int size)
{
	text.setCharacterSize(size);
}

float Word::getSpeed() const
{
	return speed;
}

void Word::setSpeed(float speed)
{
	this->speed = speed;
}

sf::Color Word::getColor() const
{
	return text.getFillColor();
}

void Word::setColor(sf::Color color)
{
	text.setFillColor(color);
}

bool Word::operator==(Word const& word)
{
	return (text.getString() == word.getString());
}
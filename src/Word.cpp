#include "Word.h"
#include <SFML/Graphics.hpp>
#include <string>

Word::Word(const std::string& str, const sf::Font& font, unsigned int char_size, const sf::Color& color, float speed) : text{font, str, char_size}
{
	text.setFillColor(color);
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
	text.setPosition({ x, y });
}

void Word::moveRight()
{
	sf::Vector2f pos = text.getPosition();
	text.setPosition({ pos.x + speed, pos.y });
}

std::string Word::getString() const
{
	return text.getString();
}

void Word::setString(std::string str)
{
	text.setString(str);
}

sf::Font Word::getFont() const
{
	return text.getFont();
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

void Word::accelerate(float acc)
{
	speed *= acc;
}

void Word::speedChange(float speed)
{
	this->speed += speed;
}

void Word::sizeChange(int size)
{
	text.setCharacterSize(text.getCharacterSize() + size);
}

void Word::rotate(float angle)
{
	text.rotate(sf::degrees(angle));
}

sf::Color Word::getColor() const
{
	return text.getFillColor();
}

void Word::setColor(sf::Color color)
{
	text.setFillColor(color);
}

bool Word::operator==(const Word & word) const
{
	return (text.getString() == word.getString());
}

Word Word::operator+=(char c)
{
	text.setString(text.getString() + c);
	return *this;
}

Word Word::operator--(int)
{
	std::string str = text.getString();
	if (str.size() != 0)
		str.erase(str.size() - 1);
	text.setString(str);
	return *this;
}

void Word::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(text);
}
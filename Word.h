#pragma once
#ifndef WORD_H
#define WORD_H

#include <SFML/Graphics.hpp>
#include <string>

class Word 
{
	sf::Text text;
	float speed;
public:
	Word(const std::string &str, const sf::Font &font, int char_size, const sf::Color &color, float speed);

	sf::Text getText() const;

	sf::Vector2f getPosition() const;
	void setPosition(float x, float y);
	void moveRight();

	std::string getString() const;

	sf::Font getFont() const;
	void setFont(sf::Font font);

	int getSize();
	void setSize(int size);

	float getSpeed() const;
	void setSpeed(float speed);

	sf::Color getColor() const;
	void setColor(sf::Color color);

	bool operator==(Word const& word);
};

#endif 


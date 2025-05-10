#ifndef WORD_H
#define WORD_H

#include <SFML/Graphics.hpp>
#include <string>

class Word : public sf::Drawable
{
	sf::Text text;
	float speed = 0.f;
	std::string fontpath;

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
public:
	Word(const std::string& str, const sf::Font& font, unsigned int char_size, const sf::Color& color, float speed);

	void setText(std::string str);
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
	void accelerate(float acc);

	sf::Color getColor() const;
	void setColor(sf::Color color);

	bool operator==(const Word & word) const;
	Word operator+=(char c);
	Word operator--(int);
};

#endif 


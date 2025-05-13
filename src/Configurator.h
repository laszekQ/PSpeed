#ifndef CONFIGURATOR_H
#define CONFIGURATOR_H

#include <fstream>
#include <string>
#include <unordered_map>
#include <SFML/Graphics.hpp>
#include "Word.h"

#include <iostream>

using settings_map = std::unordered_map<std::string, std::string>;

class Configurator 
{
	std::string config_file_path;
	std::string words_file_path;
	settings_map settings;

	std::vector<std::string> words_from_file;
	std::vector<sf::Font> fonts;

	public:
	Configurator();
	Configurator(std::string config_file_path);
	bool setConfiguration();
	settings_map * getConfiguration();
	std::vector<std::string> getWords();
	std::shared_ptr<Word> genWord();
	sf::Font& getFont();
	std::pair<float, float> genPos(int width, int height);

	template <typename T> 
	void changeSetting(std::string key, T value)
	{
		std::string s = std::to_string(value);
		settings.at(key) = s;
	}
	void changeSetting(std::string key, std::string value);
	void switchSetting(std::string key);

	template <typename T>
	void incrementSetting(std::string key, T inc)
	{
		T value = std::stof(settings[key]) + inc;
        this->changeSetting(key, value);
	}
};

#endif
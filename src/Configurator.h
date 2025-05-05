#ifndef CONFIGURATOR_H
#define CONFIGURATOR_H

#include <fstream>
#include <string>
#include <unordered_map>
#include <SFML/Graphics.hpp>
#include "Word.h"

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
	settings_map getConfiguration();
	void changeSetting(std::string key, std::string value);
	std::vector<std::string> getWords();
	std::shared_ptr<Word> genWord();
	static sf::Color genColor();
	std::pair<float, float> genPos(int width, int height);
};

#endif
#include "Configurator.h"
#include "Word.h"
#include "Util.h"
#include <fstream>
#include <string>
#include <map>
#include <ctime>
#include <cstdlib>
#include <filesystem>

using settings_map = std::unordered_map<std::string, std::string>;

Configurator::Configurator()
{
	config_file_path = "../assets/settings.cfg";
}

Configurator::Configurator(std::string config_file_path)
{
	this->config_file_path = config_file_path;
}

bool Configurator::setConfiguration()
{
	try {
		std::ifstream fin(config_file_path);
		while (!fin.eof())
		{
			std::string s, c, v;
			fin >> s >> c >> v;

			settings.insert({ s, v });
		}
		fin.close();
	}
	catch (std::string exc) { return false; }

	words_file_path = settings["words_file"];
	words_from_file = this->getWords();

	//https://www.geeksforgeeks.org/cpp-program-to-get-the-list-of-files-in-a-directory/
	//https://stackoverflow.com/questions/612097/how-can-i-get-the-list-of-files-in-a-directory-using-c-or-c
	try {
		for (const auto & entry : std::filesystem::directory_iterator(settings["fonts_path"]))
		{
			if (entry.is_regular_file()) {
				std::string font_path = entry.path().string();
				sf::Font font(font_path);
				fonts.push_back(font);
			}
		}
	}
	catch (std::string exc) { return false; }

	return true;
}

settings_map * Configurator::getConfiguration()
{
	return &settings;
}

void Configurator::changeSetting(std::string key, std::string value)
{
	settings[key] = value;
}


std::shared_ptr<Word> Configurator::genWord()
{
	int range = words_from_file.size() * std::stod(settings["dispersion"]);
	std::string str = words_from_file[rand() % range];
	
	sf::Font & font = getFont(); //TODO: make default_font always be first in the vector

	unsigned int char_size = 18;
	if (settings["random_word_char_size"] == "1")
		char_size = rand() % 53 + 12; // from 12 to 64
	else
		char_size = std::stoi(settings["base_word_char_size"]);
	
	sf::Color color;
	if (settings["random_words_colors"] == "1")
		color = util::genColor();
	else
	{
		int t_clr = std::stoi(settings["default_words_color"]);
		int r = t_clr / 1000000;
		int g = t_clr / 1000 % 1000;
		int b = t_clr % 1000000;
		color = sf::Color(r, g, b);
	}

	float speed = 10.0f;
	if (settings["random_base_speed"] == "1")
		speed = rand() % 11 + 10; // from 10 to 20
	else
		speed = std::stof(settings["base_speed"]);

	std::shared_ptr<Word> word = std::make_shared<Word>(str, font, char_size, color, speed);
	return word;
}

std::vector<std::string> Configurator::getWords()
{
	std::vector<std::string> words;
	int min_size = std::stoi(settings["minimum_word_size"]);
	int max_size = std::stoi(settings["maximum_word_size"]);

	try {
		std::ifstream fin(words_file_path);
		while (!fin.eof())
		{
			std::string s;
			fin >> s;

			if(s.size() >= min_size && s.size() <= max_size)
				words.push_back(s);
		}
		fin.close();
	}
	catch (std::string exc) { words.push_back(exc); }
	return words;
}

sf::Font& Configurator::getFont()
{
	sf::Font & font = (settings["random_words_fonts"] == "1") ?
						fonts[rand() % fonts.size()]
						: fonts[0];
	return font;
}

std::pair<float, float> Configurator::genPos(int width, int height)
{
	float x = rand() % width * std::stof(settings["starting_words_xpos_maximum"]);
	float y = rand() % height * 0.8;
	return { x , y };
}

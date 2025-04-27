#include "Configurator.h"
#include <fstream>
#include <string>
#include <map>
#include <ctime>
#include <cstdlib>

Configurator::Configurator(std::string config_file_path)
{
	this->config_file_path = config_file_path;
}

bool Configurator::setConfiguration(std::unordered_map<std::string, std::string>& settings)
{
	try {
		std::ifstream fin("settings.config");
		while (!fin.eof())
		{
			std::string s, v;
			fin >> s >> v;

			settings[s] = v;
		}
		fin.close();
	}
	catch (std::string exc) { return false; }
	return true;
}

sf::Color Configurator::genColor()
{
	int r = std::rand() % 256,
		g = std::rand() % 256,
		b = std::rand() % 256;

	return sf::Color(r, g, b);
}

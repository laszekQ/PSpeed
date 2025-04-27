#pragma once
#ifndef CONFIGURATOR_H
#define CONFIGURATOR_H

#include <fstream>
#include <string>
#include <unordered_map>
#include <SFML/Graphics.hpp>

class Configurator 
{
	std::string config_file_path;
public:
	Configurator(std::string config_file_path);
	static bool setConfiguration(std::unordered_map<std::string, std::string>& settings);
	static sf::Color genColor();
};

#endif
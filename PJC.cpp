#include <SFML/Graphics.hpp>
#include "Word.h"
#include "Configurator.h"
#include <string>
#include <unordered_map>
#include <ctime>
#include <cstdlib>
#include <thread>
#include <vector>
#include <chrono>

#include <iostream>

void performLogic(sf::RenderWindow &window, std::vector< std::shared_ptr<Word> > &words, std::shared_ptr<Word> input_word)
{
    std::string input = "";

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            
        }

        bool erased_any = false;
        for (auto p = words.begin(); p != words.end();)
        {
            if (**p == *input_word)
            {
                p = words.erase(p);
                erased_any = true;
            }
            else
            {
                p++;
            }
        }

        if (erased_any)
            input_word->setText("");

        for (auto &word : words)
            word->moveRight();
        std::this_thread::sleep_for(std::chrono::milliseconds(125));
    }
}

int main()
{
    std::srand(std::time(NULL));
    std::unordered_map<std::string, std::string> settings;
    //Configurator::setConfiguration(settings);

    sf::RenderWindow window(sf::VideoMode({1000, 600}), "MonkeyTyper W");

    sf::Font def_font;
    def_font.openFromFile("arial.ttf");

    std::vector< std::shared_ptr<Word> > words;
    std::shared_ptr<Word> input_word = std::make_shared<Word>("100", def_font, 28, sf::Color::White, 0.f);
    input_word->setPosition(100, 500);

    std::string s = "sosal";
    std::shared_ptr<Word> test = std::make_shared<Word>(s, def_font, 64, Configurator::genColor(), 12.f);
    test->setPosition(200, 300);
    words.push_back(test);

    std::jthread compute_thread(performLogic, std::ref(window), std::ref(words), std::ref(input_word));

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }
            else if (const auto* textEntered = event->getIf<sf::Event::TextEntered>())
            {
                if (textEntered->unicode < 128 && textEntered->unicode != 8)
                    *input_word += (char)textEntered->unicode;
                else if (textEntered->unicode == 8)
                    (*input_word)--;
            }
        }

        window.clear();
        for (auto& word : words)
            window.draw(word->getText());
        window.draw(input_word->getText());
        window.display();
    }

    return 0;
}
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

void performLogic(sf::RenderWindow &window, std::vector< std::shared_ptr<Word> > &words)
{
    while (window.isOpen())
    {
        for (auto &word : words)
            word->moveRight();
        std::this_thread::sleep_for(std::chrono::milliseconds(25));
    }
}

int main()
{
    std::srand(std::time(NULL));
    std::unordered_map<std::string, std::string> settings;
    //Configurator::setConfiguration(settings);

    sf::RenderWindow window(sf::VideoMode(1000, 600), "MonkeyTyper W");

    std::vector< std::shared_ptr<Word> > words;

    sf::Font def_font;
    def_font.loadFromFile("arial.ttf");
    std::string s = "Hello, World!";
    std::shared_ptr<Word> test = std::make_shared<Word>(s, def_font, 64, Configurator::genColor(), 12.f);
    test->setPosition(200, 300);
    words.push_back(test);

    std::jthread compute_thread(performLogic, std::ref(window), std::ref(words));

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear();
        for (auto& word : words)
            window.draw(word->getText());
        window.display();
    }

    return 0;
}
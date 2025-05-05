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
#include <mutex>

#include <iostream>

using settings_map_ptr = std::shared_ptr<std::unordered_map<std::string, std::string>>;

enum STATE {
    PLAY, PAUSE, DEFEAT, START
};

struct gameinfo
{
    int score;
    STATE state;
    bool running;
    gameinfo(int sc, STATE st)
    {
        score = sc;
        state = st;
        running = true;
    }
};

void performLogic(
                    std::vector< std::shared_ptr<Word> >& words,
                    std::shared_ptr<Word> input_word,
                    gameinfo& game_info,
                    Configurator &config, 
                    std::mutex &words_mutex)
{
    std::string input = "";
    int k_missed = 0;

    while (game_info.running)
    {
        if (game_info.state == DEFEAT || game_info.state == START)
        {
            k_missed = 0;
            continue;
        }
        else if (game_info.state == PAUSE)
            continue;

        words_mutex.lock();
        bool erased_any = false;
        for (auto p = words.begin(); p != words.end();)
        {
            if ((**p).getPosition().x > 1000)
            {
                k_missed++;
                p = words.erase(p);
                continue;
            }

            if (**p == *input_word)
            {
                game_info.score += (**p).getString().size();
                p = words.erase(p);
                erased_any = true;
            }
            else
            {
                p++;
            }
        }
        words_mutex.unlock();

        if (erased_any)
        {
            input_word->setText("");
            int max_n = std::stoi(config.getConfiguration()["words_added_per_erase_maximum"]);
            int n = rand() % max_n + 1 + k_missed;
            for (int i = 0; i < n; i++)
            {
                words.push_back(config.genWord());
                std::pair<int, int> pos = config.genPos(900, 500);
                words[words.size() - 1]->setPosition(pos.first, pos.second);
            }

            for (auto& word : words)
                word->accelerate(rand() % 10 / 50);

            k_missed = 0;
        }
        else if (k_missed > 3 || words.size() == 0)
            game_info.state = DEFEAT;

        for (auto& word : words)
        {
            word->moveRight();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(125));
    }
}

int main()
{
    std::srand(std::time(NULL));
    Configurator config("settings.cfg");
    config.setConfiguration();

    const int WIDTH = std::stoi(config.getConfiguration()["window_width"]);
    const int HEIGHT = std::stoi(config.getConfiguration()["window_height"]);

    sf::RenderWindow window(sf::VideoMode({WIDTH, HEIGHT}), "PSpeed");
    gameinfo game_info(0, START);

    sf::Font def_font;
    std::string def_font_path = config.getConfiguration()["fonts_path"] + config.getConfiguration()["default_words_font"];
    if (!def_font.openFromFile(def_font_path))
        return -1;

    std::vector< std::shared_ptr<Word> > words;
    std::mutex words_mutex;
    std::shared_ptr<Word> input_word = std::make_shared<Word>("", def_font, 28, sf::Color::White, 0.f);
    input_word->setPosition(WIDTH / 10, 5 * HEIGHT / 6);
    Word input_word_mask("", def_font, 28, sf::Color::Green, 0.f);
    input_word_mask.setPosition(WIDTH / 10, 5 * HEIGHT / 6);
    Word score_word("", def_font, 18, sf::Color::White, 0.f);
    score_word.setPosition(WIDTH / 10, 5.5 * HEIGHT / 6);

    //starting words
    int words_count = std::stoi(config.getConfiguration()["starting_words_count"]);
    for (int i = 0; i < words_count; i++)
    {
        words.push_back(config.genWord());
        std::pair<int, int> pos = config.genPos(window.getSize().x, window.getSize().y);
        words[words.size() - 1]->setPosition(pos.first, pos.second);
    }


    std::jthread compute_thread(performLogic,
        std::ref(words),
        std::ref(input_word),
        std::ref(game_info),
        std::ref(config), 
        std::ref(words_mutex));

    //START and PLAY
    sf::Text greeting_text(def_font, "Press Enter to start", 32);
    greeting_text.setPosition({ (float)window.getSize().x / 2, (float)window.getSize().y / 2});
    sf::Text defeat_text(def_font, "DEFEAT", 64);
    defeat_text.setPosition({ (float)window.getSize().x / 2, (float)window.getSize().y / 2 });
    defeat_text.setFillColor(sf::Color::Red);

    //PAUSE


    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                game_info.running = false;
                compute_thread.request_stop();
                compute_thread.join();
                window.close();
            }
            else
            {
                switch (game_info.state)
                {
                case START:
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter))
                    {
                        game_info.state = PLAY;
                    } 
                    break;

                case PAUSE:
                    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter))
                        game_info.state = PLAY;
                    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape))
                    {
                    //TODO: SETTINGS + SAVE/LOAD
                    }
                    break;

                case PLAY:
                    if (const auto* textEntered = event->getIf<sf::Event::TextEntered>())
                    {
                    if (textEntered->unicode > 40 && textEntered->unicode < 123)
                        *input_word += (char)textEntered->unicode;
                    else if (textEntered->unicode == 8)
                        (*input_word)--;
                    }
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Tab))
                    game_info.state = PAUSE;

                    score_word.setText(std::to_string(game_info.score));

                    for (auto& word : words)
                    {
                    if (word->getString().find(input_word->getString()) != std::string::npos)
                    {
                        input_word_mask.setText(input_word->getString());
                    }
                    }
                    break;

                case DEFEAT:
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter)) // restarting game
                    {
                        words.clear();
                        game_info.score = 0;
                        game_info.state = PLAY;
                        input_word->setText("");

                        words_mutex.lock();
                        for (int i = 0; i < words_count; i++)
                        {
                            words.push_back(config.genWord());
                            std::pair<int, int> pos = config.genPos(window.getSize().x, window.getSize().y);
                            words[words.size() - 1]->setPosition(pos.first, pos.second);
                        }
                        words_mutex.unlock();
                    }
                    break;
                }
            }
        }

        window.clear();

        words_mutex.lock();
        for (auto& word : words)
            window.draw(word->getText());
        words_mutex.unlock();

        if (game_info.state == START)
            window.draw(greeting_text);
        if (game_info.state == DEFEAT)
            window.draw(defeat_text);

        window.draw(input_word->getText());
        window.draw(input_word_mask.getText());
        window.draw(score_word.getText());

        window.display();
    }
    return 0;
}

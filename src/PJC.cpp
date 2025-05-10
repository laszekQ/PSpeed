#include <SFML/Graphics.hpp>
#include "Word.h"
#include "Configurator.h"
#include "Util.h"
#include "Button.h"
#include <string>
#include <unordered_map>
#include <ctime>
#include <cstdlib>
#include <thread>
#include <vector>
#include <chrono>
#include <mutex>

#include <iostream>

using settings_map = std::unordered_map<std::string, std::string>;

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
                game_info.score += (**p).getString().size() * ((**p).getSpeed() / 10);
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
            int max_n = std::stoi((*config.getConfiguration())["words_added_per_erase_maximum"]);
            int n = rand() % max_n + 1 + k_missed;
            for (int i = 0; i < n; i++)
            {
                words.push_back(config.genWord());
                std::pair<float, float> pos = config.genPos(900, 500);
                words[words.size() - 1]->setPosition(pos.first, pos.second);
            }

            for (auto& word : words)
                word->accelerate(std::stof((*config.getConfiguration())["speed_multiplier"]));

            k_missed = 0;
        }
        else if (k_missed > 3 || words.size() == 0)
        {
            game_info.state = DEFEAT;
            util::writeScore(game_info.score);
        }
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
    std::unique_ptr<settings_map> settings = std::make_unique<settings_map>(*config.getConfiguration());

    const unsigned int WIDTH = std::stoi((*settings)["window_width"]);
    const unsigned int HEIGHT = std::stoi((*settings)["window_height"]);

    sf::RenderWindow window(sf::VideoMode({WIDTH, HEIGHT}), "PSpeed");
    gameinfo game_info(0, START);

    sf::Font def_font;
    std::string def_font_path = (*settings)["fonts_path"] + (*settings)["default_words_font"];
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
    int words_count = std::stoi((*settings)["starting_words_count"]);
    for (int i = 0; i < words_count; i++)
    {
        words.push_back(config.genWord());
        std::pair<float, float> pos = config.genPos(window.getSize().x, window.getSize().y);
        words[i]->setPosition(pos.first, pos.second);
    }

    std::jthread compute_thread(performLogic,
        std::ref(words),
        std::ref(input_word),
        std::ref(game_info),
        std::ref(config), 
        std::ref(words_mutex));

    //START and PLAY
    sf::Text greeting_text(def_font, "Press Enter to play", 32);
    greeting_text.setOrigin(greeting_text.getLocalBounds().getCenter());
    greeting_text.setPosition({ window.getSize().x / 2.f, window.getSize().y / 2.f });
    
    //DEFEAT
    sf::Text defeat_text(def_font, "DEFEAT", 64);
    defeat_text.setOrigin(defeat_text.getLocalBounds().getCenter());
    defeat_text.setPosition({ window.getSize().x / 2.f, window.getSize().y / 2.f });
    defeat_text.setFillColor(sf::Color::Red);

    sf::Text score_write_txt(def_font, "To write your score press S", 32);
    score_write_txt.setOrigin(score_write_txt.getLocalBounds().getCenter());
    score_write_txt.setPosition({ window.getSize().x / 2.f, window.getSize().y / 2.f  + 80});
    score_write_txt.setFillColor(sf::Color::White);
    bool score_write = false;

    //PAUSE
    sf::Text best_scores_text(def_font, "Best scores:", 24);
    best_scores_text.setPosition({5.f, 5.f});
    best_scores_text.setFillColor(sf::Color::Yellow);

    Button save_button(WIDTH - 100.f, 175.f, 100.f, 50.f, "Save", def_font, 18, sf::Color(0, 0, 0), sf::Color(255, 255, 255));
    Button open_button(WIDTH - 250.f, 175.f, 100.f, 50.f, "Open", def_font, 18, sf::Color(0, 0, 0), sf::Color(255, 255, 255));
    Word filename_so("", def_font, 24, sf::Color(255, 255, 255), 0.f);
    filename_so.setPosition(WIDTH - 250.f, 120.f);

    std::vector<sf::Text> scores = util::getScores(def_font);

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                util::writeScore(game_info.score);
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
                        game_info.state = PLAY;
                    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Tab))
                        game_info.state = PAUSE;
                break;

                case PAUSE:
                    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter))
                        game_info.state = PLAY;
                    else if (const auto* textEntered = event->getIf<sf::Event::TextEntered>())
                    {
                        if (textEntered->unicode > 40 && textEntered->unicode < 123)
                            filename_so += (char)textEntered->unicode;
                        else if (textEntered->unicode == 8)
                            filename_so--;
                    }
                    else if(sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
                    {
                        words_mutex.lock();
                        if(save_button.clicked(window))
                            util::saveGame(filename_so.getString(), words, input_word, game_info.score);
                        else if(open_button.clicked(window))
                        {
                            util::loadGame(filename_so.getString(), config, words, input_word, game_info.score);
                            util::assignFonts(words, config);
                        }
                        words_mutex.unlock();
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

                    words_mutex.lock();
                    for (auto& word : words)
                    {
                        if (word->getString().find(input_word->getString()) != std::string::npos)
                        {
                            input_word_mask.setText(input_word->getString());
                        }
                    }
                    words_mutex.unlock();
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
                    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) && !score_write)
                    {
                        scores = util::getScores(def_font);
                        score_write = true;
                    }
                    else if(const auto* e = event->getIf<sf::Event::KeyReleased>())
                        if(e->code == sf::Keyboard::Key::S)
                            score_write = false;
                break;
                }
            }
        }
        
        window.clear();
        switch(game_info.state)
        {
            case START:
                window.draw(greeting_text);
            break;

            case PLAY:
                score_word.setText(std::to_string(game_info.score));
                if(input_word->getString() == "")
                        input_word_mask.setText("");

                words_mutex.lock();
                for (auto& word : words)
                    window.draw(*word);
                words_mutex.unlock();

                window.draw(input_word->getText());
                window.draw(input_word_mask.getText());
                window.draw(score_word.getText());
            break;

            case DEFEAT:
                window.draw(defeat_text);
                window.draw(score_write_txt);
            break;

            case PAUSE:
                window.draw(best_scores_text);
                for(auto &t: scores)
                    window.draw(t);
                
                window.draw(save_button);
                window.draw(open_button);
                window.draw(filename_so);
            break;
        }
        window.display();
    }
    return 0;
}

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

using settings_map = std::unordered_map<std::string, std::string>;
using sf::Keyboard::Key;
using namespace sf::Keyboard;

enum STATE {
    PLAY, PAUSE, DEFEAT, START
};

struct gameinfo
{
    int score;
    STATE state;
    bool running = true;
};

void performLogic(  std::vector< std::unique_ptr<Word> > &words,
                    Word &input_word,
                    gameinfo &game_info,
                    Configurator &config, 
                    std::mutex &words_mutex)
{
    settings_map * settings = config.getConfiguration();
    const float degrees = std::stof((*settings)["degrees"]);
    const int allowed_misses = std::stoi((*settings)["allowed_missed_words"]);
    int k_missed = 0;
    int sleeptime = std::stoi((*settings)["sleeptime"]);

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

            if (**p == input_word)
            {
                unsigned int word_length = (**p).getString().size();
                float speed = (**p).getSpeed() / 3.f;
                int rotation_bonus = std::stoi((*settings)["word_rotation"]) * std::stof((*settings)["degrees"]);
                float sleeptime_factor = 40.f / sleeptime;

                game_info.score += (word_length * speed + rotation_bonus) * sleeptime_factor;
                p = words.erase(p);
                erased_any = true;
            }
            else
                p++;
        }
        words_mutex.unlock();

        if (erased_any)
        {
            input_word.setString("");
            int max_n = std::stoi((*settings)["words_added_per_erase_maximum"]);
            int n = util::rand(k_missed, max_n);
            for (int i = 0; i < n; i++)
            {
                words.push_back(config.genWord());
                std::pair<float, float> pos = config.genPos(900, 500);
                words[words.size() - 1]->setPosition(pos.first, pos.second);
            }

            for (auto& word : words)
                word->accelerate(std::stof((*settings)["speed_multiplier"]));

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
            if((*settings)["word_rotation"] == "1")
                word->rotate(degrees);
        }

        sleeptime = std::stoi((*settings)["sleeptime"]);
        std::this_thread::sleep_for(std::chrono::milliseconds(sleeptime));
    }
}

int main()
{
    std::srand(std::time(NULL));
    Configurator config("settings.cfg");
    config.setConfiguration();
    settings_map * settings = config.getConfiguration();

    const unsigned int WIDTH = std::stoi((*settings)["window_width"]);
    const unsigned int HEIGHT = std::stoi((*settings)["window_height"]);

    sf::RenderWindow window(sf::VideoMode({WIDTH, HEIGHT}), "PSpeed");
    window.setFramerateLimit(25);
    gameinfo game_info(0, START);

    sf::Font def_font;
    std::string def_font_path = (*settings)["fonts_path"] + (*settings)["default_words_font"];
    if (!def_font.openFromFile(def_font_path))
        return -1;

    std::vector< std::unique_ptr<Word> > words;
    std::mutex words_mutex;
    
    //PLAY
    Word input_word("", def_font, 28, sf::Color::White, 0.f);
    input_word.setPosition(WIDTH / 10, 5 * HEIGHT / 6);

    Word input_word_mask("", def_font, 28, sf::Color::Green, 0.f);
    input_word_mask.setPosition(WIDTH / 10, 5 * HEIGHT / 6);

    Word score_word("", def_font, 18, sf::Color::White, 0.f);
    score_word.setPosition(WIDTH / 10, 5.5 * HEIGHT / 6);

    sf::RectangleShape input_bg({(float)WIDTH, (float)HEIGHT});
    input_bg.setFillColor(sf::Color(0, 0, 102, 255));
    input_bg.setPosition({0, 5.f * HEIGHT / 6 - 28});

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

    //START
    sf::Text greeting_text(def_font, "Press Enter to play", 32);
    greeting_text.setOrigin(greeting_text.getLocalBounds().getCenter());
    greeting_text.setPosition({ window.getSize().x / 2.f, window.getSize().y / 2.f });
    sf::Text greeting_pause_text(def_font, "And Tab to pause", 24);
    greeting_pause_text.setOrigin(greeting_pause_text.getLocalBounds().getCenter());
    greeting_pause_text.setPosition({ window.getSize().x / 2.f, window.getSize().y / 2.f + 52.f});

    //DEFEAT
    sf::Text defeat_text(def_font, "DEFEAT", 64);
    defeat_text.setOrigin(defeat_text.getLocalBounds().getCenter());
    defeat_text.setPosition({ window.getSize().x / 2.f, window.getSize().y / 2.f });
    defeat_text.setFillColor(sf::Color::Red);

    sf::Text score_write_txt(def_font, "Press S to save your score", 32);
    score_write_txt.setOrigin(score_write_txt.getLocalBounds().getCenter());
    score_write_txt.setPosition({ window.getSize().x / 2.f, window.getSize().y / 2.f  + 65});
    score_write_txt.setFillColor(sf::Color::White);
    bool score_write = false;

    //PAUSE
    sf::Text action_text(def_font, "", 18);
    action_text.setStyle(sf::Text::Italic);
    action_text.setFillColor(sf::Color::White);
    action_text.setPosition({5.f, HEIGHT - 25.f});

    sf::Text best_scores_text(def_font, "Best scores:", 24);
    best_scores_text.setPosition({5.f, 5.f});
    best_scores_text.setFillColor(sf::Color::Yellow);

    Button save_button(WIDTH - 100.f, 175.f, 100.f, 50.f, "Save", def_font, 18, sf::Color(0, 0, 0), sf::Color(255, 255, 255));
    Button open_button(WIDTH - 250.f, 175.f, 100.f, 50.f, "Open", def_font, 18, sf::Color(0, 0, 0), sf::Color(255, 255, 255));
    Word filename_so("", def_font, 24, sf::Color(255, 255, 255), 0.f);
    filename_so.setPosition(WIDTH - 250.f, 120.f);

    sf::Text shortcuts_txt(def_font, "", 18);
    shortcuts_txt.setFillColor(sf::Color::White);
    shortcuts_txt.setPosition({WIDTH - 475.f, 250.f});
    std::string scuts_str = "Shortcuts:\n";
                scuts_str += "Ctrl + Up / Down - word size\n";
                scuts_str += "Ctrl + Left / Right - word speed\n";
                scuts_str += "Ctrl + A / D - accelerate / decelerate\n";
                scuts_str += "Ctrl + C - random word colors\n";
                scuts_str += "Ctrl + F - random word fonts\n";
                scuts_str += "Ctrl + S - random word size\n";
                scuts_str += "Ctrl + P - random word speed\n";
                scuts_str += "Ctrl + N - word rotation\n";
                scuts_str += "Ctrl + Z - erase input\n";
    shortcuts_txt.setString(scuts_str);

    std::vector<sf::Text> scores = util::getScores(def_font);

    bool shortcut_pressed = false;

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                if((*settings)["save_score_on_exit"] == "1")
                    util::writeScore(game_info.score);
                game_info.running = false;
                compute_thread.join();
                window.close();
            }
            else
            {
                switch (game_info.state)
                {
                case START:
                    if (isKeyPressed(Key::Enter))
                        game_info.state = PLAY;
                    else if (isKeyPressed(Key::Tab))
                        game_info.state = PAUSE;
                break;

                case PAUSE:
                    if(isKeyPressed(Key::Enter))
                    {
                        game_info.state = PLAY;
                        action_text.setString("");
                    }
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
                        {
                            if(util::saveGame(filename_so.getString(), words, input_word, game_info.score))
                                action_text.setString("Saved file!");
                            else
                                action_text.setString("Failed to save file.");
                        }
                        else if(open_button.clicked(window))
                        {
                            if(util::loadGame(filename_so.getString(), config, words, input_word, game_info.score))
                                action_text.setString("Opened file!");
                            else
                                action_text.setString("Failed to open file.");
                        }
                        words_mutex.unlock();
                    }
                break;

                case PLAY:
                    if (event->is<sf::Event::TextEntered>())
                    {
                        const auto& textEntered = event->getIf<sf::Event::TextEntered>();
                        if (textEntered->unicode > 40 && textEntered->unicode < 123)
                            input_word += (char)textEntered->unicode;
                        else if (textEntered->unicode == 8)
                            input_word--;
                    }
                    else if(event->is<sf::Event::KeyPressed>())
                    {
                        const auto& keyEvent = event->getIf<sf::Event::KeyPressed>();

                        if (keyEvent->code == Key::Tab)
                            game_info.state = PAUSE;    
                        else if (keyEvent->control && !shortcut_pressed) // if Ctrl is pressed
                        {
                            shortcut_pressed = true;
                            float speed = std::stof((*settings)["base_speed"]);
                            unsigned int char_size = std::stoi((*settings)["base_word_char_size"]);
                            
                            bool g_speed = speed > 0.f;
                            bool g_size = char_size > 4;

                            switch (keyEvent->code)
                            {
                                case Key::Right:
                                    config.incrementSetting("base_speed", 1.f);
                                    util::speedUpWords(words, 1.f);
                                    break;
                                
                                case Key::Left:
                                    if(g_speed)
                                    {
                                        config.incrementSetting("base_speed", -1.f);
                                        util::speedUpWords(words, -1.f);
                                    }
                                    break;
                                case Key::Up:
                                    if(g_size)
                                    {
                                        config.incrementSetting("base_word_char_size", 2);
                                        util::enlargeWords(words, 2);
                                    }
                                    break;
                                case Key::Down:
                                    config.incrementSetting("base_word_char_size", -2);
                                    util::enlargeWords(words, -2);
                                    break;
                                case Key::A:
                                    config.incrementSetting("sleeptime", -5);
                                    break;
                                case Key::D:
                                    config.incrementSetting("sleeptime", 5);
                                    break;
                                case Key::C:
                                    config.switchSetting("random_words_colors");
                                    break;
                                case Key::F:
                                    config.switchSetting("random_words_fonts");
                                    break;
                                case Key::S:
                                    config.switchSetting("random_word_char_size");
                                    break;
                                case Key::P:
                                    config.switchSetting("random_base_speed");
                                    break;
                                case Key::N:
                                    config.switchSetting("word_rotation");
                                    break;
                                case Key::Z:
                                    input_word.setString("");
                                    break;
                                default:
                                    shortcut_pressed = false;
                                    break;
                            }
                        }
                    }
                    words_mutex.lock();
                    for (auto& word : words)
                        if (word->getString().find(input_word.getString()) != std::string::npos)
                            input_word_mask.setString(input_word.getString());
                    words_mutex.unlock();
                break;

                case DEFEAT:
                    if (isKeyPressed(Key::Enter)) // restarting game
                    {
                        words.clear();
                        game_info.score = 0;
                        game_info.state = PLAY;
                        input_word.setString("");

                        words_mutex.lock();
                        for (int i = 0; i < words_count; i++)
                        {
                            words.push_back(config.genWord());
                            std::pair<int, int> pos = config.genPos(window.getSize().x, window.getSize().y);
                            words[words.size() - 1]->setPosition(pos.first, pos.second);
                        }
                        words_mutex.unlock();
                    }
                    else if(isKeyPressed(Key::Tab))
                        game_info.state = PAUSE;
                    else if (isKeyPressed(Key::S) && !score_write)
                    {
                        util::writeScore(game_info.score);
                        scores = util::getScores(def_font);
                        score_write = true;
                    }
                    else if(const auto* e = event->getIf<sf::Event::KeyReleased>())
                        if(e->code == Key::S)
                            score_write = false;
                break;
                }
            }
        }

        shortcut_pressed = false;
        
        window.clear();
        switch(game_info.state)
        {
            case START:
                window.draw(greeting_text);
                window.draw(greeting_pause_text);
            break;

            case PLAY:
                score_word.setString(std::to_string(game_info.score));
                if(input_word.getString() == "")
                        input_word_mask.setString("");

                words_mutex.lock();
                for (auto& word : words)
                    window.draw(*word);
                words_mutex.unlock();

                window.draw(input_bg);
                window.draw(input_word.getText());
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
                window.draw(action_text);
                window.draw(shortcuts_txt);
            break;
        }
        window.display();
    }
    return 0;
}

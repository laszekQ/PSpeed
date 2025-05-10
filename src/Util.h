#ifndef UTIL_H
#define UTIL_H

#include <SFML/Graphics.hpp>
#include "Word.h"
#include "Configurator.h"
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <ctime>

#include <iostream>

namespace util{
    struct record
    {
        std::string date;
        int score;

        record(std::string str, int sc)
        {
            date = str;
            score = sc;
        }
    };

    inline bool recordComp(record r1, record r2)
    {
        return r1.score > r2.score;
    }

    inline void writeScore(int score)
    {
        try{
        std::vector<record> records;
        std::ifstream fin("scores.txt"); //structure: {date}\t{score}\n
        
        while(!fin.eof())
        {
            std::string date;
            int score;
            fin >> date >> score;
            if(date == "")
                    continue;
            records.push_back(record(date, score));
        }
        fin.close();

        //https://forkful.ai/en/cpp/dates-and-times/getting-the-current-date/
        std::time_t t = std::time(0);
        std::tm* now = std::localtime(&t);
        
        std::string year = std::to_string(now->tm_year + 1900);
        std::string month = std::to_string(now->tm_mon + 1);
        std::string day = std::to_string(now->tm_mday);
        std::string date = year + '-' + month + '-' + day;

        records.push_back(record(date, score));
        std::sort(records.begin(), records.end(), recordComp);
        
        std::ofstream fout("scores.txt");
        for(record &r : records)
            fout << r.date << '\t' << r.score << '\n';
        fout.close();

        } catch(const std::exception &e) {throw e;}
    }

    inline std::vector<record> readScore()
    {
        std::vector<record> records;
        try{
            records.reserve(10);

            std::ifstream fin("scores.txt");

            for(int i = 0; i < 10 && !fin.eof(); i++)
            {
                std::string date;
                int score;
                fin >> date >> score;
                if(date == "")
                    continue;
                records.push_back(record(date, score));
            }
            fin.close();

        }catch(const std::exception &e) {throw e;}

        return records;
    }

    inline std::vector<sf::Text> getScores(sf::Font &font)
    {
        std::vector<util::record> records = util::readScore();
        std::vector<sf::Text> scores;
        scores.reserve(records.size());
        for(int i = 0; i < records.size(); i++)
        {
            sf::Text txt(font);
            txt.setString(records[i].date + '\t' + std::to_string(records[i].score));
            txt.setFont(font);
            txt.setCharacterSize(24);
            txt.setFillColor(sf::Color::White);
            txt.setPosition({5.f, 48.f + i * 24.f});
            scores.push_back(txt);
        }
        return scores;
    }

    inline void saveGame(std::string file, std::vector< std::shared_ptr<Word> > &words, std::shared_ptr<Word> &input_word, int score)
    {
        try{
            std::ofstream fout(file);
            
            std::string s = input_word->getString() == "" ? "_" : input_word->getString();

            fout << s << ' ' << score << ' ' << words.size() << '\n';

            for(auto & word : words)
            {
                fout << word->getString() << ' ';
                fout << word->getColor().toInteger() << ' ' << word->getSize() << ' ';
                fout << word->getPosition().x << ' ' << word->getPosition().y << ' ' << word->getSpeed() << '\n';
            }

            fout.close();
        }catch(const std::exception &e) {throw e;}
    }

    inline void loadGame(std::string file, Configurator &config, std::vector< std::shared_ptr<Word> > &words, std::shared_ptr<Word> &input_word, int &score)
    {
        try{
            std::ifstream fin(file);

            std::string str = "";
            fin >> str;
            if(str == "_")
                str = "";
            input_word->setText(str);
            fin >> score;

            int k = 0;
            fin >> k;

            words.clear();
            for(int i = 0; i < k; i++)
            {
                std::string s;
                unsigned int color, char_size;
                float x, y, speed;

                fin >> s >> color >> char_size >> x >> y >> speed;

                words.push_back(std::make_shared<Word>(s, sf::Font(), char_size, sf::Color(color), speed));
                words[i]->setPosition(x, y);
            }
            fin.close();
        }catch(const std::exception &e) {throw e;}
    }

    inline void assignFonts(std::vector< std::shared_ptr<Word> > &words, Configurator &config)
    {
        std::vector<sf::Font> fonts = config.getFonts();

        for(auto &word : words)
        {
            sf::Font & font = ((*config.getConfiguration())["random_words_fonts"] == "1") ?
						fonts[rand() % fonts.size()]
						: fonts[0];
            word->setFont(font);
        }
    }

    inline sf::Color genColor()
    {
        int r = std::rand() % 256,
            g = std::rand() % 256,
            b = std::rand() % 256;

        return sf::Color(r, g, b);
    }
}
#endif
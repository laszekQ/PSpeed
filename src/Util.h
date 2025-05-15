#ifndef UTIL_H
#define UTIL_H

#include <SFML/Graphics.hpp>
#include "Word.h"
#include "Configurator.h"
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <ctime>

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
        std::vector<record> records;
        std::ifstream fin("scores.txt"); //structure: {date}\t{score}\n
        
        if(!fin.is_open())
            return;

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
    }

    inline std::vector<record> readScore()
    {
        std::vector<record> records;
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

    inline bool saveGame(std::string file, std::vector< std::unique_ptr<Word> > &words, Word &input_word, int score)
    {
        try{
            std::ofstream fout(file);
            
            std::string s = input_word.getString() == "" ? "_" : input_word.getString();

            fout << s << ' ' << score << ' ' << words.size() << '\n';

            for(auto & word : words)
            {
                fout << word->getString() << ' ';
                fout << word->getColor().toInteger() << ' ' << word->getSize() << ' ';
                fout << word->getPosition().x << ' ' << word->getPosition().y << ' ' << word->getSpeed() << '\n';
            }

            fout.close();
        }catch(const std::exception &e) {return false;}
        return true;
    }

    inline bool loadGame(std::string file, Configurator &config, std::vector< std::unique_ptr<Word> > &words, Word &input_word, int &score)
    {
        std::ifstream fin(file);

        if(!fin.is_open())
            return false;

        std::string str = "";
        fin >> str;
        if(str == "_")
            str = "";
        input_word.setString(str);
        fin >> score;

        int k = 0;
        fin >> k;

        words.clear();
        for(int i = 0; i < k; i++)
        {
            std::string s;
            unsigned int color, char_size;
            float x, y, speed;
            sf::Font& font = config.getFont();

            fin >> s >> color >> char_size >> x >> y >> speed;

            words.push_back(std::make_unique<Word>(s, font, char_size, sf::Color(color), speed));
            words[i]->setPosition(x, y);
        }
        fin.close();
        return true;
    }

    inline void speedUpWords(std::vector< std::unique_ptr<Word> > &words, float speed)
    {
        for(auto &word : words)
            word->speedChange(speed);
    }
    
    inline void enlargeWords(std::vector< std::unique_ptr<Word> > &words, int size)
    {
        for(auto &word : words)
            word->sizeChange(size);
    }

    inline sf::Color genColor()
    {
        int r = std::rand() % 256,
            g = std::rand() % 256,
            b = std::rand() % 256;

        return sf::Color(r, g, b);
    }

    inline int rand(int l, int r)
    {
        int out = std::rand() % (r - l) + 1 + l;
        return out;
    }
}
#endif
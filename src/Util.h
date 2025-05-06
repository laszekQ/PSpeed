#ifndef UTIL_H
#define UTIL_H

#include <SFML/Graphics.hpp>
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

    inline sf::Color genColor()
    {
        int r = std::rand() % 256,
            g = std::rand() % 256,
            b = std::rand() % 256;

        return sf::Color(r, g, b);
    }
}
#endif
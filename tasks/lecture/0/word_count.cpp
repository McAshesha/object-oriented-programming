#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <map>
#include <algorithm>
#include <iomanip>
#include <cctype>


std::string toLower(const std::string& str)
{
    std::string lowerStr;
    lowerStr.reserve(str.size());
    std::transform(str.begin(), str.end(), std::back_inserter(lowerStr),
                   [](unsigned char c) { return std::tolower(c); });
    return lowerStr;
}

std::list<std::string> extractWords(const std::string& line)
{
    std::list<std::string> words;
    std::string currentWord;

    for (char c : line)
    {
        if (std::isalnum(static_cast<unsigned char>(c)))
        {
            currentWord += c;
        }
        else
        {
            if (!currentWord.empty())
            {
                words.push_back(toLower(currentWord));
                currentWord.clear();
            }
        }
    }
    if (!currentWord.empty())
    {
        words.push_back(toLower(currentWord));
    }
    return words;
}

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        std::cerr << "Usage: word_count.exe input.txt output.csv" << std::endl;
        return 1;
    }

    std::string inputFileName = argv[1];
    std::string outputFileName = argv[2];

    // Чтение строк из входного файла
    std::list<std::string> lines;
    std::ifstream inputFile(inputFileName);
    if (!inputFile.is_open())
    {
        std::cerr << "Error opening input file: " << inputFileName << std::endl;
        return 1;
    }

    std::string line;
    while (std::getline(inputFile, line))
    {
        lines.push_back(line);
    }
    inputFile.close();

    // Подсчет частоты слов
    std::map<std::string, int> wordFrequency;
    for (const auto& l : lines)
    {
        std::list<std::string> words = extractWords(l);
        for (const auto& word : words)
        {
            wordFrequency[word]++;
        }
    }

    // Вычисление общего количества слов
    int totalWords = 0;
    for (const auto& pair : wordFrequency)
    {
        totalWords += pair.second;
    }

    // Сортировка слов по убыванию частоты
    std::list<std::pair<std::string, int>> sortedWords(wordFrequency.begin(), wordFrequency.end());
    sortedWords.sort([](const auto& a, const auto& b)
    {
        return a.second > b.second;
    });

    // Запись результатов в CSV файл
    std::ofstream outputFile(outputFileName);
    if (!outputFile.is_open())
    {
        std::cerr << "Error opening output file: " << outputFileName << std::endl;
        return 1;
    }

    outputFile << "Word,Frequency,Frequency (%)\n";
    for (const auto& [word, count] : sortedWords)
    {
        double percentage = (static_cast<double>(count) / totalWords) * 100.0;
        outputFile << std::quoted(word) << ","
                   << count << ","
                   << std::fixed << std::setprecision(2) << percentage << "%\n";
    }
    outputFile.close();

    return 0;
}

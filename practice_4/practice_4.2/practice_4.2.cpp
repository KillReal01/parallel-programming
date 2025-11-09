/*
    Course: Parallel Programming
    Assignment: 4.2
    Completed by: Bereza Kirill
*/


#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <thread>
#include <latch>
#include <cctype>
#include <map>


void process_letter(char letter, const std::vector<std::string>& words, std::set<std::string>& result, std::latch& done_latch)
{
    for (const auto& word : words)
    {
        if (!word.empty() && std::tolower(word[0]) == letter)
            result.insert(word);
    }
    done_latch.count_down();
}

int main()
{
    std::ifstream file("input.txt");
    if (!file)
    {
        std::cerr << "Cannot open file.txt\n";
        return 1;
    }

    std::vector<std::string> words;
    std::string word;
    while (file >> word)
        words.push_back(word);

    std::vector<char> letters;
    for (char c = 'a'; c <= 'z'; ++c)
        letters.push_back(c);

    std::vector<std::set<std::string>> results(letters.size());
    std::latch done_latch(letters.size());

    for (size_t i = 0; i < letters.size(); ++i)
        std::thread(process_letter, letters[i], std::cref(words), std::ref(results[i]), std::ref(done_latch)).detach();

    done_latch.wait();

    std::map<char, size_t> counts;
    for (size_t i = 0; i < letters.size(); ++i)
        counts[letters[i]] = results[i].size();

    char min_letter = letters.front();
    size_t min_count = counts[min_letter];
    for (const auto& [letter, count] : counts)
    {
        if (count < min_count)
        {
            min_count = count;
            min_letter = letter;
        }
    }

    std::cout << "Least used letter: " << min_letter << " (" << min_count << " words)\n";
    for (const auto& word : results[min_letter - 'a'])
        std::cout << word << "\n";

    return 0;
}

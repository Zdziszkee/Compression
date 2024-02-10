//
// Created by zdziszkee on 1/14/24.
//

#ifndef METADATA_HPP
#define METADATA_HPP
#include <string>

class Metadata {
    std::map<char, unsigned long long> frequencies;
    std::map<char, std::pair<unsigned long long, unsigned long long>> intervals;
    unsigned long long frequencies_sum = 0;
    size_t number_of_symbols = 0;

public:
    explicit Metadata(const std::string& input) {
        for (char character: input) {
            ++frequencies[character];
        }
        number_of_symbols = input.size();
        unsigned long long interval = 0LL;
        for (auto& frequency: frequencies) {
            const char& character = frequency.first;
            const unsigned long long& character_frequency = frequency.second;
            frequencies_sum += character_frequency;
            intervals[character] = std::make_pair(interval, interval + character_frequency - 1);
            interval += character_frequency;
        }
        std::map< unsigned long long,char> map;
        
    }

      std::map<char, unsigned long long>& get_frequencies()  {
        return frequencies;
    }

     std::map<char, std::pair<unsigned long long, unsigned long long>>& get_intervals()  {
        return intervals;
    }

    unsigned long long get_frequencies_sum() const {
        return frequencies_sum;
    }

    size_t get_number_of_symbols() const {
        return number_of_symbols;
    }
};
#endif //METADATA_HPP

//
// Created by zdziszkee on 1/14/24.
//

#ifndef METADATA_HPP
#define METADATA_HPP
#include <string>
#include <unordered_map>

class Metadata {
    std::unordered_map<char, long long> frequencies;
    std::unordered_map<char, std::pair<long long, long long>> intervals;
    long long frequencies_sum = 0;
    size_t number_of_symbols = 0;

public:
    explicit Metadata(const std::string& input) {
        for (char character: input) {
            ++frequencies[character];
        }
        number_of_symbols = input.size();
        long long interval = 0LL;
        for (auto& frequency: frequencies) {
            const char& character = frequency.first;
            const long long& character_frequency = frequency.second;
            frequencies_sum += character_frequency;
            intervals[character] = std::make_pair(interval, interval + character_frequency - 1);
            interval += character_frequency;
        }
    }

    const std::unordered_map<char, long long>& get_frequencies() const {
        return frequencies;
    }

    const std::unordered_map<char, std::pair<long long, long long>>& get_intervals() const {
        return intervals;
    }

    long long get_frequencies_sum() const {
        return frequencies_sum;
    }

    size_t get_number_of_symbols() const {
        return number_of_symbols;
    }
};
#endif //METADATA_HPP

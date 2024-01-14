//
// Created by zdziszkee on 1/13/24.
//

#ifndef ENCODER_HPP
#define ENCODER_HPP
#include <map>
#include <string>
#include <unordered_map>

class Encoder {
    const std::string& input;
    std::map<char, long long> frequencies;
    std::unordered_map<char, std::pair<long long, long long>> intervals;
    long long frequencies_sum = 0;
    long long state = 0;

    void encode_step(const char& character) {
        state = state / frequencies[character] * frequencies_sum + intervals[character].first + state % frequencies[
                    character];
    }

public:
    explicit Encoder(const std::string& input)
        : input(input) {
        for (char character: input) {
            ++frequencies[character];
        }
        long long interval = 0LL;
        for (auto& frequency: frequencies) {
            const char& character = frequency.first;
            const long long& character_frequency = frequency.second;
            frequencies_sum += character_frequency;
            intervals[character] = std::make_pair(interval, interval + character_frequency - 1);
            interval += character_frequency;
        }
    }

    const long long& encode() {
        for (const char& character: input) {
            encode_step(character);
        }
        return state;
    }
};
#endif //ENCODER_HPP

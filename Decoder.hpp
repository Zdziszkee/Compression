//
// Created by zdziszkee on 1/13/24.
//

#ifndef DECODER_HPP
#define DECODER_HPP
#include <algorithm>
#include <map>
#include <stdexcept>
#include <unordered_map>
#include <utility>

class Decoder {
public:
    Decoder(const long long number_of_symbols_in_message, const long long frequency_sum, const long state,
            const std::map<char, long long>& frequencies,
            const std::unordered_map<char, std::pair<long long, long long>>& intervals)
        : number_of_symbols_in_message(number_of_symbols_in_message),
          frequency_sum(frequency_sum),
          state(state),
          frequencies(frequencies),
          intervals(intervals) {
    }

    std::string decode() {
        std::string string;
        for (int i = 0; i < number_of_symbols_in_message; ++i) {
            string += decode_step();
        }
        std::reverse(string.begin(), string.end());
        return string;
    }

private:
    const long long number_of_symbols_in_message;
    const long long frequency_sum;
    long state;
    std::map<char, long long> frequencies;
    std::unordered_map<char, std::pair<long long, long long>> intervals;

    const std::pair<char, std::pair<long long, long long>>& get_interval(const long long r) const {
        for (auto& char_interval: intervals) {
            const auto& interval = char_interval.second;
            if (interval.first <= r && r <= interval.second) {
                return char_interval;
            }
        }
        throw std::runtime_error("There must be always an interval fo char!");
    }

    const char& decode_step() {
        const long long d = state / frequency_sum;
        const long long r = state % frequency_sum;
        const auto& interval = get_interval(r);
        const char& character = interval.first;
        state = d * frequencies[character] + r - interval.second.first;
        return character;
    }
};
#endif //DECODER_HPP

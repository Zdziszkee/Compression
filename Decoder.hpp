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

#include "Metadata.hpp"

class Decoder {
public:
    Decoder(long long state, Metadata& metadata)
        : state(state),
          number_of_symbols_in_message(metadata.get_number_of_symbols()),
          frequency_sum(metadata.get_frequencies_sum()),
          frequencies(metadata.get_frequencies()),
          intervals(metadata.get_intervals()) {
    }

    std::string decode() {
        std::string string = "";
        for (int i = 0; i < number_of_symbols_in_message; ++i) {
            string += decode_step();
        }
        std::reverse(string.begin(), string.end());
        return string;
    }

private:
    const size_t number_of_symbols_in_message;
    const long long frequency_sum;
    long long state;
    std::map<char, long long>& frequencies;
    std::map<char, std::pair<long long, long long>>& intervals;

       std::pair<char, std::pair<long long, long long>> get_interval(const long long r) const {
        for (auto& char_interval: intervals) {
            const auto& interval = char_interval.second;
            if (interval.first <= r && r <= interval.second) {
                return char_interval;
            }
        }
        throw std::runtime_error("There must be always an interval fo char!");
    }

    char decode_step() {
        const long long d = state / frequency_sum;
        const long long r = state % frequency_sum;
        const auto& interval = get_interval(r);
        const char character = interval.first;
        state = d * frequencies[character] + r - interval.second.first;
        return character;
    }
};
#endif //DECODER_HPP

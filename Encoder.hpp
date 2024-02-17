//
// Created by zdziszkee on 1/13/24.
//

#ifndef ENCODER_HPP
#define ENCODER_HPP
#include <map>
#include <string>
#include <unordered_map>

#include "Metadata.hpp"

class Encoder {
    const std::string& input;
    std::map<char, unsigned long long>& frequencies;
    std::map<char, std::pair<unsigned long long, unsigned long long>>& intervals;
    unsigned long long frequencies_sum = 0;
    unsigned long long state = 0;
    std::map<unsigned long long, unsigned long long> encoding_table;
    std::map<unsigned long long, Metadata::DecodingData> decoding_table;

    void encode_step(const char& character) {


        state = state / frequencies[character] * frequencies_sum + intervals[character].first + state % frequencies[
                    character];
    }

public:
    explicit Encoder(const std::string& input, Metadata& metadata)
        : input(input),
          frequencies(metadata.get_frequencies()),
          intervals(metadata.get_intervals()),
          frequencies_sum(metadata.get_frequencies_sum()),
          encoding_table(metadata.encoding_table),
          decoding_table(metadata.decoding_table) {
    }

    unsigned long long encode() {
        for (const char& character: input) {
            encode_step(character);
        }
        return state;
    }
};
#endif //ENCODER_HPP

//
// Created by zdziszkee on 1/14/24.
//

#ifndef METADATA_HPP
#define METADATA_HPP
#include <string>

class Metadata {
    class DecodingData {
    public:
        DecodingData(const char character, const unsigned long long index, const unsigned long long bits)
            : character(character),
              index(index),
              bits(bits) {
        }

        const char character;
        const unsigned long long index;
        const unsigned long long bits;
    };


    std::map<char, unsigned long long> frequencies;
    std::map<char, std::pair<unsigned long long, unsigned long long>> intervals;
    unsigned long long frequencies_sum = 0;
    size_t number_of_symbols = 0;
    /**
     *  state -> decoding data
     */
    std::map<unsigned long long, DecodingData> decoding_table;

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

        unsigned long long index = frequencies_sum;
        std::map<char, unsigned long long> cache;
        for (auto frequency: frequencies) {
            cache.insert(frequency);
        }
        for (char character: input) {
            unsigned long long indexCopy = index;
            unsigned long long bits = 0;
            while (!(indexCopy >= frequencies_sum && indexCopy <= frequencies_sum + input.size())) {
                indexCopy *= 2;
                ++bits;
            }
            DecodingData data = DecodingData(character, index, bits);
            decoding_table.insert(std::make_pair(index, data));
            ++index;
        }
    }

    std::map<char, unsigned long long>& get_frequencies() {
        return frequencies;
    }

    std::map<char, std::pair<unsigned long long, unsigned long long>>& get_intervals() {
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

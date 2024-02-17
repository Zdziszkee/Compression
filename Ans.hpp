//
// Created by zdziszkee on 2/12/24.
//

#ifndef ANS_HPP
#define ANS_HPP
#include <cmath>
#include <map>
#include <string>
#include <utility>
#include <vector>


class Ans {
    struct DecodingData {
        char symbol;
        unsigned bits;
        unsigned new_state;

        DecodingData(char symbol, unsigned bits, unsigned new_state)
            : symbol(symbol), bits(bits), new_state(new_state) {
        }
    };

    std::map<char, unsigned long long> frequencies;
    std::map<char, unsigned long long> frequencies_quantized;
    unsigned long long alphabet_size = 0;
    unsigned long long number_of_symbols = 0;
    unsigned long long L = 1; // range bounds left
    unsigned long long R = 0; // range bound right
    unsigned long long state = 0;
    std::vector<char> symbols_sample_distribution;
    std::map<char, unsigned long long> bit_shifts;
    std::map<char, unsigned long long> intervals;
    std::vector<unsigned long long> encoding_table;
    std::vector<DecodingData> decoding_table;

public:
    std::vector<bool> compress(const std::string& input) {
        for (char character: input) {
            ++frequencies[character];
        }
        number_of_symbols = input.size();
        alphabet_size = frequencies.size();
        /**
         * Set range to be power of 2, and aprox 4 times larger than alphabet size
         */
        while (L < 4 * frequencies.size()) {
            L *= 2; //maybe just multiply frequencies... ?
            R += 1;
        }
        quantize_probabilities_fast();
        spread();


        unsigned long long r = R + 1;

        for (const auto& symbol_frequency: frequencies) {
            char symbol = symbol_frequency.first;
            unsigned long long quantized_frequency = frequencies_quantized[symbol];
            unsigned long long max_bit_shift = R - floor(std::log2(quantized_frequency));
            //calculating how many times multiply by 2 is needed
            unsigned long long bit_shift = (max_bit_shift << r) - (quantized_frequency << max_bit_shift);
            bit_shifts[symbol] = bit_shift;
        }


        size_t size = frequencies.size();
        size_t i = size - 1;
        for (auto outer_iterator = frequencies_quantized.rbegin(); outer_iterator != frequencies_quantized.rend(); ++
             outer_iterator) {
            size_t j = 0;
            char symbol = outer_iterator->first;
            unsigned long long frequency = outer_iterator->second;
            unsigned long long start = -1 * frequency;

            for (auto inner_iterator = frequencies_quantized.begin(); j < i; ++inner_iterator) {
                start += inner_iterator->second; // Accumulate frequencies
                j++;
            }

            intervals[symbol] = start;
            i--;
        }

        encoding_table.resize(L);
        for (int x = L; x < 2 * L; x++) {
            char s = symbols_sample_distribution[x - L];
            encoding_table[intervals[s] + (std::map<char, unsigned long long>(frequencies_quantized)[s])++] = x;
        }

        decoding_table.resize(L);
        for (int x = 0; x < L; x++) {
            char symbol = symbols_sample_distribution[x];
            int frequency = frequencies_quantized[symbol]++;
            int bits = R - floor(log2(frequency));
            int new_x = (frequency << bits) - L;
            decoding_table[x] = DecodingData(symbol, bits, new_x);
        }
    }

    void quantize_probabilities_fast() {
        unsigned long long cummulative_frequency = 0;
        char max_probability_symbol;
        double max_probability = 0;

        for (const auto& symbol_frequency: frequencies) {
            char symbol = symbol_frequency.first;
            unsigned long long frequency = symbol_frequency.second;
            double probability = frequency / number_of_symbols;

            frequencies_quantized[symbol] = std::round(L * probability);
            //put 1 if 0
            if (!frequencies_quantized[symbol]) {
                frequencies_quantized[symbol]++;
            }
            cummulative_frequency += frequencies_quantized[symbol];

            if (probability > max_probability) {
                max_probability = probability;
                max_probability_symbol = symbol;
            }
        }
        frequencies_quantized[max_probability_symbol] += L - cummulative_frequency;
    }

    void spread() {
        symbols_sample_distribution.resize(L);
        state = L;
        unsigned long long i = 0;
        unsigned long long step = (L >> 1LL) + (L >> 3LL) + 3LL;
        unsigned long long mask = L - 1;

        for (std::pair<char, unsigned long long> symbol_frequency: frequencies) {
            char symbol = symbol_frequency.first;
            unsigned long long quantized_frequency = frequencies_quantized[symbol];
            for (unsigned long long j = 0; j < quantized_frequency; j++) {
                symbols_sample_distribution[i] = symbol;
                i = (i + step) & mask;
            }
        }
    }
};


#endif //ANS_HPP

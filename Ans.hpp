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
    std::map<char, unsigned long long> frequencies;
    std::map<char, unsigned long long> frequencies_quantized;
    unsigned long long alphabet_size = 0;
    unsigned long long number_of_symbols = 0;
    unsigned long long L = 1; // range bounds left
    unsigned long long R = 0; // range bound right
    unsigned long long state = 0;
    std::vector<char> symbols_row;
    std::map<char, unsigned long long> bits_row;
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
            unsigned long long bits = R - floor(std::log2(quantized_frequency)); //calculating how many times multiply by 2
            unsigned long long symbol_bits = (bits << r) - (quantized_frequency << bits);
            bits_row[symbol] = symbol_bits;
        }


        size_t size = frequencies.size();

        for (size_t i = size - 1; i >= 0; i--) {
            Pair *current = symbol_data.at(i);
            char symbol = current->first;
            int L_r = ls_map[symbol];
            int start = -1 * L_r;
            for (int j = 0; j < i; j++) {
                char symbol_prim = symbol_data.at(j)->first;
                int L_r_prim = ls_map[symbol_prim];
                start += L_r_prim;
            }
            symbol_start[symbol] = start;
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
        symbols_row.resize(L);
        state = L;
        unsigned long long i = 0;
        unsigned long long step = (L >> 1LL) + (L >> 3LL) + 3LL;
        unsigned long long mask = L - 1;

        for (std::pair<char, unsigned long long> symbol_frequency: frequencies) {
            char symbol = symbol_frequency.first;
            unsigned long long quantized_frequency = frequencies_quantized[symbol];
            for (unsigned long long j = 0; j < quantized_frequency; j++) {
                symbols_row[i] = symbol;
                i = (i + step) & mask;
            }
        }
    }
};


#endif //ANS_HPP

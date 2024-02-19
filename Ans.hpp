//
// Created by zdziszkee on 2/12/24.
//

#ifndef ANS_HPP
#define ANS_HPP
#include <cmath>
#include <map>
#include <string>
#include <vector>


class Ans {
    struct DecodingData {
        char symbol;
        int bits;
        int new_state;

        DecodingData(char symbol, int bits, int new_state)
            : symbol(symbol), bits(bits), new_state(new_state) {
        }
    };

    std::map<char, int> frequencies;
    std::map<char, int> frequencies_quantized;
    int alphabet_size = 0;
    size_t number_of_symbols = 0;
    int L = 1; // range bounds left
    int R = 0; // range bound right
    int r = R +1;
    int starting_state = 0;
    std::vector<char> symbols_sample_distribution;
    std::map<char, int> bit_shifts;
    std::map<char, int> intervals;
    std::vector<int> encoding_table;
    std::vector<DecodingData *> decoding_table;

public:
    void quantize_probabilities_fast() {
        int used = 0;
        char max_proba_symbol;
        double max_proba = 0;
        for (const std::pair<char, int> symbol_frequency: frequencies) {
            std::cout << symbol_frequency.first;
            char symbol = symbol_frequency.first;
            double proba = (double) symbol_frequency.second / (double) number_of_symbols;
            frequencies_quantized[symbol] = std::round(L * proba);

            if (!frequencies_quantized[symbol])
                frequencies_quantized[symbol]++;
            used += frequencies_quantized[symbol];

            if (proba > max_proba) {
                max_proba = proba;
                max_proba_symbol = symbol;
            }
        }
        std::cout << std::endl;
        frequencies_quantized[max_proba_symbol] += L - used;
    }


    void spread() {
        symbols_sample_distribution.resize(L);
        starting_state = L;
        int i = 0;
        int step = (L >> 1) + (L >> 3) + 3;
        int mask = L - 1;

        for (const std::pair<char, int> symbol_frequency: frequencies_quantized) {
            char symbol = symbol_frequency.first;
            int frequency = symbol_frequency.second;
            for (int j = 0; j < frequency; j++) {
                symbols_sample_distribution[i] = symbol;
                i = (i + step) & mask;
            }
        }
    }

    void generate_nb_bits() {
        for (const std::pair<char, int> symbol_frequency: frequencies_quantized) {
            const char symbol = symbol_frequency.first;
            const int frequency = symbol_frequency.second;
            const int bits = R - static_cast<int>(floor(log2(frequency)));
            const int nb_val = (bits << r) - (frequency << bits);
            bit_shifts[symbol] = nb_val;
        }
    }

    void generate_start() {
        int vocab_size = static_cast<int>(frequencies.size());

        auto outer_iterator = frequencies_quantized.rbegin();
        for (int i = vocab_size - 1; i >= 0; i--) {
            const char symbol = outer_iterator->first;
            const int frequency = frequencies_quantized[symbol];
            int start = -1 * frequency;
            auto inner_iterator = frequencies_quantized.begin();
            for (int j = 0; j < i; j++) {
                char symbol_prim = inner_iterator->first;
                const int frequency_prim = frequencies_quantized[symbol_prim];
                start += frequency_prim;
                ++inner_iterator;
            }
            intervals[symbol] = start;
            ++outer_iterator;
        }
    }

    void generate_encoding_table() {
        std::map<char, int> next = frequencies_quantized;
        encoding_table.resize(L);

        for (int x = L; x < 2 * L; x++) {
            const char s = symbols_sample_distribution[x - L];
            encoding_table[intervals[s] + (next[s])++] = x;
        }
    }

    void generate_decoding_table() {
        std::map<char, int> next = frequencies_quantized;
        decoding_table.resize(L);

        for (int x = 0; x < L; x++) {
             const char symbol = symbols_sample_distribution[x];
            const int n = next[symbol]++;
            const int nb_bits = R - static_cast<int>(floor(log2(n)));
            const int new_x = (n << nb_bits) - L;
            auto* t = new DecodingData(symbol, nb_bits, new_x);
            decoding_table[x] = t;
        }
    }

    static int get_extractor(int exp) {
        return (1 << exp) - 1;
    }

    static void use_bits(std::vector<bool>& message, int state, int nb_bits) {
       const int n_to_extract = get_extractor(nb_bits);
        int least_significant_bits = state & n_to_extract;

        for (int i = 0; i < nb_bits; i++, least_significant_bits >>= 1)
            message.push_back((least_significant_bits & 1));
    }

    void write_state(std::vector<bool>& message, int state) const {
        for (int i = 0; i < r; i++, state >>= 1)
            message.push_back((state & 1));
    }

    std::vector<bool> encode(std::string message) {
        number_of_symbols = message.size();
        for (char character: message) {
            frequencies[character] = frequencies[character] + 1;
        }

        L = 1;
        R = 0;

        while (4 * frequencies.size() > L) {
            L *= 2;
            R += 1;
        }
        r = R + 1;

        quantize_probabilities_fast();
        spread();
        generate_nb_bits();
        generate_start();
        generate_encoding_table();
        generate_decoding_table();

        std::vector<bool> result;
        int state = starting_state;
        for (int i = static_cast<int>(number_of_symbols - 1); i >= 0; --i) {
            char symbol = message[i];
            int nb_bits = (state + bit_shifts[symbol]) >> r;
            use_bits(result, state, nb_bits);
            state = encoding_table[intervals[symbol] + (state >> nb_bits)];
        }

        write_state(result, state);
        return result;
    }

    int read_decoding_state(std::vector<bool>& message) const {
        std::vector<bool> state_vec;

        for (int i = 0; i < r; i++) {
            state_vec.push_back(message.back());
            message.pop_back();
        }

      return  bits_to_int(state_vec);
    }

    static int update_decoding_state(std::vector<bool>& message, int nb_bits, int new_x) {
        int accumulate_threshold = 1;
        std::vector<bool> state_vec;
        int x_add;

        for (int i = 0; i < nb_bits; i++) {
            state_vec.push_back(message.back());
            message.pop_back();
        }

        if (state_vec.size() > accumulate_threshold) {
            x_add = bits_to_int(state_vec);
        } else {
            x_add = state_vec.at(0);
        }

        return new_x + x_add;
    }


    std::string decode(std::vector<bool>& message) {
        std::string output;
        int x_start = read_decoding_state(message);
        DecodingData* t = decoding_table.at(x_start - L);

        while (!message.empty()) {
            output.push_back(t->symbol);
            x_start = update_decoding_state(message, t->bits, t->new_state);
            t = decoding_table.at(x_start);
        }

        return output;
    }

    static int bits_to_int(std::vector<bool>& bits) {
        int result = 0;
        for (const bool bit: bits) {
            result = (result << 1) | bit;
        }
        return result;
    }
};


#endif //ANS_HPP

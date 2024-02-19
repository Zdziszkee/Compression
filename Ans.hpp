//
// Created by zdziszkee on 2/12/24.
//

#ifndef ANS_HPP
#define ANS_HPP
#include <cmath>
#include <map>
#include <numeric>
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

    std::map<char, int> frequencies;
    std::map<char, int> frequencies_quantized;
    int alphabet_size = 0;
    int number_of_symbols = 0;
    int L = 1; // range bounds left
    int R = 0; // range bound right
    int starting_state = 0;
    std::vector<char> symbols_sample_distribution;
    std::map<char, int> bit_shifts;
    std::map<char, int> intervals;
    std::vector<int> encoding_table;
    std::vector<DecodingData*> decoding_table;

public:


void set_table_size() {
    L = 1;
    R = 0;
    int vocab_size = frequencies.size();

    while (L < 4 * vocab_size) {
        L *= 2;
        R += 1;
    }
}

// https://github.com/JarekDuda/AsymmetricNumeralSystemsToolkit/blob/master/ANStoolkit.cpp
void quantize_probabilities_fast() {
    int used = 0;
    char max_proba_symbol;
    double max_proba = 0;
    for (const std::pair<char, int> symbol_frequency: frequencies) {
        std::cout << symbol_frequency.first;
        char symbol = symbol_frequency.first;
        double proba = (double) symbol_frequency.second / (double)number_of_symbols;
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
        int L_s = symbol_frequency.second;
        for (int j = 0; j < L_s; j++) {
            symbols_sample_distribution[i] = symbol;
            i = (i + step) & mask;
        }
    }

}

void generate_nb_bits() {
    int r = R + 1;

    for (const std::pair<char, int> symbol_frequency: frequencies_quantized) {
        char symbol = symbol_frequency.first;
        int L_s = symbol_frequency.second;
        int k_s = R - floor(log2(L_s));
        int nb_val = (k_s << r) - (L_s << k_s);
        bit_shifts[symbol] = nb_val;
    }

}

void generate_start() {
    int vocab_size = frequencies.size();

    auto outer_iterator = frequencies_quantized.rbegin();
    for (int i = vocab_size - 1; i >= 0; i--) {
        char symbol = outer_iterator->first;
        int L_r = frequencies_quantized[symbol];
        int start = -1 * L_r;
        auto inner_iterator = frequencies_quantized.begin();
        for (int j = 0; j < i; j++) {
            char symbol_prim = inner_iterator->first;
            int L_r_prim = frequencies_quantized[symbol_prim];
            start += L_r_prim;
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
        char s = symbols_sample_distribution[x - L];
        encoding_table[intervals[s] + (next[s])++] = x;
    }
}

void generate_decoding_table() {
    std::map<char, int> next = frequencies_quantized;
    decoding_table.resize(L);

    for (int x = 0; x < L; x++) {
        char symbol = symbols_sample_distribution[x];
        int n = next[symbol]++;
        int nb_bits = R - floor(log2(n));
        int new_x = (n << nb_bits) - L;
        DecodingData* t = new DecodingData(symbol, nb_bits, new_x);
        decoding_table[x] = t;
    }
}

int get_extractor(int exp) {
    return (1 << exp) - 1;
}

void use_bits(std::vector<bool>& message, int state, int nb_bits) {
    int n_to_extract = get_extractor(nb_bits);
    int least_significant_bits = state & n_to_extract;

    for (int i = 0; i < nb_bits; i++, least_significant_bits >>= 1)
        message.push_back((least_significant_bits & 1));
}

void output_state(std::vector<bool>& message, int state) {
    for (int i = 0; i < R + 1; i++, state >>= 1)
        message.push_back((state & 1));
}

std::vector<bool> encode(std::string message) {
    number_of_symbols = message.size();
    for (char character: message) {
        frequencies[character] = frequencies[character] + 1;
    }
    for (auto pair: frequencies) {
        std::cout << pair.first;
        auto make_pair = std::pair(pair.first, (double) pair.second / (double) number_of_symbols);
    }

    std::cout << std::endl;;
    create_tables();
    std::vector<bool> result;
    int r = R + 1;
    int state = starting_state;
    int len = message.length();

    for (int i = 0; i < len; i++) {
        char symbol = message[i];
        int nb_bits = (state + bit_shifts[symbol]) >> r;
        use_bits(result, state, nb_bits);
        state = encoding_table[intervals[symbol] + (state >> nb_bits)];
    }

    output_state(result, state);
    return result;
}

int read_decoding_state(std::vector<bool>& message) {
    int r = R + 1;
    std::vector<bool> state_vec;

    for (int i = 0; i < r; i++) {
        state_vec.push_back(message.back());
        message.pop_back();
    }

    int x_start = bits_to_int(state_vec);
    return x_start;
}

int update_decoding_state(std::vector<bool>& message, int nb_bits, int new_x) {
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

void create_tables() {
    set_table_size();
    quantize_probabilities_fast();
    spread();
    generate_nb_bits();
    generate_start();
    generate_encoding_table();
    generate_decoding_table();
}

std::string decode(std::vector<bool>& message) {
    std::string output = "";
    int x_start = read_decoding_state(message);
    DecodingData* t = decoding_table.at(x_start - L);

    while (message.size()) {
        output.push_back(t->symbol);
        x_start = update_decoding_state(message, t->bits, t->new_state);
        t = decoding_table.at(x_start);
    }

    std::reverse(output.begin(), output.end());
    return output;
}
int bits_to_int(std::vector<bool>& bits) {
    int result = 0;
    for (bool bit : bits) {
        result = (result << 1) | bit;
    }
    return result;
}
};


#endif //ANS_HPP

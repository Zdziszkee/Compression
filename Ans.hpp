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
        int max_bit_shift;
        int new_state;

        DecodingData(char symbol, int max_bit_shift, int new_state)
            : symbol(symbol), max_bit_shift(max_bit_shift), new_state(new_state) {
        }
    };

    std::map<char, int> frequencies;
    std::map<char, int> frequencies_quantized;
    int alphabet_size = 0;
    size_t number_of_symbols = 0;
    int L = 1; // number of states in finate state machine (equal to sum of quantized frequencies)
    int R = 0;
    int r = R + 1;
    int starting_state = 0;
    std::vector<char> symbols_sample_distribution;
    std::map<char, int> bit_shifts;
    std::map<char, int> intervals;
    std::vector<int> encoding_table;
    std::vector<DecodingData *> decoding_table;

public:
    //stolen from jarek duda
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

    //stolen from jarek duda
    void spread() {
        symbols_sample_distribution.resize(L);
        starting_state = L;
        int i = 0;
        int step = (L >> 1) + (L >> 3) + 3;
        int mask = L - 1;

        for (const std::pair<char, int> symbol_frequency: frequencies_quantized) {
            const char symbol = symbol_frequency.first;
            const int frequency = symbol_frequency.second;
            for (int j = 0; j < frequency; j++) {
                symbols_sample_distribution[i] = symbol;
                i = (i + step) & mask;
            }
        }
    }

    void create_bit_shifts_row() {
        for (const std::pair<char, int> symbol_frequency: frequencies_quantized) {
            const char symbol = symbol_frequency.first;
            const int frequency = symbol_frequency.second;
            const int max_bit_shift = R - static_cast<int>(floor(log2(frequency))); //wyznacza maksymalną liczbę bitów, o którą można przesunąć stanx tak, aby wciąż należał do[Ls,2Ls-1]
            const int bit_shift = (max_bit_shift << r) - (frequency << max_bit_shift);
            bit_shifts[symbol] = bit_shift;
        }
    }

    void create_symbol_intervals() {
        int size = static_cast<int>(frequencies.size());
        auto outer_iterator = frequencies_quantized.rbegin();
        for (int i = size - 1; i >= 0; i--) {
            const char symbol = outer_iterator->first;
            const int frequency = frequencies_quantized[symbol];
            int start = -1 * frequency;
            auto inner_iterator = frequencies_quantized.begin();
            for (int j = 0; j < i; j++) {
                char symbol_prim = inner_iterator->first;
                const int second_frequency = frequencies_quantized[symbol_prim];
                start += second_frequency;
                ++inner_iterator;
            }
            intervals[symbol] = start;
            ++outer_iterator;
        }
    }

    void generate_encoding_table() {
        std::map<char, int> frequencies_copy = frequencies_quantized;
        encoding_table.resize(L);

        for (int x = L; x < 2 * L; x++) {
            const char s = symbols_sample_distribution[x - L];
            encoding_table[intervals[s] + frequencies_copy[s]] = x;
            frequencies_copy[s]++;
        }
    }

    void create_decoding_table() {
        std::map<char, int> frequencies_copy = frequencies_quantized;
        decoding_table.resize(L);

        for (int x = 0; x < L; x++) {
            const char symbol = symbols_sample_distribution[x];
            const int frequency = frequencies_copy[symbol];
            const int max_bit_shift = R - static_cast<int>(floor(log2(frequency)));
            const int new_state = (frequency << max_bit_shift) - L;
            auto* decoding_data = new DecodingData(symbol, max_bit_shift, new_state);
            decoding_table[x] = decoding_data;
            frequencies_copy[symbol]++;
        }
    }


    static void emit_bits(std::vector<bool>& buffer, int state, int bits) {
        const int bits_to_extract_mask = (1 << bits) - 1;
        int least_significant_bits = state & bits_to_extract_mask;

        for (int i = 0; i < bits; i++, least_significant_bits >>= 1)
            buffer.push_back((least_significant_bits & 1));
    }

    void write_state(std::vector<bool>& buffer, int state) const {
        for (int i = 0; i < r; i++, state >>= 1)
            buffer.push_back((state & 1));
    }

    std::vector<bool> encode(std::string text) {
        number_of_symbols = text.size();
        for (char character: text) {
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
        create_bit_shifts_row();
        create_symbol_intervals();
        generate_encoding_table();
        create_decoding_table();

        std::vector<bool> buffer;
        int state = starting_state;
        for (int i = static_cast<int>(number_of_symbols - 1); i >= 0; --i) {
            char symbol = text[i];
            int number_of_bits_to_emit = (state + bit_shifts[symbol]) >> r;
            emit_bits(buffer, state, number_of_bits_to_emit);
            state = encoding_table[intervals[symbol] + (state >> number_of_bits_to_emit)];
        }

        write_state(buffer, state);
        return buffer;
    }

    int read_decoding_state(std::vector<bool>& buffer) const {
        std::vector<bool> state_buffer;

        for (int i = 0; i < r; i++) {
            state_buffer.push_back(buffer.back());
            buffer.pop_back();
        }

        return bits_to_int(state_buffer);
    }

    static int update_decoding_state(std::vector<bool>& buffer, int nb_bits, int new_x) {
        if (nb_bits > buffer.size()) {
            nb_bits = static_cast<int>(buffer.size()); // Prevent out-of-bounds access
        }

        int x_add = 0;
        for (int i = 0; i < nb_bits; i++) {
            x_add = (x_add << 1) | buffer.back();
            buffer.pop_back();
        }

        return new_x + x_add;
    }


    std::string decode(std::vector<bool>& buffer) {
        std::string output;
        int x_start = read_decoding_state(buffer);
        DecodingData* decoding_data = decoding_table.at(x_start - L);

        while (!buffer.empty()) {
            output.push_back(decoding_data->symbol);
            x_start = update_decoding_state(buffer, decoding_data->max_bit_shift, decoding_data->new_state);
            decoding_data = decoding_table.at(x_start);
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

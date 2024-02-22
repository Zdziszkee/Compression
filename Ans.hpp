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
        int number_of_bits_to_emit;
        int new_state;

        DecodingData(char symbol, int number_of_bits_to_emit, int new_state)
            : symbol(symbol), number_of_bits_to_emit(number_of_bits_to_emit), new_state(new_state) {
        }
    };

    std::map<char, int> frequencies;
    std::map<char, int> frequencies_quantized;
    int alphabet_size = 0;
    int number_of_symbols = 0;
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
            const char symbol = symbols_sample_distribution[x - L];
            encoding_table[intervals[symbol] + frequencies_copy[symbol]++] = x;
        }
    }

    void create_decoding_table() {
        std::map<char, int> frequencies_copy = frequencies_quantized;
        decoding_table.resize(L);

        for (int x = 0; x < L; x++) {
            const char symbol = symbols_sample_distribution[x];
            const int frequency = frequencies_copy[symbol]++;
            const int max_bit_shift = R - static_cast<int>(floor(log2(frequency)));
            const int new_state = (frequency << max_bit_shift) - L;
            auto* decoding_data = new DecodingData(symbol, max_bit_shift, new_state);
            decoding_table[x] = decoding_data;
        }
    }


    static void emit_bits(std::vector<bool>& buffer, int state, int number_of_bits_to_emit) {
        const int bits_to_extract_mask = (1 << number_of_bits_to_emit) - 1;
        int least_significant_bits = state & bits_to_extract_mask;

        for (int i = 0; i < number_of_bits_to_emit; i++, least_significant_bits >>= 1)
            buffer.push_back((least_significant_bits & 1));
    }

    void write_state(std::vector<bool>& buffer, int state) const {
        for (int i = 0; i < r; i++, state >>= 1)
            buffer.push_back((state & 1));
    }

    std::vector<bool> encode(std::string text) {
        number_of_symbols = static_cast<int>(text.size());
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
        for (int i = number_of_symbols - 1; i >= 0; --i) {
            char symbol = text[i];
            int number_of_bits_to_emit = (state + bit_shifts[symbol]) >> r;
            emit_bits(buffer, state, number_of_bits_to_emit);
            /** Aktualny stan x działa jak bufor,
             *  trzymając między [R, R + 1) bitów informacji,
             *  do którego dokładamy informację o nowym symbolu.
             */
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
        int result = 0;
        for (const bool bit: buffer) {
            result = result << 1 | bit;
        }
        return result;
    }

    static int decoding_step(std::vector<bool>& buffer, int number_of_bits_to_emit, int new_state) {
        if (number_of_bits_to_emit > buffer.size()) {
            number_of_bits_to_emit = static_cast<int>(buffer.size());
        }

        int accumulator = 0;
        for (int i = 0; i < number_of_bits_to_emit; i++) {
            accumulator = (accumulator << 1) | buffer.back();
            buffer.pop_back();
        }

        return new_state + accumulator;
    }


    std::string decode(std::vector<bool>& buffer) const{
        std::string output;
        int starting_state = read_decoding_state(buffer);
        DecodingData* decoding_data = decoding_table.at(starting_state - L);

        while (!buffer.empty()) {
            output.push_back(decoding_data->symbol);
            starting_state = decoding_step(buffer, decoding_data->number_of_bits_to_emit, decoding_data->new_state);
            decoding_data = decoding_table.at(starting_state);
        }

        return output;
    }

};


#endif //ANS_HPP

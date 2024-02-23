#include <iostream>
#include <fstream>
#include <vector>
#include "Ans.hpp"
#include <chrono>


int main(int arguments_size, char** arguments) {
    if (arguments_size != 2) {
        std::cerr << "Wrong number of arguments!" << std::endl;
        return 1;
    }
    const char* file_name = arguments[1];
    std::ifstream file(file_name);

    if (!file.is_open()) {
        std::cerr << "Error opening file!" << std::endl;
        return 1;
    }
    std::string file_contents;

    if (!file.is_open()) {
        std::cerr << "Unable to open file: " << file_contents << std::endl;
    }

    file.seekg(0, std::ios::end);
    file_contents.reserve(file.tellg());
    file.seekg(0, std::ios::beg);
    file_contents.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    Ans ans;

    auto compress_start = std::chrono::high_resolution_clock::now();
    std::vector<bool> compressed = ans.encode(file_contents);
    auto compress_end = std::chrono::high_resolution_clock::now();
    std::cout << "Compress time: " << (compress_end - compress_start).count() << " nanoseconds\n";

    size_t compressed_size = compressed.size();
    std::cout << "Encoded: " << compressed_size << std::endl;
    auto decompress_start = std::chrono::high_resolution_clock::now();
    auto decompress = ans.decode(compressed);
    auto decompress_end = std::chrono::high_resolution_clock::now();
    std::cout << "Decompress time: " << (decompress_end - decompress_start).count() << " nanoseconds\n";

    std::cout<<decompress<<std::endl;
    auto decompressed_size = (file_contents.size() * 8);
    std::cout << "Decoded: " << decompressed_size << std::endl;

    std::cout<<"compression ratio: "<<(double)compressed_size/ ((double)decompressed_size );

    return 0;
}

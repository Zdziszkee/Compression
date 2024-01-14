#include <iostream>
#include <fstream>
#include <unordered_map>

#include "Decoder.hpp"
#include "Encoder.hpp"
#include "Metadata.hpp"

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

    Metadata metadata(file_contents);

    Encoder encoder(file_contents);
    long long encoded = encoder.encode();
    std::cout << encoded << std::endl;

    Decoder decoder()
    return 0;
}

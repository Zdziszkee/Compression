cmake_minimum_required(VERSION 3.27)
project(Compression)
set(CMAKE_CXX_STANDARD 17)
add_executable(Compression main.cpp
        Encoder.hpp
        Decoder.hpp
        Metadata.hpp)
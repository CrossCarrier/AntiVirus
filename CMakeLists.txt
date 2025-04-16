cmake_minimum_required(VERSION 3.30)
project(AntiVirus)

set(CMAKE_CXX_STANDARD 23)

find_package(OpenSSL REQUIRED)

target_link_libraries(AntiVirus PRIVATE OpenSSL::SSL OpenSSL::Crypto)

add_executable(AntiVirus main.cpp
        FileManager/source/FileManager.cpp
        FileManager/include/FileManager.hpp
        FileManager/include/HASH-SHA256.hpp
        FileManager/source/HASH-SHA256.cpp)

cmake_minimum_required(VERSION 3.30)

set(CMAKE_C_COMPILER "/opt/homebrew/bin/gcc-14")
set(CMAKE_CXX_COMPILER "/opt/homebrew/bin/g++-14")

project(AntiVirus)

set(CMAKE_CXX_STANDARD 23)

set(OPENSSL_ROOT_DIR "/opt/homebrew/opt/openssl@3")
set(OPENSSL_INCLUDE_DIR "/opt/homebrew/opt/openssl@3/include")

find_package(OpenSSL REQUIRED)

add_executable(AntiVirus
        main.cpp
        FileManager/source/File.cpp
        FileManager/include/File.hpp
        FileManager/include/HASH-SHA256.hpp
        FileManager/source/HASH-SHA256.cpp
        DirectoryManager/include/Directory.hpp
        DirectoryManager/source/Directory.cpp
        Scanner/include/Scanner.hpp
        Scanner/source/Scanner.cpp
)

target_link_libraries(AntiVirus PRIVATE OpenSSL::SSL OpenSSL::Crypto)


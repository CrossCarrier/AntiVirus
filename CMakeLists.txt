cmake_minimum_required(VERSION 3.30)

project(AntiVirus)

# Standard języka
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Flagi
add_compile_options(-Wall -Werror -Wpedantic)

# OpenSSL       
set(OPENSSL_ROOT_DIR "/opt/homebrew/opt/openssl@3")
set(OPENSSL_INCLUDE_DIR "/opt/homebrew/opt/openssl@3/include")
find_package(OpenSSL REQUIRED)

# Zewnętrzne źródła (np. pliki źródłowe projektu)
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
    Logger/include/Logger.hpp
    Logger/source/Logger.cpp
    Cleaner/include/Cleaner.hpp
    Cleaner/source/Cleaner.cpp
)

# Linkowanie
target_link_libraries(AntiVirus PRIVATE OpenSSL::SSL OpenSSL::Crypto)

# 🧠 DODATKOWO: Podaj sysroot do SDK systemowego, żeby nie walczyć z błędami _Alignof
if(APPLE)
    set(SDKROOT "/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk")
    target_compile_options(AntiVirus PRIVATE -isysroot ${SDKROOT})
    target_link_options(AntiVirus PRIVATE -isysroot ${SDKROOT})
endif()

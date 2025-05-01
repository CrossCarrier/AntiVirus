cmake_minimum_required(VERSION 3.30)

project(AntiVirus)

# Standard języka
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Flagi
add_compile_options(-Wall -Werror -Wpedantic)

# OpenSSL       
find_package(OpenSSL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(YARA REQUIRED yara)
find_package(Boost REQUIRED COMPONENTS filesystem system)
include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIR})

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
    Scanner/include/YARA_Wrapper.hpp
    Scanner/source/Scanner.cpp
    Scanner/source/YARA_Wrapper.cpp
    Logger/include/Logger.hpp
    Logger/source/Logger.cpp
    Cleaner/include/Cleaner.hpp
    Cleaner/source/Cleaner.cpp
)

# Linkowanie
target_include_directories(AntiVirus PRIVATE ${YARA_INCLUDE_DIRS})
target_link_libraries(
    AntiVirus PRIVATE OpenSSL::SSL 
    OpenSSL::Crypto
    ${YARA_LIBRARIES}
    ${Boost_LIBRARIES})

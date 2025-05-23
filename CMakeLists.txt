cmake_minimum_required(VERSION 3.30)
set(CMAKE_CXX_COMPILER "/opt/homebrew/bin/g++-14")
set(CMAKE_C_COMPILER "/opt/homebrew/bin/gcc-14")
set(CMAKE_CXX_STANDARD 23)
project(AntiVirus)

# Flagi
# add_compile_options(-Wall -Werror -Wpedantic)

# OpenSSL       
find_package(OpenSSL REQUIRED)
# find_package(PkgConfig REQUIRED)
# pkg_check_modules(YARA REQUIRED yara)
find_package(Boost REQUIRED COMPONENTS filesystem system)
include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIR})

include (FetchContent)
FetchContent_Declare(
        cli11
        GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
        GIT_TAG v2.3.2
    )
FetchContent_MakeAvailable(cli11)

# Zewnętrzne źródła (np. pliki źródłowe projektu)
add_executable(AntiVirus
#    DirectoryManager/include/Directory.hpp
#    DirectoryManager/source/Directory.cpp
#    Scanner/include/Scanner.hpp
#    Scanner/include/YARA_Wrapper.hpp
#    Scanner/source/Scanner.cpp
#    Scanner/source/YARA_Wrapper.cpp
#    Logger/include/Logger.hpp
#    Logger/source/Logger.cpp
#    Cleaner/include/Cleaner.hpp
#    Cleaner/source/Cleaner.cpp
        main.cpp
        FileManager/include/HASH-SHA256.hpp
        FileManager/source/HASH-SHA256.cpp
        RuleEngine/include/RuleEngine.hpp
        RuleEngine/source/RuleEngine.cpp
        Scanner/include/scanner.hpp
        Scanner/source/scanner.cpp
        HELPERS/include/support.hpp
        FileManager/source/File.cpp
        FileManager/include/File.hpp
        ERRORS_PACK/include/errors.hpp
        ERRORS_PACK/source/errors.cpp
        HELPERS/include/json.hpp
)

# Set working directory for the target
set_target_properties(AntiVirus PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)

# Linkowanie
target_include_directories(AntiVirus PRIVATE ${YARA_INCLUDE_DIRS})
target_link_libraries(AntiVirus PRIVATE 
    CLI11::CLI11
    OpenSSL::SSL 
    OpenSSL::Crypto
    ${Boost_LIBRARIES})

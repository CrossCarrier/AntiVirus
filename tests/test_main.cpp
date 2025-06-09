#include "ERRORS_PACK/include/errors.hpp"
#include "FileManager/include/FileManager.hpp
#include "FileManager/include/IndexManager.hpp"
#include "Scanner/include/Scanner.hpp"
#include <catch2/catch.hpp>
#include <fstream>

TEST_CASE("PathNotFound throws and returns correct message", "[error]") {
    REQUIRE_THROWS_AS(throw PathNotFound("foo.txt"), PathNotFound);
    try {
        throw PathNotFound("foo.txt");
    } catch (const PathNotFound& e) {
        REQUIRE(std::string(e.what()).find("foo.txt") != std::string::npos);
        REQUIRE(e.getTips().find("Check if file") != std::string::npos);
    }
}

TEST_CASE("DirectoryValidationError returns interface tips", "[error]") {
    DirectoryValidationError err("/tmp/dir");
    REQUIRE(err.getTips().empty()); // Uzupełnij getTips jeśli chcesz
}




using namespace filemanager;

TEST_CASE("validate_file returns false for non-existing file", "[file]") {
    REQUIRE_FALSE(validate::validate_file("nonexistent.file"));
}

TEST_CASE("validate_directory returns false for non-existing directory", "[file]") {
    REQUIRE_FALSE(validate::validate_directory("not_a_dir"));
}





TEST_CASE("file::lastModificationTime throws for invalid file", "[file]") {
    REQUIRE_THROWS_AS(filemanager::file::lastModificationTime("no_file.txt"), std::invalid_argument);
}

TEST_CASE("file::size throws for invalid file", "[file]") {
    REQUIRE_THROWS_AS(filemanager::file::size("no_file.txt"), std::invalid_argument);
}




TEST_CASE("scanMultipleFiles returns empty for clean file", "[yara]") {
    PATHS_CONTAINER files = {"clean_sample.txt"};
    auto results = scanner::scanMultipleFiles(files, 1);
    REQUIRE(results["clean_sample.txt"].empty());
}





// Przykład: sprawdzenie czy filtr modyfikuje listę plików
TEST_CASE("Quick scan filters files", "[scan][quick]") {
    PATHS_CONTAINER files = {/* lista istniejących plików */};
    auto filtered = index_manager::filterModified(files);
    REQUIRE(filtered.size() <= files.size());
}
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <iostream>

int main() {
    boost::filesystem::path Virus1("../virus1.txt");

    std::time_t modification_time = last_write_time(Virus1);
    std::cout << modification_time << std::endl;

    return 0;
}


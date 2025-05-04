#include "../include/IndexManager.hpp"
#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>
#include <iostream>

auto IndexManager::update_database() -> void {
	try {
		for (auto& entry: boost::make_iterator_range(boost::filesystem::recursive_directory_iterator("."), {}))
		{
			if (!boost::filesystem::is_regular_file(entry)) {
				continue;
			}
			std::cout << entry.path() << std::endl;
		}
	}
	catch (...) {
		std::cerr << "Error occured!" << std::endl;
	}
}

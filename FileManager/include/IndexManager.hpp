#pragma once

#include "../../HELPERS/include/FixedSizeContainer.hpp"
#include "File.hpp"
#include <boost/filesystem/path.hpp>

class IndexManager {
private:
    static boost::filesystem::path m_FilesData;

public:
    static auto update_index(const File &_file) -> void;
    static auto update_database() -> void;
    static auto update_all_indexes(const FixedSizeContainer<File> &_files_containers) -> void;
    static auto fetch_all_data() -> FixedSizeContainer<File>;
    static auto fetch_modified_data() -> FixedSizeContainer<File>;
};

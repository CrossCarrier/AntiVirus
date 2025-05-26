#include "../include/IndexManager.hpp"
#include "../../HELPERS/include/support.hpp"
#include "../include/File.hpp"
#include <algorithm>
#include <memory>

namespace {
    template <typename T>
    auto add_property(nlohmann::json &__json, const std::string &key, T value) -> void {
        __json[key] = value;
    }

    auto create_file_entry(const File &file) -> nlohmann::json {
        nlohmann::json entry;
        add_property(entry, "Hash", file.get_HashID());
        add_property(entry, "Modification time", file.get_LastModificationTime());
        add_property(entry, "Size", file.get_Size());
        return entry;
    }

    template <typename FileContainer>
    auto update_json_with_files(nlohmann::json &__json, const FileContainer &files) -> void {
        std::ranges::for_each(
            files, [&](const File &__val) -> void { __json[__val.get_FilePath().c_str()] = create_file_entry(__val); });
    }
} // namespace
namespace index_manager {
    FILE_PACK fetch_data(FILE::path &&__path, const bool __mod = false /* Flag for fetching only modified files*/) {
        FILE_PACK __data;

        auto fetched_data = support::json_utils::read_data(std::move(__path));
        std::ranges::for_each(fetched_data, [&](const std::string &__val) -> void {
            bool filemod = false;
            std::unique_ptr<File> f = std::make_unique<File>(__val);
            if (f->get_HashID() != static_cast<std::string>(fetched_data[__val]["Hash"]) ||
                f->get_LastModificationTime() != static_cast<time_t>(fetched_data[__val]["Modification time"]) ||
                f->get_Size() != fetched_data[__val]["Size"]) {
                filemod = true;
            }

            if ((__mod && filemod) || (!__mod)) {
                __data.push_back(std::move(*f));
            }
        });

        return __data;
    }

    auto update_metaindex(FILE::path &&__path /* Path to file, where files are indexed*/) -> void {
        nlohmann::json data;
        auto sys_files = support::filesystem_utils::load_files_from_system();
        update_json_with_files(data, sys_files);
        support::json_utils::write_data(__path, data);
    }

} // namespace index_manager

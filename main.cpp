#include "HELPERS/include/support.hpp"
#include "Scanner/include/Scanner.hpp"
#include <CLI/CLI.hpp>

int main(int argc, char *argv[]) {
    CLI::App app{"Antivirus"};

    std::string file_path;
    app.add_option("-s, --scan_file", file_path, "Skanowanie pliku");

    try {
        app.parse(argc, argv);

        if (!file_path.empty()) {
            auto results = scanner::scan_file(file_path);
            nlohmann::json data;
            std::string detected = "detected";
            if (results.empty()) {
                detected = "undetected";
            }

            data[file_path]["viruses_detected"] = detected;
            data[file_path]["viruses"] = results;
            support::json_utils::write_data(std::filesystem::path("output.json"), data);
        }

    } catch (const CLI::ParseError &error) {
        return app.exit(error);
    }

    return 0;
}
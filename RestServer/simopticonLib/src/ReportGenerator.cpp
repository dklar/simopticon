#include "ReportGenerator.h"



std::vector<fs::path> ReportGenerator::generateReports(const std::vector<fs::path> &paths) {
    // std::vector<fs::path> zipPaths;
    // for (const auto &path : paths) {
    //     fs::path zipPath = generateReport(path);
    //     zipPaths.push_back(zipPath);
    // }
    // return zipPaths;
    std::vector<fs::path> zipPaths(paths.size());
    std::vector<std::thread> threads;
    threads.reserve(paths.size());


    auto generateReportThread = [&](size_t index) {
        zipPaths[index] = generateReport(paths[index]);
    };
    for (size_t i = 0; i < paths.size(); ++i) {
        threads.emplace_back(generateReportThread, i);
    }
    for (auto &thread : threads) {
        thread.join();
    }

    return zipPaths;
}

fs::path ReportGenerator::generateReport(const fs::path &path) {
    createCsvData(path);
    createPlots(path, "speed");
    createPlots(path, "distance");
    
    auto zipPath = path.parent_path() / (path.filename().string() + ".zip");
    deleteOldReport(zipPath);
    return zipDirectory(path, zipPath.string());
}

void ReportGenerator::createCsvData(const std::filesystem::path &path) {
    std::cout << path << std::endl;
    std::string command = "cd " + path.string() + " && opp_scavetool x *vec -o parsedData.csv";
    int result = std::system(command.c_str());
    if (result == 0) {
        createPlots(path,"speed");
        createPlots(path,"distance");
    } else {
        std::cout << "Fehler beim Ausführen des Programms." << std::endl;
    }
}

void ReportGenerator::deleteOldReport(const fs::path &zipPath) {
    if (fs::exists(zipPath)) {
        fs::remove(zipPath);
    }
}

void ReportGenerator::deleteOldReports(const std::vector<fs::path>& filePaths) {
    for (const auto &path : filePaths) {
        deleteOldReport(path);
    }
}

void ReportGenerator::createPlots(const std::filesystem::path& path, const std::string& arg) {
    try {
        std::filesystem::path pythonScriptPath = "../python/parse.py";
        std::vector<std::string> pythonArgs = {path / "parsedData.csv", arg, "save", "False","1920", "1080"};

        std::string command = "python \"" + pythonScriptPath.string() + "\"";
        for (const auto& arg : pythonArgs) {
            command += " \"" + arg + "\"";
        }
        int result = std::system(command.c_str());

        if (result != 0) {
            std::cerr << "Fehler beim Ausführen des Python-Scripts." << std::endl;
        }
    } catch (const std::exception& error) {
        std::cerr << "Fehler beim Ausführen des Python-Scripts: " << error.what() << std::endl;
    }
}

fs::path ReportGenerator::zipDirectory(const fs::path& inputdir, const std::string& output_filename){
    int errorp;
    zip_t* zipper = zip_open(output_filename.c_str(), ZIP_CREATE | ZIP_EXCL, &errorp);
    if (!zipper) {
        zip_error_t ziperror;
        zip_error_init_with_code(&ziperror, errorp);
        throw std::runtime_error("Failed to open output file " + output_filename + ": " + zip_error_strerror(&ziperror));
    }

    try {
        walkDirectory(inputdir, inputdir, zipper);
    } catch (...) {
        zip_close(zipper);
        throw;
    }

    zip_close(zipper);
    return fs::path(output_filename);
}

void ReportGenerator::walkDirectory(const fs::path& startdir, const fs::path& inputdir, zip_t* zipper){
    for (const auto& entry : fs::directory_iterator(inputdir)) {
        const auto& path = entry.path();
        if (path.filename() == "." || path.filename() == "..") {
            continue;
        }

        if (fs::is_directory(path)) {
            if (zip_dir_add(zipper, (path.string().substr(startdir.string().length() + 1)).c_str(), ZIP_FL_ENC_UTF_8) < 0) {
                throw std::runtime_error("Failed to add directory to zip: " + std::string(zip_strerror(zipper)));
            }
            walkDirectory(startdir, path, zipper);
        } else {
            zip_source_t* source = zip_source_file(zipper, path.c_str(), 0, 0);
            if (!source) {
                throw std::runtime_error("Failed to add file to zip: " + std::string(zip_strerror(zipper)));
            }
            if (zip_file_add(zipper, (path.string().substr(startdir.string().length() + 1)).c_str(), source, ZIP_FL_ENC_UTF_8) < 0) {
                zip_source_free(source);
                throw std::runtime_error("Failed to add file to zip: " + std::string(zip_strerror(zipper)));
            }
        }
    }
}

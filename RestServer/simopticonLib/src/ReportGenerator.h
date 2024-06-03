#ifndef REPORTGENERATOR_H
#define REPORTGENERATOR_H
#include <iostream>
#include <zip.h>
#include <filesystem>
#include <stdexcept>
#include <filesystem>
#include <vector>
#include <string>
#include <thread>


namespace fs = std::filesystem;

class ReportGenerator {
public:
    std::vector<fs::path> generateReports(const std::vector<fs::path> &path);
    fs::path generateReport(const fs::path &path);
    void deleteOldReport(const fs::path &zipPath);
    void deleteOldReports(const std::vector<fs::path> &filePaths);
private:
    void createCsvData(const fs::path &path);
    void createPlots(const fs::path &path, const std::string &argument);
    fs::path zipDirectory(const fs::path &inputDir, const std::string &outputFilename);
    void walkDirectory(const fs::path &startDir, const fs::path &inputDir, zip_t *zipper);
};

#endif // REPORTGENERATOR_H
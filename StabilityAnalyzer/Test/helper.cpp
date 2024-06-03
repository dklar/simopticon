#include "helper.h"

bool readTestVectorFile(const std::string& filename, std::vector<double>& inputVector, std::vector<double>& outputVector, double& fPeak, double& ninf) {
    try {
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Fehler beim Öffnen der Datei: " << filename << std::endl;
            return false;
        }

        std::string line;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] != '#') {
                break;
            }
            std::cout << line << std::endl;
        }
        
        std::vector<double> data;
        double inputValue;
        while (file.read(reinterpret_cast<char*>(&inputValue), sizeof(double))) {
            data.push_back(inputValue);
        }
        
        fPeak = data.back();
        data.pop_back();
        ninf = data.back();
        data.pop_back();

        size_t halfSize = data.size() / 2;
        std::cout << halfSize << std::endl;
        inputVector.assign(data.begin(), data.begin() + halfSize);
        outputVector.assign(data.begin() + halfSize, data.end());

        file.close();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Fehler beim Lesen der Daten aus der Datei: " << e.what() << std::endl;
        return false;
    }
}

bool readTestVectorFile(const std::string& filename, std::vector<double>& inputVector) {
    try {
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Fehler beim Öffnen der Datei: " << filename << std::endl;
            return false;
        }

        std::string line;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] != '#') {
                break;
            }
            std::cout << line << std::endl;
        }
        
        double inputValue;
        while (file.read(reinterpret_cast<char*>(&inputValue), sizeof(double))) {
            inputVector.push_back(inputValue);
        }        
        file.close();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Fehler beim Lesen der Daten aus der Datei: " << e.what() << std::endl;
        return false;
    }
}
bool readTestVectorFile(const std::string& filename, std::vector<std::complex<double>>& inputVector) {
    try {
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Fehler beim Öffnen der Datei: " << filename << std::endl;
            return false;
        }

        std::string line;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] != '#') {
                break;
            }
            //std::cout << line << std::endl;
        }
         
        double realPart, imagPart;
        while (file.read(reinterpret_cast<char*>(&realPart), sizeof(double)) &&
               file.read(reinterpret_cast<char*>(&imagPart), sizeof(double))) {
            inputVector.emplace_back(realPart, imagPart);
        }

        file.close();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Fehler beim Lesen der Daten aus der Datei: " << e.what() << std::endl;
        return false;
    }
}

bool readResultFile(const std::string& filename, std::vector<std::complex<double>>& inputVector,double& fPeak, double& ninf) {
    try {
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Fehler beim Öffnen der Datei: " << filename << std::endl;
            return false;
        }

        std::string line;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] != '#') {
                break;
            }
            //std::cout << line << std::endl;
        }

        double peak;
        file.read(reinterpret_cast<char*>(&fPeak), sizeof(double));
        double hinf;
        file.read(reinterpret_cast<char*>(&ninf), sizeof(double));

        double realPart, imagPart;
        while (file.read(reinterpret_cast<char*>(&realPart), sizeof(double)) &&
               file.read(reinterpret_cast<char*>(&imagPart), sizeof(double))) {

            inputVector.push_back(std::complex(realPart, imagPart));
        }
        file.close();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Fehler beim Lesen der Daten aus der Datei: " << e.what() << std::endl;
        return false;
    }
}

std::vector<testcaseinfo> readfile(const std::string& filename) {
    std::vector<testcaseinfo> testcases;
    try {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Fehler beim Öffnen der Datei: " << filename << std::endl;
            return testcases;
        }
        while (!file.eof()) {
            std::string line, tf, file1, file2, max;

            for (auto i = 0; i < 3; i++) {
                std::getline(file, line);
                tf += line;
            }
            std::getline(file, line);
            file1 = line;
            std::getline(file, line);
            file2 = line;
            std::getline(file, line);
            max = line;

            testcaseinfo testcase;
            testcase.transferFunction = tf;
            testcase.inputFilename = file1;
            testcase.outputFilename = file2;
            testcase.max_y2 = std::stod(max);
            testcases.push_back(testcase);
        }

        file.close();
    } catch (const std::exception& e) {
        std::cerr << "Fehler beim Lesen der Daten aus der Datei: " << e.what() << std::endl;
    }
    return testcases;
}

std::vector<testcaseinfo>  readMagnitudeTestfile(const std::string& filename) {
    std::vector<testcaseinfo> testcases;
    try {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Fehler beim Öffnen der Datei: " << filename << std::endl;
            return testcases;
        }
        while (!file.eof()) {
            std::string line,file1, file2;
            std::getline(file, line);
            file1 = line;
            std::getline(file, line);
            file2 = line;
            testcaseinfo testcase;
            testcase.inputFilename = file1;
            testcase.outputFilename = file2;
            testcases.push_back(testcase);
        }
        file.close();
    } catch (const std::exception& e) {
        std::cerr << "Fehler beim Lesen der Daten aus der Datei: " << e.what() << std::endl;
    }
    return testcases;
}

bool readData(std::string input_filename,std::vector<double>& x,std::vector<double>& y){
    std::ifstream file(input_filename, std::ios::binary);
    std::vector<double> values;

    if (!file.is_open()) {
        std::cerr << "Fehler beim Öffnen der Datei." << std::endl;
        return false;
    }

    // Lese die Daten aus der Datei
    double value;
    while (file.read(reinterpret_cast<char*>(&value), sizeof(double))) {
        values.push_back(value);
    }
    size_t half_size = values.size() / 2;
    x.assign(values.begin(), values.begin() + half_size);
    y.assign(values.begin() + half_size, values.end());
    return true;
}

bool readData(std::string input_filename,std::vector<double>& x){
    std::ifstream file(input_filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Fehler beim Öffnen der Datei." << std::endl;
        return false;
    }
    double value;
    while (file.read(reinterpret_cast<char*>(&value), sizeof(double))) {
        x.push_back(value);
    }
    return true;
}
bool readData(std::string input_filename,ImageDomainVector& x){
    std::ifstream file(input_filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Fehler beim Öffnen der Datei." << std::endl;
        return false;
    }
    double realPart, imaginaryPart;
    while (file.read(reinterpret_cast<char*>(&realPart), sizeof(double)) &&
           file.read(reinterpret_cast<char*>(&imaginaryPart), sizeof(double))) {
        x.emplace_back(realPart, imaginaryPart);
    }
    return true;
}

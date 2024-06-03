#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <cmath>
#include <complex>
#include "../types.h"

struct testcaseinfo {
    std::string inputFilename;
    std::string outputFilename;
    std::string transferFunction;
    double max_y2;
};

bool readTestVectorFile(const std::string& filename, std::vector<double>& inputVector, std::vector<double>& outputVector, double& fPeak, double& ninf);

bool readTestVectorFile(const std::string& filename, std::vector<double>& inputVector);
bool readTestVectorFile(const std::string& filename, std::vector<std::complex<double>>& inputVector);
bool readResultFile(const std::string& filename, std::vector<std::complex<double>>& inputVector,double& fPeak, double& ninf) ;
std::vector<testcaseinfo> readfile(const std::string& filename);
bool readData(std::string input_filename,std::vector<double>& x,std::vector<double>& y);
bool readData(std::string input_filename,std::vector<double>& x);
bool readData(std::string input_filename,ImageDomainVector& x);
std::vector<testcaseinfo>  readMagnitudeTestfile(const std::string& filename);

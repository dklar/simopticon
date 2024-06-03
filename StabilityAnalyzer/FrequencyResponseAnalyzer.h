#include <map>
#include <vector>
#include <cmath>
#include <limits>
#include <algorithm>
#include <numeric>
#include "types.h"
#include "FourierTransform.h"

using namespace std;

class FrequencyResponseAnalyzer
{
private:
    map<double, vector<double>> mValidFrequencyResponse;
    pair<double,double> mMaximum;
    double mLimit = 1e6;
    void addFrequencyResponsePoints(const std::vector<double> & inputVector,const std::vector<double> & outputvector,double fa);
    double calculateMedian(const std::vector<double>& data);
    double calculateMean(const std::vector<double>& data);

public:
    FrequencyResponseAnalyzer(const std::vector<double> & inputVector,const std::vector<double> & outputvector,double fa);
    FrequencyResponseAnalyzer();
    ~FrequencyResponseAnalyzer();
    tuple<std::vector<double>, std::vector<double>> calculateFreqResponse(bool useMean = false);
    pair<double,double> getMaximum();
    double getNorm();
    bool isStable();
};


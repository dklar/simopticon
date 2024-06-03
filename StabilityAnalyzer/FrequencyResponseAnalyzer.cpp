#include "FrequencyResponseAnalyzer.h"


FrequencyResponseAnalyzer::FrequencyResponseAnalyzer(const std::vector<double> & inputVector,const std::vector<double> & outputvector,double fa)
{
    this->addFrequencyResponsePoints(inputVector,outputvector,fa);
}

FrequencyResponseAnalyzer::~FrequencyResponseAnalyzer()
{
}

tuple<std::vector<double>, std::vector<double>> FrequencyResponseAnalyzer::calculateFreqResponse(bool useMean) {
    std::vector<double> x, y;
    for (const auto& entry : mValidFrequencyResponse) {
        double freq = entry.first;
        const std::vector<double>& gains = entry.second;

        x.push_back(freq);
        if (useMean) {
            double tmp = calculateMean(gains);
            y.push_back(tmp);

            if (tmp > mMaximum.second) {
                mMaximum.second = tmp;
                mMaximum.first = freq;
            }
        } else {
            double tmp = calculateMedian(gains);
            y.push_back(tmp);

            if (tmp > mMaximum.second) {
                mMaximum.second = tmp;
                mMaximum.first = freq;
            }
        }
    }

    return make_tuple(x, y);
}

pair<double, double> FrequencyResponseAnalyzer::getMaximum() {
    this->calculateFreqResponse();
    return FrequencyResponseAnalyzer::mMaximum;
}

void FrequencyResponseAnalyzer::addFrequencyResponsePoints(const std::vector<double> & inputSpectrum,const std::vector<double> & outputSpectrum,double fa){
    double minimumPeak = 2.0;
    for (auto i = 0; i < inputSpectrum.size(); i++)
    {
        double absOut = std::abs(outputSpectrum[i]);
        double absIn = std::abs(inputSpectrum[i]);
        if (absOut > minimumPeak || absIn > minimumPeak) {
            double validGain = absOut/absIn;
            double validFreq = fa*i;

            if (mValidFrequencyResponse.find(validFreq) != mValidFrequencyResponse.end()) {
                mValidFrequencyResponse[validFreq].push_back(validGain);
            } else {
                mValidFrequencyResponse[validFreq] = {validGain};
            }
        }
    }
    
}

double FrequencyResponseAnalyzer::getNorm() {
    return FrequencyResponseAnalyzer::mMaximum.second;
}

bool FrequencyResponseAnalyzer::isStable(){
    return FrequencyResponseAnalyzer::mMaximum.second < FrequencyResponseAnalyzer::mLimit;
}

double FrequencyResponseAnalyzer::calculateMean(const std::vector<double>& data) {
        double sum = std::accumulate(data.begin(), data.end(), 0.0);
        return sum / data.size();
    }

double FrequencyResponseAnalyzer::calculateMedian(const std::vector<double>& data) {
    std::vector<double> sortedData = data;
    std::sort(sortedData.begin(), sortedData.end());

    size_t size = sortedData.size();
    if (size % 2 == 0) {
        return (sortedData[size / 2 - 1] + sortedData[size / 2]) / 2.0;
    } else {
        return sortedData[size / 2];
    }
}


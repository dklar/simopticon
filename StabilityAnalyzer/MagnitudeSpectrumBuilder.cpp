#include "MagnitudeSpectrumBuilder.h"

MagnitudeSpectrumBuilder::MagnitudeSpectrumBuilder(){

}

ImageDomainVector MagnitudeSpectrumBuilder::getImageDomainData(TimeDomainVector& data) {
    auto window = FourierTransform::calculateHanningWindow(data.size());
    for (int i = 0; i < data.size(); ++i) {
        data[i] *=  window[i];
    }
    return FourierTransform::transform(data);
}

std::vector<double>  MagnitudeSpectrumBuilder::getSpectrumData(TimeDomainVector& data_t) {
    int N = static_cast<int>(data_t.size() / 2 + 1);
    ImageDomainVector data_s = MagnitudeSpectrumBuilder::getImageDomainData(data_t);
    std::vector<double> magnitudeSpectrum(N);
    for (auto i = 0; i < N; i++)
    {
        double val = 2*std::abs(data_s[i])/N;
        magnitudeSpectrum[i] = val;
    }
    return magnitudeSpectrum;
}

std::vector<double>& MagnitudeSpectrumBuilder::getFrequencies(int vectorsize,double fa){
    int N = (int)(vectorsize/ 2 + 1);
    for (int i = 0; i < N; ++i) {
        auto tmp = i * fa / (2 * (N - 1));
        frequencies.push_back(tmp);
    }
    return frequencies;
}

MagnitudeSpectrumBuilder::~MagnitudeSpectrumBuilder(){

}
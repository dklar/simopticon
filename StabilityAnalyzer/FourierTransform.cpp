#include "FourierTransform.h"



FourierTransform::FourierTransform() {

}

std::vector<std::complex<double>>FourierTransform::transform(const std::vector<double>& input) {
    Eigen::FFT<double> fft;
    std::vector<std::complex<double> > freqvec;
    fft.fwd( freqvec,input);
    return freqvec;
}

std::vector<std::complex<double>>FourierTransform::transform(const std::vector<std::complex<double>>& input) {
    Eigen::FFT<double> fft;
    std::vector<std::complex<double> > freqvec(input.size());;
    fft.fwd(freqvec,input);
    return freqvec;
}

// ImageDomainVector FourierTransform::transform(const TimeDomainVector& input) {
//     Eigen::FFT<double> fft;
//     ImageDomainVector freqvec;
//     fft.fwd( freqvec,input);
//     return freqvec;
// }

std::vector<std::complex<double>> FourierTransform::cooleyTukeyFFT(const std::vector<std::complex<double>>& input){
    
    int N = input.size();
    if (N==1){
        return input;
    }else{
        std::vector<std::complex<double>> even;
        std::vector<std::complex<double>> odd;
        std::partition_copy(input.begin(), input.end(), std::back_inserter(even),
                        std::back_inserter(odd),
                        [&](const auto& elem) { return std::distance(input.begin(), std::find(input.begin(), input.end(), elem)) % 2 == 0; });


        std::vector<std::complex<double>> evenFFT(N);
        std::vector<std::complex<double>> oddFFT(N);

    omp_set_num_threads(omp_get_max_threads());
#pragma omp parallel sections
{
        #pragma omp section
        evenFFT = cooleyTukeyFFT(even);
        #pragma omp section
        oddFFT = cooleyTukeyFFT(odd);
}
        std::vector<std::complex<double>> result(N);
        #pragma omp parallel for
        for (auto k = 0; k < N/2; k++){
            std::complex<double> t = std::polar(1.0, -2.0 * M_PI * k / N) * oddFFT[k];
            result[k] = evenFFT[k] + t;
            result[k + N / 2] = evenFFT[k] - t;
        }
        return result;
    }
}

std::vector<std::complex<double>>FourierTransform::cooleyTukeyFFT(const std::vector<double>& input){
    int N = input.size();
    std::vector<std::complex<double>> complexInput(N);
    for (int i = 0; i < N; ++i) {
        complexInput[i] = std::complex<double>(input[i], 0.0);
    }
    if (N == 1) {
        return complexInput;
    }else{
        return cooleyTukeyFFT(complexInput);
    }
}

std::vector<std::complex<double>> FourierTransform::chirpZ_Transformaiton(const std::vector<double>& input){
    int N = input.size();
    //omp_set_num_threads(omp_get_max_threads());
    std::vector<std::complex<double>> output(N);
    for (int m = 0; m < N; ++m) {
        std::complex<double> sum = 0.0;
        //#pragma omp parallel for
        for (int n = 0; n < N; ++n) {
            double angle = -2.0 * M_PI * m * n / N;
            std::complex<double> twiddle_factor = std::polar(1.0, angle);
            auto tmp = input[n] * twiddle_factor;
            //#pragma omp critical
            sum += tmp;
        }

        output[m] = sum;
    }

    return output;
}

const std::vector<double> FourierTransform::getWindowFunction(int samples){
    std::vector<double> hammingWindow;
    // omp_set_num_threads(omp_get_max_threads());
    // #pragma omp parallel for
    for (int i = 1 - samples; i < samples; i += 2) {
        double value = 0.54 + 0.46 * std::cos(M_PI * i / (samples - 1));
        hammingWindow.push_back(value);
    }
    return hammingWindow;
}

const std::vector<double> FourierTransform::calculateHanningWindow(int N) {
    std::vector<double> hanningWindow(N, 0.0);

    for (int i = 0; i < N; ++i) {
        hanningWindow[i] = 0.5 * (1.0 - std::cos(2.0 * M_PI * i / (N - 1)));
    }

    return hanningWindow;
}

double FourierTransform::H_infinty_norm(const std::vector<double>& input,const std::vector<std::complex<double>> & output){
    std::vector<double> G_values(input.size());
    for (size_t i = 0; i < input.size(); ++i) {
        G_values[i] = std::abs(output[i]) / std::abs(input[i]);
    }
    double max = *std::max_element(G_values.begin(), G_values.end());
    std::cout << "H_infity Norm = " << max << std::endl;
    return max;
}

std::vector<double> FourierTransform::decodeAmplitudeSignal(const std::vector<double>& input){
    auto window = FourierTransform::getWindowFunction(input.size());
    std::vector<double> output(input.size());
    omp_set_num_threads(omp_get_max_threads());
    #pragma omp parallel for
    for (auto i = 0; i < input.size(); i++)
    {
        double val = window[i] * input[i];
        output[i] = val;
    }
    return output;
}
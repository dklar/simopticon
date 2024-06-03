#ifndef FOURIER_TRANSFORM_H
#define FOURIER_TRANSFORM_H

#include "types.h"
#include <complex>
#include <cmath>
#include <vector>
#include <algorithm>
#include <valarray>
#include <unsupported/Eigen/FFT>
#include <omp.h>
#include <iostream>


class FourierTransform {
public:
    FourierTransform();


    static std::vector<std::complex<double>> transform(const std::vector<double>& input);
    static std::vector<std::complex<double>> transform(const std::vector<std::complex<double>>& input);
    static std::vector<std::complex<double>> cooleyTukeyFFT(const std::vector<double>& input);
    static std::vector<std::complex<double>> cooleyTukeyFFT(const std::vector<std::complex<double>>& input);
    static std::vector<std::complex<double>> chirpZ_Transformaiton(const std::vector<double>& input);

    const static std::vector<double> getWindowFunction(int samples);
    const static std::vector<double> calculateHanningWindow(int N);
    std::vector<double> decodeAmplitudeSignal(const std::vector<double>& input);
    static double H_infinty_norm(const std::vector<double>& input,const std::vector<std::complex<double>> & output);
    
    


private:
    
};

#endif // FOURIER_TRANSFORM_H

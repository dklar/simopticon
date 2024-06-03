#include <gtest/gtest.h>
#include "helper.h"
#include "../FourierTransform.h"
#include "../FrequencyResponseAnalyzer.h"
#include "../MagnitudeSpectrumBuilder.h"
#include <cmath>
#include <complex>
#include <iomanip>
#include <iostream>
#include <fstream>

using namespace std::complex_literals;


TEST(FFT, cooleyTukeyFFT_test1) { 

    std::vector<std::complex<double>> input = {1.0, 2.0, 3.0, 4.0};
    std::vector<std::complex<double>>  result = FourierTransform::cooleyTukeyFFT(input);

    std::vector<std::complex<double>> expectedResults = {10.0,-2.0 + 2i,-2.0,-2.0 - 2i};
    ASSERT_EQ(result.size(), expectedResults.size());

    for (auto i = 0; i < result.size(); i++) {
        ASSERT_NEAR(result[i].real(), expectedResults[i].real(), 1e-6);
        ASSERT_NEAR(result[i].imag(), expectedResults[i].imag(), 1e-6); 
    }
}
TEST(FFT, cooleyTukeyFFT_test2) { 

    std::vector<double> input = {1.0, 2.0, 3.0, 4.0};
    std::vector<std::complex<double>>  result = FourierTransform::cooleyTukeyFFT(input);

    std::vector<std::complex<double>> expectedResults = {10.0,-2.0 + 2i,-2.0,-2.0 - 2i};
    ASSERT_EQ(result.size(), expectedResults.size());

    for (auto i = 0; i < result.size(); i++) {
        ASSERT_NEAR(result[i].real(), expectedResults[i].real(), 1e-6);
        ASSERT_NEAR(result[i].imag(), expectedResults[i].imag(), 1e-6); 
    }
}
TEST(FFT, chirpZTest) { 
    std::vector<double> input = {1.0, 2.0, 3.0, 4.0};
    std::vector<std::complex<double>>  result = FourierTransform::chirpZ_Transformaiton(input);

    std::vector<std::complex<double>> expectedResults = {10.0,-2.0 + 2i,-2.0,-2.0 - 2i};
    ASSERT_EQ(result.size(), expectedResults.size());

    for (auto i = 0; i < result.size(); i++) {
        ASSERT_NEAR(result[i].real(), expectedResults[i].real(), 1e-6);
        ASSERT_NEAR(result[i].imag(), expectedResults[i].imag(), 1e-6); 
    }
}
TEST(FFT, EigenTest) { 
    std::vector<double> input = {1.0, 2.0, 3.0, 4.0};
    std::vector<std::complex<double>>  result = FourierTransform::transform(input);

    std::vector<std::complex<double>> expectedResults = {10.0,-2.0 + 2i,-2.0,-2.0 - 2i};
    ASSERT_EQ(result.size(), expectedResults.size());

    for (auto i = 0; i < result.size(); i++) {
        ASSERT_NEAR(result[i].real(), expectedResults[i].real(), 1e-6);
        ASSERT_NEAR(result[i].imag(), expectedResults[i].imag(), 1e-6); 
    }
}
 

TEST(windowFunction, HammingWindow){
    std::vector<double> results = FourierTransform::getWindowFunction(50);
    std::vector<double> expectedResults;

    ASSERT_TRUE(readData("/home/dklar/Dokumente/pythonProjects/TestdataFiles/windowtest_hamming1.bin", expectedResults));
    ASSERT_EQ(results.size(), expectedResults.size());

    for (auto i = 0; i < results.size()-1; i++) {
        ASSERT_NEAR(results[i], expectedResults[i], 1e-6);
    }
}

TEST(windowFunction, HanningWindow){
    std::vector<double> results = FourierTransform::calculateHanningWindow(50);
    std::vector<double> expectedResults;
    ASSERT_TRUE(readData("/home/dklar/Dokumente/pythonProjects/TestdataFiles/windowtest_hanning1.bin",expectedResults));
    ASSERT_EQ(results.size(), expectedResults.size());

    for (auto i = 0; i < results.size()-1; i++) {
        ASSERT_NEAR(results[i], expectedResults[i], 1e-6);
    }
}

TEST(windowFunction, HammingWindow2){
    std::vector<double> results = FourierTransform::getWindowFunction(8001);
    std::vector<double> expectedResults;

    ASSERT_TRUE(readData("/home/dklar/Dokumente/pythonProjects/TestdataFiles/windowtest_hamming2.bin", expectedResults));
    ASSERT_EQ(results.size(), expectedResults.size());

    for (auto i = 0; i < results.size()-1; i++) {
        ASSERT_NEAR(results[i], expectedResults[i], 1e-6);
    }
}

TEST(windowFunction, HanningWindow2){
    std::vector<double> results = FourierTransform::calculateHanningWindow(8001);
    std::vector<double> expectedResults;
    ASSERT_TRUE(readData("/home/dklar/Dokumente/pythonProjects/TestdataFiles/windowtest_hanning2.bin",expectedResults));
    ASSERT_EQ(results.size(), expectedResults.size());

    for (auto i = 0; i < results.size()-1; i++) {
        ASSERT_NEAR(results[i], expectedResults[i], 1e-6);
    }
}


TEST(MagnitudeSpectrumBuilderTest,getFrequenciesTest){
    std::vector<testcaseinfo> testcases = readMagnitudeTestfile("/home/dklar/Dokumente/pythonProjects/TestdataFiles/magnitudeTest.txt");
    ASSERT_NE(testcases.size(),0);
    for (auto i :testcases){
        std::cout << i.inputFilename << " " << i.outputFilename << std::endl;
        std::vector<double> sineSignalX,sineSignalY;
        std::vector<double> magSignalX_expected,magSignalY_expected;
        std::vector<double> expected_freq;

        ASSERT_TRUE(readData(i.inputFilename,sineSignalX,sineSignalY));
        ASSERT_TRUE(readData(i.outputFilename,magSignalX_expected,magSignalY_expected));
        int size = sineSignalX.size();
        double fa = 1/(sineSignalX[1]-sineSignalX[0]);
        MagnitudeSpectrumBuilder mag;
        expected_freq = mag.getFrequencies(size,fa);
        for (auto i = 0; i < magSignalY_expected.size(); i++) {
            ASSERT_NEAR(expected_freq[i], magSignalX_expected[i], 1e-6)<< "Failed at index " << i;
        }
    }
}

TEST(MagnitudeSpectrumBuilderTest,getImageDomainDataTest){
    std::vector<double> x1,y1,x2,y2;
    ImageDomainVector value1,value2,value1_expected,value2_expected;
    ASSERT_TRUE(readData("/home/dklar/Dokumente/pythonProjects/TestdataFiles/amplitudeTestdataOutput_1.bin",value1_expected));
    ASSERT_TRUE(readData("/home/dklar/Dokumente/pythonProjects/TestdataFiles/amplitudeTestdataOutput_2.bin",value2_expected));
    ASSERT_TRUE(readData("/home/dklar/Dokumente/pythonProjects/TestdataFiles/amplitudeTestdataInput_1.bin",x1,y1));
    ASSERT_TRUE(readData("/home/dklar/Dokumente/pythonProjects/TestdataFiles/amplitudeTestdataInput_2.bin",x2,y2));
    MagnitudeSpectrumBuilder mag1,mag2;
    value1 = mag1.getImageDomainData(y1);
    value2 = mag2.getImageDomainData(y2);

    ASSERT_EQ(value1.size(), value1_expected.size());
    ASSERT_EQ(value2.size(), value2_expected.size());

    for (auto i = 0; i < value1.size(); i++) {
        ASSERT_NEAR(value1[i].real(),value1_expected[i].real(), 1e-3)<< "Failed at index " << i;
        ASSERT_NEAR(value1[i].imag(),value1_expected[i].imag(), 1e-3)<< "Failed at index " << i;
    }
    for (auto i = 0; i < value2.size(); i++) {
        ASSERT_NEAR(value2[i].real(),value2_expected[i].real(), 1e-3)<< "Failed at index " << i;
        ASSERT_NEAR(value2[i].imag(),value2_expected[i].imag(), 1e-3)<< "Failed at index " << i;
    }
}

TEST(MagnitudeSpectrumBuilderTest,MagnitudeDataTest){

    std::vector<testcaseinfo> testcases = readMagnitudeTestfile("/home/dklar/Dokumente/pythonProjects/TestdataFiles/magnitudeTestCase.txt");
    ASSERT_NE(testcases.size(),0);
    for (auto i :testcases){
        std::cout << i.inputFilename << " " << i.outputFilename << std::endl;
        std::vector<double> x,y,freq,mag;
        readData(i.inputFilename,x,y);
        readData(i.outputFilename,mag,freq);// TODO in python sind mag und freq vertauscht (y,x) anstatt (x,y)

        MagnitudeSpectrumBuilder magBuilder;
        std::vector<double> magCalulated = magBuilder.getSpectrumData(y);
        ASSERT_EQ(magCalulated.size(),mag.size());
        for (auto j = 0; j < magCalulated.size(); j++){
            ASSERT_NEAR(mag[j],magCalulated[j],1e-3);
        }
    }
}

TEST(MagnitudeSpectrumBuilderTest,Test2){

    std::vector<testcaseinfo> testcases = readfile("/home/dklar/Dokumente/pythonProjects/stableTestCases.txt");
    for (auto i :testcases){
        std::cout << i.inputFilename << " " << i.outputFilename << std::endl;
        std::vector<double> x_in,y_in,x_out,y_out;
        readData(i.inputFilename,x_in,y_in);
        readData(i.outputFilename,x_out,y_out);
        MagnitudeSpectrumBuilder mag;
        std::vector<double>  res = mag.getSpectrumData(y_in);
        auto maxElement = std::max_element(res.begin(), res.end());
        double maxValue = *maxElement;
        ASSERT_EQ(maxValue, i.max_y2);
    }


}


TEST(FrequencyResponseAnalyzerTest,StableTest){
    std::vector<testcaseinfo> testcases = readfile("/home/dklar/Dokumente/pythonProjects/TestdataFiles/freqResponse_stable.txt");
    ASSERT_NE(testcases.size(),0) << "ERROR no testcases found on disk";
    for (auto i :testcases){
        std::cout << i.inputFilename << " " << i.outputFilename << std::endl;
        std::vector<double> inputSignalX,inputSignalY,outputSignalX,outPutSignalY;
        ASSERT_TRUE(readData(i.inputFilename,inputSignalX,inputSignalY));
        ASSERT_TRUE(readData(i.outputFilename,inputSignalX,inputSignalY));
        ASSERT_EQ(inputSignalX.size(),inputSignalY.size());
        MagnitudeSpectrumBuilder inputMagBuilder,outputMagBuilder;
        std::vector<double> inputSpectrum  = inputMagBuilder.getSpectrumData(inputSignalY);
        std::vector<double> outputSpectrum = outputMagBuilder.getSpectrumData(outPutSignalY);
        double fa = 1./(inputSignalX[1] - inputSignalX[0]);
        FrequencyResponseAnalyzer analyzer(outputSpectrum,inputSpectrum,fa);
        ASSERT_EQ(i.max_y2,analyzer.getMaximum());

    }
}


int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
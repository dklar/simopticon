import control as ctrl
from control.matlab import rss, lsim
import numpy as np
import scipy 
from plots import *
from calculations import *
from h_infintyAproximator import *
from constants import *
import struct
import os
import sys


def createMagnitudeTestValues():
    try:
        info_filename = "magnitudeTest.txt"
        with open(info_filename, 'w') as info_file:
            numberExamples = 5
            for i in range(numberExamples):
                numberFreq = 55
                amplitudes = np.random.randint(1, 11, size=numberFreq, dtype=int)
                frequencies = np.random.uniform(radToHz(10**-1), radToHz(10**1), size=numberFreq)
                y,x = createSineSamplesWithOversampling(amplitudes,frequencies,0,100,16)
                y2, x2 = calculateMagnitudeSpectrum(y,x,True)
                y2 = np.round(y2,5)
                input_filename = f"magSpecInput_{i}.bin"
                output_filename = f"magSpecOutput_{i}.bin"

                input_filepath = os.path.abspath(os.path.join(os.path.dirname(__file__), "TestdataFiles", input_filename))
                output_filepath = os.path.abspath(os.path.join(os.path.dirname(__file__), "TestdataFiles", output_filename))

                with open(input_filename, 'wb') as file:
                    for value in x:
                        file.write(struct.pack('d', value))
                    for value in y:
                        file.write(struct.pack('d', value))
                with open(output_filename, 'wb') as file:
                    for value in x2:
                        file.write(struct.pack('d', value))
                    for value in y2:
                        file.write(struct.pack('d', value))
                if (i==numberExamples-1):
                    info_file.write(f"{input_filepath}\n{output_filepath}")
                else:
                    info_file.write(f"{input_filepath}\n{output_filepath}\n")

    except Exception as e:
        print("Error writing data to the file: {}".format(e))

def generateNormTestdata():

    testcases_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "TestdataFiles"))
    os.makedirs(testcases_dir, exist_ok=True)
    info_filename = os.path.join(testcases_dir, "freqResponse_stable.txt")
    G_i = stableTransferfunctions
    try:
        with open(info_filename, 'w') as info_file:
            for i in range(len(G_i)):

                G = G_i[i]
                numberFreq = 55
                amplitudes = np.random.randint(1, 11, size=numberFreq, dtype=int)
                frequencies = np.random.uniform(radToHz(10**-1), radToHz(10**2), size=numberFreq)
                x1, y1 = createSineSamplesWithOversampling(amplitudes, frequencies, 0, 100, 4)  
                y2, x2, _ = lsim(G, x1, T=y1)
                magSpecInput, freqInput = calculateMagnitudeSpectrum(y1,x1,True)
                magSpecOutput, freqOutput = calculateMagnitudeSpectrum(y2,x2,True)
                approximator = hInfinityApproximator()
                approximator.addFreqResponsePoints(magSpecInput,magSpecOutput,freqInput)
                approximator.calcuateFreqResponse()
                maxX,maxY = approximator.getNorm()

                input_filename = "inputsignal{:d}.bin".format(i)
                output_filename = "outputsignal{:d}.bin".format(i)
                input_filepath = os.path.join(testcases_dir, input_filename)
                output_filepath = os.path.join(testcases_dir, output_filename)


                with open(input_filepath, 'wb') as file:
                    for value in x1:
                        file.write(struct.pack('d', value))
                    for value in y1:
                        file.write(struct.pack('d', value))

                with open(output_filepath, 'wb') as file:
                    for value in x2:
                        file.write(struct.pack('d', value))
                    for value in y2:
                        file.write(struct.pack('d', value))

 
                if maxY>1e6:
                    maxY =1e6

                if (i == len(G_i)-1):
                    info_file.write(f"{str(G)[1:]}{input_filepath}\n{output_filepath}\n{maxY}")
                else:
                    info_file.write(f"{str(G)[1:]}{input_filepath}\n{output_filepath}\n{maxY}\n")

    except Exception as e:
        print("Error writing data to the file: {}".format(e))

def generateFreqResponseData():
    testcases_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "TestdataFiles"))
    os.makedirs(testcases_dir, exist_ok=True)
    info_filename = os.path.join(testcases_dir, "freqResponse_stable.txt")
    input_filepath = os.path.join(testcases_dir, "inputsignal.bin")
    magSpecInput_filepath = os.path.join(testcases_dir, "inputSpectrum.bin")
    G_i = stableTransferfunctions
    numberFreq = 55
    amplitudes = np.random.randint(1, 11, size=numberFreq, dtype=int)
    frequencies = np.random.uniform(radToHz(10**-1), radToHz(10**2), size=numberFreq)
    u, t1 = createSineSamplesWithOversampling(amplitudes, frequencies, 0, 100, 2)
    magSpecInput,  freq1 = calculateMagnitudeSpectrum(u,t1,True)

    try:
        with open(input_filepath, 'wb') as file:
            for value in t1:
                file.write(struct.pack('d', value))
            for value in u:
                file.write(struct.pack('d', value))
        with open(magSpecInput_filepath, 'wb') as file:
            for value in freq1:
                file.write(struct.pack('d', value))
            for value in magSpecInput:
                file.write(struct.pack('d', value))
    except Exception as e:
        print("Error writing data to the file: {}".format(e))
    try:
        with open(info_filename, 'w') as info_file:
            for i in range(len(G_i)):
                G = G_i[i]
                y, x, _ = lsim(G, u, T=t1)
                omega,gain = calculateAproxFreqResponse(y,u,t1)
                magSpecOut,freq2 = calculateMagnitudeSpectrum(y,x,True)
                output_filepath = os.path.join(testcases_dir, "outputsignal_{:d}.bin".format(i))
                freqRes_filepath = os.path.join(testcases_dir, "freqResponse_{:d}.bin".format(i))
                magSpecOut_filepath = os.path.join(testcases_dir, "outputSpectrum_{:d}.bin".format(i))
                with open(freqRes_filepath, 'wb') as file:
                    for value in omega:
                        file.write(struct.pack('d', value))
                    for value in gain:
                        file.write(struct.pack('d', value))
                with open(output_filepath, 'wb') as file:
                    for value in x:
                        file.write(struct.pack('d', value))
                    for value in y:
                        file.write(struct.pack('d', value))
                with open(magSpecOut_filepath, 'wb') as file:
                    for value in freq2:
                        file.write(struct.pack('d', value))
                    for value in magSpecOut:
                        file.write(struct.pack('d', value))
                if (i == len(G_i)-1):
                    info_file.write(f"{str(G)[1:]}{input_filepath}\n{output_filepath}\n{freqRes_filepath}\n{magSpecInput_filepath}\n{magSpecOut_filepath}")
                else:
                    info_file.write(f"{str(G)[1:]}{input_filepath}\n{output_filepath}\n{freqRes_filepath}\n{magSpecInput_filepath}\n{magSpecOut_filepath}\n")

    except Exception as e:
        print("Error writing data to the file: {}".format(e))

def generateFreqResponseDataInstable():
    testcases_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "TestdataFiles"))
    os.makedirs(testcases_dir, exist_ok=True)
    info_filename = os.path.join(testcases_dir, "freqResponse_instable.txt")
    input_filepath = os.path.join(testcases_dir, "inputsignalInstable.bin")
    magSpecInput_filepath = os.path.join(testcases_dir, "inputSpectrumInstable.bin")
    G_i = instableTransferfunctions
    numberFreq = 55
    amplitudes = np.random.randint(1, 11, size=numberFreq, dtype=int)
    frequencies = np.random.uniform(radToHz(10**-1), radToHz(10**2), size=numberFreq)
    u, t1 = createSineSamplesWithOversampling(amplitudes, frequencies, 0, 100, 2)
    magSpecInput,  freq1 = calculateMagnitudeSpectrum(u,t1,True)

    try:
        with open(input_filepath, 'wb') as file:
            for value in t1:
                file.write(struct.pack('d', value))
            for value in u:
                file.write(struct.pack('d', value))
        with open(magSpecInput_filepath, 'wb') as file:
            for value in freq1:
                file.write(struct.pack('d', value))
            for value in magSpecInput:
                file.write(struct.pack('d', value))
    except Exception as e:
        print("Error writing data to the file: {}".format(e))
    try:
        with open(info_filename, 'w') as info_file:
            for i in range(len(G_i)):
                G = G_i[i]
                y, x, _ = lsim(G, u, T=t1)
                omega,gain = calculateAproxFreqResponse(y,u,t1)
                magSpecOut,freq2 = calculateMagnitudeSpectrum(y,x,True)
                output_filepath = os.path.join(testcases_dir, "outputsignalInstable_{:d}.bin".format(i))
                freqRes_filepath = os.path.join(testcases_dir, "freqResponseInstable_{:d}.bin".format(i))
                magSpecOut_filepath = os.path.join(testcases_dir, "outputSpectrumInstable_{:d}.bin".format(i))
                with open(freqRes_filepath, 'wb') as file:
                    for value in omega:
                        file.write(struct.pack('d', value))
                    for value in gain:
                        file.write(struct.pack('d', value))
                with open(output_filepath, 'wb') as file:
                    for value in x:
                        file.write(struct.pack('d', value))
                    for value in y:
                        file.write(struct.pack('d', value))
                with open(magSpecOut_filepath, 'wb') as file:
                    for value in freq2:
                        file.write(struct.pack('d', value))
                    for value in magSpecOut:
                        file.write(struct.pack('d', value))
                if (i == len(G_i)-1):
                    info_file.write(f"{str(G)[1:]}{input_filepath}\n{output_filepath}\n{freqRes_filepath}\n{magSpecInput_filepath}\n{magSpecOut_filepath}")
                else:
                    info_file.write(f"{str(G)[1:]}{input_filepath}\n{output_filepath}\n{freqRes_filepath}\n{magSpecInput_filepath}\n{magSpecOut_filepath}\n")

    except Exception as e:
        print("Error writing data to the file: {}".format(e))


def read_binary_file(file_path):
    with open(file_path, 'rb') as file:
        data = file.read()
        double_size = struct.calcsize('d')
        num_values = len(data) // double_size
        values = struct.unpack('d' * num_values, data)
        half_length = num_values // 2
        x_values = values[:half_length]
        y_values = values[half_length:]
    
    return x_values, y_values

def backtrace_generateFreqResponseData():
    testcases_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "TestdataFiles"))
    info_filename = os.path.join(testcases_dir, "freqResponse_stable.txt")
    with open(info_filename, 'r') as text_file:
        lines = text_file.readlines()
        for i in range(0, len(lines), 8):
            numerator = lines[i].strip() 
            denominator = lines[i + 2].strip() 
            file_paths = [line.strip() for line in lines[i + 3:i + 8]]

            print(f"{numerator}\n{'-' * len(denominator)}\n{denominator}")
            x_in = []
            y_in = []
            x_out = []
            y_out = []
            x_freqResponse = []
            y_freqResponse = []
            x_spec = []
            y_spec = []
            x_spec2 = []
            y_spec2 = []
            for j, file_path in enumerate(file_paths):
                if os.path.exists(file_path):
                    if j == 0:
                        x_in, y_in = read_binary_file(file_path)
                    if j == 1:
                        x_out, y_out = read_binary_file(file_path)
                    if j == 2:
                        x_freqResponse, y_freqResponse = read_binary_file(file_path)
                    if j == 3:
                        x_spec, y_spec = read_binary_file(file_path)
                    if j == 4:
                        x_spec2, y_spec2 = read_binary_file(file_path)
                    print(f"\nFile: {file_path}")
            mag1,freq1 = calculateMagnitudeSpectrum(y_in,x_in,True)
            mag2,freq2 = calculateMagnitudeSpectrum(y_out,x_out,True)
            omega,gain = calculateAproxFreqResponse(y_out,y_in,x_in)
            for i in range(len(gain)):
                assert gain[i] == y_freqResponse[i], "Fehler"
            for i in range(len(mag1)):
                assert mag1[i] == y_spec[i], "Fehler"
            for i in range(len(mag2)):
                assert mag2[i] == y_spec2[i], "Fehler"
            print("\n" + "=" * 30 + "\n")

def generateMagnitudeSpectrumTestData2():
    testcases_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "TestdataFiles"))
    os.makedirs(testcases_dir, exist_ok=True)
    info_filename = os.path.join(testcases_dir, "magnitudeTestCase.txt")
    try:
        freq_i = [[1,5,10],[7,9,23],[7,9,23],
                  [12,20,10],[3,25,12],[7,2,21]]
        amp_i = [[1,2,3],[4,5,6],[7,8,9],
                  [9,8,7],[6,5,4],[3,2,1]
                ]

        with open(info_filename, 'w') as info_file:
            for i in range(len(freq_i)):
                amplitudes = amp_i[i]
                frequencies = freq_i[i]
                y1, x1 = createSineSamplesWithOversampling(amplitudes, frequencies, 0, 100, 2)  

                mag,freq = calculateMagnitudeSpectrum(y1,x1,True)

                file1Path = os.path.join(testcases_dir, "signal_input{:d}.bin".format(i))
                file2Path = os.path.join(testcases_dir, "spectrum_expected{:d}.bin".format(i))

                with open(file1Path, 'wb') as file:
                    for value in x1:
                        file.write(struct.pack('d', value))
                    for value in y1:
                        file.write(struct.pack('d', value))

                with open(file2Path, 'wb') as file:
                    for value in freq:
                        file.write(struct.pack('d', value))
                    for value in mag:
                        file.write(struct.pack('d', value))

                if (i == len(freq_i)-1):
                    info_file.write(f"{file1Path}\n{file2Path}")
                else:
                    info_file.write(f"{file1Path}\n{file2Path}\n")

    except Exception as e:
        print("Error writing data to the file: {}".format(e))

def generateWindowTestData():
    hanning = np.hanning(50)
    hamming = np.hamming(50)
    with open("windowtest_hanning1.bin", 'wb') as file:
        for value in hanning:
            file.write(struct.pack('d', value))
    with open("windowtest_hamming1.bin",'wb') as file:
        for value in hamming:
            file.write(struct.pack('d', value))
    hanning = np.hanning(8001)
    hamming = np.hamming(8001)
    with open("windowtest_hanning2.bin", 'wb') as file:
        for value in hanning:
            file.write(struct.pack('d', value))
    with open("windowtest_hamming2.bin",'wb') as file:
        for value in hamming:
            file.write(struct.pack('d', value))

def generateAmplitudenSpektrumTestValues():
    numberFreq = 6
    amplitudes = np.random.randint(1, 11, size=numberFreq, dtype=int)
    frequencies = np.random.randint(1, 80, size=numberFreq)
    x1, y1 = createSineSamplesWithOversampling(amplitudes, frequencies, 0, 100, 16)  
    x2, y2 = createSineSamplesWithOversampling([7], [5], 0, 100, 16)
    res1 = calculateImageDomainData(y1,x1)
    res2 = calculateImageDomainData(y2,x2)

    with open("amplitudeTestdataInput_1.bin", 'wb') as file:
        for value in x1:
            file.write(struct.pack('d', value))
        for value in y1:
            file.write(struct.pack('d', value))
    with open("amplitudeTestdataInput_2.bin",'wb') as file:
        for value in x2:
            file.write(struct.pack('d', value))
        for value in y2:
            file.write(struct.pack('d', value))
    with open("amplitudeTestdataOutput_1.bin", 'wb') as file:
        for value in res1:
            file.write(struct.pack('d', value.real))
            file.write(struct.pack('d', value.imag))
    with open("amplitudeTestdataOutput_2.bin",'wb') as file:
        for value in res2:
            file.write(struct.pack('d', value.real))
            file.write(struct.pack('d', value.imag))

def generateHinfTestDataStable():
    testcases_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "TestdataFiles"))
    os.makedirs(testcases_dir, exist_ok=True)
    info_filename = os.path.join(testcases_dir, "HinfNorm.txt")
    input_filepath = os.path.join(testcases_dir, "HinfNormInputSignal.bin")
    G_i = stableTransferfunctions
    numberFreq = 55
    amplitudes = np.random.randint(1, 11, size=numberFreq, dtype=int)
    frequencies = np.random.uniform(radToHz(10**-1), radToHz(10**2), size=numberFreq)
    u, t1 = createSineSamplesWithOversampling(amplitudes, frequencies, 0, 100, 2)


    try:
        with open(input_filepath, 'wb') as file:
            for value in t1:
                file.write(struct.pack('d', value))
            for value in u:
                file.write(struct.pack('d', value))
    except Exception as e:
        print("Error writing data to the file: {}".format(e))
    try:
        with open(info_filename, 'w') as info_file:
            for i in range(len(G_i)):
                G = G_i[i]
                y, x, _ = lsim(G, u, T=t1)
                omega,gain = calculateAproxFreqResponse(y,u,t1)
                maxgain = np.max(gain)
                maxfreq = omega[np.argmax(gain)]
                with open(input_filepath, 'wb') as file:
                    for value in t1:
                        file.write(struct.pack('d', value))
                    for value in u:
                        file.write(struct.pack('d', value))
                output_filepath = os.path.join(testcases_dir, "HinfNormOutputsignal_{:d}.bin".format(i))

                with open(output_filepath, 'wb') as file:
                    for value in x:
                        file.write(struct.pack('d', value))
                    for value in y:
                        file.write(struct.pack('d', value))

                if (i == len(G_i)-1):
                    info_file.write(f"{str(G)[1:]}{input_filepath}\n{output_filepath}\n{maxgain}\n{maxfreq}")
                else:
                    info_file.write(f"{str(G)[1:]}{input_filepath}\n{output_filepath}\n{maxgain}\n{maxfreq}\n")

    except Exception as e:
        print("Error writing data to the file: {}".format(e))

def generateHinfTestDataInstable():
    testcases_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "TestdataFiles"))
    os.makedirs(testcases_dir, exist_ok=True)
    info_filename = os.path.join(testcases_dir, "HinfNormInstable.txt")
    input_filepath = os.path.join(testcases_dir, "HinfNormInstableInputSignal.bin")
    G_i = instableTransferfunctions
    numberFreq = 55
    amplitudes = np.random.randint(1, 11, size=numberFreq, dtype=int)
    frequencies = np.random.uniform(radToHz(10**-1), radToHz(10**2), size=numberFreq)
    u, t1 = createSineSamplesWithOversampling(amplitudes, frequencies, 0, 100, 2)


    try:
        with open(input_filepath, 'wb') as file:
            for value in t1:
                file.write(struct.pack('d', value))
            for value in u:
                file.write(struct.pack('d', value))
    except Exception as e:
        print("Error writing data to the file: {}".format(e))
    try:
        with open(info_filename, 'w') as info_file:
            for i in range(len(G_i)):
                G = G_i[i]
                y, x, _ = lsim(G, u, T=t1)
                omega,gain = calculateAproxFreqResponse(y,u,t1)
                if len(omega) == 0 or len(gain) == 0:
                    maxfreq = -1
                    maxgain = -1
                else:
                    maxgain = np.max(gain)
                    maxfreq = omega[np.argmax(gain)]
                with open(input_filepath, 'wb') as file:
                    for value in t1:
                        file.write(struct.pack('d', value))
                    for value in u:
                        file.write(struct.pack('d', value))
                output_filepath = os.path.join(testcases_dir, "HinfNormInstableOutputsignal_{:d}.bin".format(i))

                with open(output_filepath, 'wb') as file:
                    for value in x:
                        file.write(struct.pack('d', value))
                    for value in y:
                        file.write(struct.pack('d', value))

                if (i == len(G_i)-1):
                    info_file.write(f"{str(G)[1:]}{input_filepath}\n{output_filepath}\n{maxgain}\n{maxfreq}")
                else:
                    info_file.write(f"{str(G)[1:]}{input_filepath}\n{output_filepath}\n{maxgain}\n{maxfreq}\n")

    except Exception as e:
        print("Error writing data to the file: {}".format(e))

generateHinfTestDataInstable()
#generateHinfTestDataStable()
#generateFreqResponseDataInstable()
#generateFreqResponseData()
#generateFreqResponseData()
#generateMagnitudeSpectrumTestData2()
#createMagnitudeTestValues()
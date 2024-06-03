import control as ctrl
import numpy as np
import scipy 
from scipy import signal

def radToHz(w):
    return w/(2*np.pi)

def HzToRad(f):
    return 2*np.pi*f

def toDecibel(x):
    return 20 * np.log10(x)

def createSineSamplesWithOversampling(amplitudes,frequencies,start=0,stop=1,oversampling=8):
    """
    Creates a composite sine signal with specified amplitudes and frequencies.

    Parameters:
    - amplitudes (array-like): Amplitudes of individual sine components.
    - frequencies (array-like): Frequencies of individual sine components.(in Herz)
    - start (float, optional): Start time of the signal. Default is 0.
    - stop (float, optional): Stop time of the signal. Default is 1.
    - oversampling (int, optional): Oversampling factor for the signal. Default is 8.


    Returns:
    - tuple: A tuple containing the composite sine signal values and the corresponding time axis.

    Example:
    - Create a composite sine signal with specified amplitudes and frequencies:
    - y(x) = 1*sin(2 * pi * 2) + 3*sin(2 * pi * 4) + 5*sin(2 * pi * 6):

      >>> amplitudes = [1, 3, 5]
      >>> frequencies = [2, 4, 6]
      >>> signal, time_axis = createSineSamples(amplitudes, frequencies)
    """
    max_frequency = max(frequencies)
    fs = 2 * max_frequency
    fs_oversampling = oversampling * fs
    print("Minimumfrequency = {:.2f} Hz, selected frequency= {:.2f}Hz".format(fs,fs_oversampling))
    t = np.linspace(start, stop, int(fs_oversampling * (stop - start)), endpoint=False)
    sinus_signal = np.zeros_like(t, dtype=float)
    for amp, freq in zip(amplitudes, frequencies):
        sinus_signal += amp * np.sin(2 * np.pi * freq * t)
    return sinus_signal, t

def createSineSamples(amplitudes,frequencies,start=0,stop=1,samples=100):
    t = np.linspace(start, stop, samples, endpoint=False)
    sinus_signal = np.zeros_like(t, dtype=float)
    for amp, freq in zip(amplitudes, frequencies):
        sinus_signal += amp * np.sin(2 * np.pi * freq * t)
    return sinus_signal, t

def createStepfunction(start = 0,stop = 10,samples = 100,deadtime=1):
    """
    Creates a step function with a specified deadtime.

    Parameters:
    - start (float, optional): Start time of the step function. Default is 0.
    - stop (float, optional): Stop time of the step function. Default is 10.
    - samples (int, optional): Number of samples for the step function. Default is 100.
    - deadtime (float, optional): Duration of the deadtime before the step occurs. Default is 1.

    Returns:
    - tuple: A tuple containing the step function values and the corresponding time axis.

    Example:
    - Create a step function with a deadtime of 1 unit at default settings:
      >>> step_function, time_axis = createStepFunction()
    """
    T = np.linspace(start,stop,samples)
    u = np.zeros_like(T)
    u[T >= deadtime] = 1
    return u, T

def createSquareSignal(start,stop,num):
    t = np.linspace(start, stop, num, endpoint=False)
    return signal.square(2 * np.pi * 5 * t)


def calculateImageDomainData(values,time,window=True):
    dt = time[1] - time[0]
    fa = 1.0/dt 
    N = int(len(values) / 2 +1)
    X = np.linspace(0, fa/2, N, endpoint=True)
    if window:
        han = np.hanning(len(values))
        values = han*values

    return np.fft.fft(values)

def calculateMagnitudeSpectrum(values,time,window=True):
    """
    Calculates the magnitudes of the Fast Fourier Transform (FFT) of a given signal.

    Parameters:
    - values (array-like): Input signal for FFT.
    - time (array-like): Time values corresponding to the signal.
    - window (bool, optional): Apply Hanning window to the signal if True. Default is True.

    Returns:
    - tuple: A tuple containing the FFT magnitudes and the corresponding frequency axis.

    Example:
    >>> values = np.sin(2 * np.pi * 5 * np.linspace(0, 1, 1000))
    >>> time = np.linspace(0, 1, 1000)
    >>> magnitudes, frequencies = calculateMagnitudeSpectrum(values, time)
    """
    dt = time[1] - time[0]
    fa = 1.0/dt 
    #print("dt={:.5f}s (Sample Time); fa={:.5f}Hz (Frequency)".format(dt,fa))
    N = int(len(values) / 2 +1)
    X = np.linspace(0, fa/2, N, endpoint=True)
    if window:
        window = np.hanning(len(values))
        values = window*values

    fftResult = np.fft.fft(values)
    fftResultMagnitude = 2.0*np.abs(fftResult[:N])/N
    return  fftResultMagnitude,X

def calculateAproxFreqResponse(y:np.ndarray, u:np.ndarray, t:np.ndarray,minimumPeak = 3,dB=True,hertz=False):
    """
    Calculate the approximate frequency response from input-output data.

    Parameters:
    - y (np.ndarray): Output signal.
    - u (np.ndarray): Input signal.
    - t (np.ndarray): Time vector corresponding to the input and output signals.
    - minimumPeak (float, optional): Minimum peak value to consider in the frequency response.
      Defaults to 3.
    - dB (bool, optional): If True, the gain will be returned in decibels. Defaults to True.
    - hertz (bool, optional): If True, frequencies will be returned in Hertz. If False,
      frequencies will be in radians per second. Defaults to False.

    Returns:
    - freq (ndarray): Array of frequencies.
    - gain (ndarray): Array of corresponding gains.

    Notes:
    - The function calculates the magnitude spectrum of the input and output signals and
      computes the frequency response based on their absolute values.
    - The resulting frequency and gain arrays can be converted to decibels and radians per
      second or Hertz, depending on the optional parameters.
    - A peak is considered if either the output or input signal has an absolute value
      greater than the specified minimum peak value.

    Examples:
    >>> freq, gain = calculateAproxFreqResponse(output, input, time)
    >>> freq, gain = calculateAproxFreqResponse(output, input, time, dB=True, hertz=True)
    >>> freq, gain = calculateAproxFreqResponse(output, input, time, minimumPeak=5)
    """
    if len(y) != len(u) or len(y) != len(t) or len(u) != len(t):
        print("Error: y, u, and t must have the same length")
        return None, None
    magSpecInput,  freq1 = calculateMagnitudeSpectrum(u,t,True)
    magSpecOutput, freq2 = calculateMagnitudeSpectrum(y,t,True)
    gain = []
    freq = []
    for i in range(len(magSpecInput)):
        absOut = np.abs(magSpecOutput[i])
        absIn  = np.abs(magSpecInput[i])
        if (absOut > minimumPeak or absIn > minimumPeak) and (absOut < 1e9):
            validGain = absOut / absIn
            validFreq = freq1[i]
            gain.append(validGain)
            freq.append(validFreq)
    gain = np.array(gain)
    freq = np.array(freq)
    if dB:
        gain = toDecibel(gain)
    if not hertz:
        freq = HzToRad(freq)
    print(f"Valid Gains: {len(freq)}")
    return freq, gain



def calculateFrequencyResponse(G, inputSamples):
    """
    Calculate the time response of a transfer function for given input samples.

    Parameters:
    - G: Transfer function (ctrl.TransferFunction or signal.TransferFunction)
    - inputSamples: Input frequencies

    Returns:
    - complexTransferFunction: Complex Transfer Function
    """
    if isinstance(G, ctrl.TransferFunction):
        mag, phase, _ = ctrl.frequency_response(G, inputSamples)
        H_complex = mag * np.exp(1j * phase)
        complexTransferFunction = H_complex 
    elif isinstance(G, signal.TransferFunction):
        mag, H_complex = signal.freqresp(G, inputSamples)
        complexTransferFunction = H_complex 
    else:
        raise ValueError("Invalid type for transfer function")
    return complexTransferFunction

def H_infinity_calculation_time(inputSpectrum:np.ndarray,outputSpectrum:np.ndarray):
    G = np.abs(outputSpectrum)/np.abs(inputSpectrum)#outputSpectrum/inputSpectrum #
    max = np.max(G)
    return max ,G

def getLocalMaxima(y, x):
    x_maxima = []
    y_maxima = []
    for i in range(1, len(y)-1):
        if y[i-1] < y[i] > y[i+1]:
            y_maxima.append(y[i])
            x_maxima.append(x[i])
    return x_maxima, y_maxima

def getLocalMinima(y, x):
    x_minima = []
    y_minima = []
    for i in range(1, len(y)-1):
        if y[i-1] > y[i] < y[i+1]:
            y_minima.append(y[i])
            x_minima.append(x[i])
    return x_minima, y_minima

def getMaximum(magnitudes:np.ndarray,frequencies:np.ndarray):
    absoluteMaximum = np.max(magnitudes)
    absoluteMaximum_dB = 20*np.log10(absoluteMaximum)
    peakFrequency = frequencies[np.argmax(magnitudes)]
    return [peakFrequency,absoluteMaximum_dB]

def getMaximum(G:ctrl.TransferFunction,inputSamples:np.ndarray):
    magnitudes, _, _ = ctrl.frequency_response(G, inputSamples)# Frequenzvektor 
    absoluteMaximum = np.max(magnitudes)# Magnitudenmaximum
    absoluteMaximum_dB = 20*np.log10(absoluteMaximum)
    peakFrequency = inputSamples[np.argmax(magnitudes)]# Peakfrequenz
    print(G)
    print("Magnitudenmaximum: {:.4f}dB ({:.4f}) bei w = {:.4f} rad/sec".format(absoluteMaximum_dB,absoluteMaximum,peakFrequency))
    return [peakFrequency,absoluteMaximum_dB]

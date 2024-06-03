import control as ctrl
from control.matlab import rss, lsim
import numpy as np
import scipy 
import matplotlib.pyplot as plt
from plots import *
from calculations import *



class hInfinityApproximator:

    def __init__(self,funcTocall):
        self.ymax = -100.0
        self.xmax = -100.0
        self.validFreqResponse = {}
        self._systemValueCreator = funcTocall


    def addFreqResponsePoints(self, inputSpectrum:np.ndarray,outputSpectrum:np.ndarray,freq:np.ndarray,minimumPeak=3):
        for i in range(len(outputSpectrum)):
            absOut = np.abs(outputSpectrum[i])
            absIn  = np.abs(inputSpectrum[i])
            if (absOut > minimumPeak or absIn > minimumPeak):
                validGain = absOut / absIn
                validFreq = freq[i]
                if validFreq in self.validFreqResponse:
                    self.validFreqResponse[validFreq].append(validGain)
                else:
                    self.validFreqResponse[validFreq] = [validGain]

    def calcuateFreqResponse(self, use_mean=False):
            x = []
            y = []
            sorted_freq_response = dict(sorted(self.validFreqResponse.items()))
            for freq, gains in sorted_freq_response.items():
                x.append(freq)

                if use_mean:
                    tmp = np.mean(gains)
                    y.append(tmp)
                    if tmp > self.ymax:
                        self.ymax = tmp
                        self.xmax = freq
                else:
                    tmp = np.median(gains)
                    y.append(tmp)
                    if tmp > self.ymax:
                        self.ymax = tmp
                        self.xmax = freq
            return x, y
        
    def getNorm(self):
        return self.xmax,self.ymax

    def generateNewValues(self,*args):
        if self._systemValueCreator is None:
            print("Error: function is not set. Cannot generate new values.")
            return
        counter = 0
        while len(self.validFreqResponse) < 200 and counter<100:
            magSpecInput, magSpecOutput, freq = self._systemValueCreator(args[0])
            self.addFreqResponsePoints(magSpecInput, magSpecOutput,freq)
            counter+=1




def calculateAproxFreqResponse(system, startDecade=-2, endDecade=1,minimumPeak=2):
    frequencies = np.logspace(startDecade, endDecade, 50)
    validFreq = []
    validGain = []
    for freq in frequencies:
        amplitude = np.random.uniform(1, 20)
        u1, T1 = createSineSamplesWithOversampling([amplitude], [freq], 0, 100, 2)#amplitude*sin(2*pi*freq)
        yout, Tout,_ = lsim(system,u1, T=T1)
        magSpecInput, _ = calculateMagnitudeSpectrum(u1,T1,False)
        magSpecOutput, freqOutput = calculateMagnitudeSpectrum(yout,Tout,False)
        for i in range(len(magSpecOutput)):
            absOut = np.abs(magSpecOutput[i])
            absIn  = np.abs(magSpecInput[i])
            if (absOut > minimumPeak and absIn > minimumPeak):
                validGain.append(absOut / absIn)
                validFreq.append(HzToRad(freqOutput[i]))
    return validFreq,validGain



def generateMagnitudSpectrum(G):
    numberFreq = 15
    amplitudes = np.random.randint(1, 11, size=numberFreq, dtype=int)
    frequencies = np.random.uniform(radToHz(10**-1), radToHz(10**2), size=numberFreq)
    #u1,T1 = createSineSamples(amplitudes,frequencies,0,100,3000)#Abtastfrequenz Problem
    u1, T1 = createSineSamplesWithOversampling(amplitudes, frequencies, 0, 100, 32)  
    yout, Tout,_ = lsim(G,u1, T=T1)
    magSpecInput, freqInput = calculateMagnitudeSpectrum(u1,T1,False)
    magSpecOutput, freqOutput = calculateMagnitudeSpectrum(yout,Tout,False)
    return magSpecInput,magSpecOutput,HzToRad(freqOutput)

def relativeError(xAprox, xAccurate, yAccurate, yAprox):
    relativeErrorY = []
    relativeErrorX = []

    for w in range(len(xAccurate)):
        if xAprox[-1] >= xAccurate[w]:
            relativeErrorY.append(((yAccurate[w] - yAprox[w]) / yAccurate[w]) * 100)
            relativeErrorX.append(xAccurate[w])
    return relativeErrorY,relativeErrorX

def großerTest():
    num = np.random.randint(1, 11, dtype=int)
    denum = np.random.randint(-20, 20, size=3)
    #G = ctrl.tf([1], [0.25, -0.125, 1,0])
    G = ctrl.tf(num,denum)
    approximator = hInfinityApproximator(generateMagnitudSpectrum)


    approximator.generateNewValues(G)
    freq,gainAprox = approximator.calcuateFreqResponse()
    _,norm_value = approximator.getNorm()

    
    omega = np.logspace(-1, 2, 1001) 
    gainAccurate = abs(calculateFrequencyResponse(G,omega))

    interp_func = scipy.interpolate.interp1d(freq, gainAprox, kind='nearest-up', fill_value='extrapolate')
    gainAproxInterpolated = interp_func(omega)
    
    relativeErrorY, relativeErrorX = relativeError(freq, omega, gainAccurate, gainAproxInterpolated)
    average_error=sum(relativeErrorY)/len(relativeErrorY)

    print(G)
    print("Genaue  H unendlich norm: {:.5g}".format(max(gainAccurate)))
    print("Approx. H unendlich norm: {:.5g}".format(max(gainAproxInterpolated)))
    print("Durchschnittlicher Fehler der aprox. Frequenzantwort = {:.3g}%".format(average_error))

    # gainAprox_dB = toDecibel(gainAprox)
    # gainAccurate_dB = toDecibel(gainAccurate)
    # fig, axis = plt.subplots(1,2)
    # setFreqResponsePlot(axis[1],gainAccurate_dB, omega, title="Frequency Response")
    # axis[1].plot(interp_func.x,toDecibel(interp_func.y))
    # axis[1].scatter(freq,gainAprox_dB,color='red')

    # axis[0].plot(relativeErrorX,relativeErrorY)
    # axis[0].set_title("Relativer Fehler der Frequenzantwort\nDurchschnittlicher Fehler: {:.4f}%".format(average_error))
    # axis[0].set_ylabel('Relativer Fehler in %')
    # axis[0].set_xlabel('Frequenz in $[rad/sec]$')
    # axis[0].grid(True)
    # plt.show()

def plotExample():
    G_stable =ctrl.TransferFunction(ctrl.tf([1], [1, 13, 40])) #[4], [1, 2, 4]))#
    approximator = hInfinityApproximator(generateMagnitudSpectrum)
    approximator.generateNewValues(G_stable)

    
    freq,gainAprox = approximator.calcuateFreqResponse()
    _,norm_value = approximator.getNorm()

    
    omega = np.logspace(-1, 2, 1001) 
    gainAccurate = abs(calculateFrequencyResponse(G_stable,omega))

    gainAprox_dB = toDecibel(gainAprox)
    gainAccurate_dB = toDecibel(gainAccurate)

    interp_func = scipy.interpolate.interp1d(freq, gainAprox, kind='nearest-up', fill_value='extrapolate')
    gainAproxInterpolated = interp_func(omega)
    
    relativeErrorY, relativeErrorX = relativeError(freq, omega, gainAccurate, gainAproxInterpolated)


    
    fig, axis = plt.subplots(1,2,figsize=(4.7747, 3.5))
    setFreqResponsePlot(axis[1],gainAccurate_dB, omega, title="Frequenzantwort")
    axis[1].plot(interp_func.x,toDecibel(interp_func.y))
    axis[1].scatter(freq,gainAprox_dB,color='red')

    average_error=sum(relativeErrorY)/len(relativeErrorY)
    axis[0].plot(relativeErrorX,relativeErrorY)
    axis[0].set_title("Relativer Fehler\n(Durchschnitt: {:.4f}%)".format(average_error))
    axis[0].set_ylabel('Relativer Fehler in %')
    axis[0].set_xlabel('Frequenz in $[rad/sec]$')
    axis[0].grid(True)
    plt.tight_layout()
    plt.show()
    print("Index- und G-Wert-Paare:", norm_value)


def plotFinal():
    G_stable = ctrl.TransferFunction(ctrl.tf([4], [1, 2, 4]))  # Beispiel Übertragungsfunktion
    validFreq, gainAprox = calculateAproxFreqResponse(G_stable)

    omega = np.logspace(-1, 2, 1001) 
    gainAccurate = abs(calculateFrequencyResponse(G_stable,omega))
    gainAccurate_dB = toDecibel(gainAccurate)
    gainAprox_dB = toDecibel(gainAprox)
    fig, axis = plt.subplots(1,2)
    setFreqResponsePlot(axis[1],gainAccurate_dB, omega, title="Frequency Response")
    plt.plot(validFreq, gainAprox_dB, 'bo', markersize=3)  # Scatter-Plot der Frequenzantwort
    plt.title('Frequency Response')
    plt.xlabel('Frequency [rad/s]')
    plt.ylabel('Gain')
    plt.grid(True)
    plt.show()

if __name__ == "__main__":
    #plotFinal()
    for i in range(5):
        plotExample()
        #großerTest()
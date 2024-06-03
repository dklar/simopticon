import control as ctrl

import numpy as np
import scipy 
import matplotlib.pyplot as plt

def setResponsePlot(ax,inputValues,outputvalues,timestep,twinx=False,title="Response of the system"):
    if twinx:
        max_yout_index = np.argmax(outputvalues)
        max_yout_value = np.max(outputvalues)
        ax.set_xlabel('Time ($sec$)')
        ax.set_ylabel('Output')
        ax.plot(timestep, outputvalues, label='Outputsignal', color='tab:blue')
        ax.scatter(timestep[max_yout_index], max_yout_value, color='red', marker='o', label='Maximum')
        ax.text(timestep[max_yout_index], max_yout_value, f'({timestep[max_yout_index]:.3f}, {max_yout_value:.3f})', verticalalignment='bottom', horizontalalignment='right')
        ax.set_title(title)
        ax2 = ax.twinx()
        ax2.set_ylabel('Input')
        ax2.plot(timestep, inputValues, linestyle='--', linewidth=0.8, label='Inputsignal', color='tab:red')

    else:
        ax.plot(timestep, inputValues, linestyle='--', linewidth=0.8,label='Inputsignal')
        ax.plot(timestep, outputvalues, label='Outputsignal')
        ax.scatter(timestep[np.argmax(outputvalues)], np.max(outputvalues), color='red', marker='o', label='Maximum')
        ax.text(timestep[np.argmax(outputvalues)], np.max(outputvalues), f'({timestep[np.argmax(outputvalues)]:.3f}, {np.max(outputvalues):.3f})', verticalalignment='bottom', horizontalalignment='right')
        ax.set_xlabel('Time')
        ax.set_ylabel('T_out')
        ax.set_title(title)
        ax.legend()

def setResponsePlot(ax,outputvalues,timestep,title="Response of the system"):
    ax.plot(timestep, outputvalues)
    #ax.scatter(timestep[np.argmax(outputvalues)], np.max(outputvalues), color='red')
    ax.set_xlabel('Zeit ($sek$)',fontsize=9)
    ax.set_title(title, fontsize=10)


def setMagnitudeSpecPlot(ax,values,time,title="Magnitude spectrum",x_label='Frequency ($Hz$)'):
    ax.plot(time, values)
    ax.set_title(title, fontsize=10)
    ax.set_xlabel(x_label,fontsize=9)
    ax.set_ylabel('Amplitude',fontsize=9)

def setFreqResponsePlot(ax, responseValue,freqValues,title="Frequenzantwort",x_label='Frequenz $[rad/sec]$'):
    ax.semilogx(freqValues, responseValue,label='Theoretischer Verlauf')
    ax.set_title(title)
    ax.set_xlabel(x_label)
    ax.set_ylabel('Amplitude $[dB]$')
    ax.grid(True)

#!/usr/bin/env python

from __future__ import print_function, division
import numpy as np
import pandas as pd
import re
import matplotlib
import matplotlib.pyplot as plt
# %config InlineBackend.figure_format = 'retina'
import seaborn #plotting lib, but just adding makes the matplotlob plots better
# import glob # use this for file IO later


# Experiments:
# -21 to +21 deg, with 1.4deg steps
# filename_1 ='WindShape_Calibration/sensors0012.LOG' # 9.3-9.6m/s case
# filename='WindShape_Calibration/sensors0014.LOG' # Apparently just during file transfer... not interesting
# filename_2 ='WindShape_Calibration/sensors0015.LOG' # 11.5 m/s case

filename_3 ='sensors0017.LOG' # speed change ?


def get_data(filename):
    data1 = pd.read_csv(filename , delimiter='\t', skiprows=1, header=None,skipfooter=4, engine='python')
    # data2 = data1['time'].str.split(':', expand=True)
    data2 = data1[0].str.split(':', expand=True)
    data1.drop(columns=[0], inplace=True)
    columns = ['baro_t','dp0_p','dp1_p','dp2_p','dp0_t','dp1_t',
               'dp2_t','acc_x','acc_y','acc_z','gyro_x','gyro_y','gyro_z','vcc','CoreTemp', 'xxx']
    data1.columns = columns
    data1.drop(columns=['xxx'], inplace=True)
    data2.columns = ['time','baro_p']
    data = pd.concat([data1,data2], axis=1 )
    return data

def plot_fit(data):
    st=44000;fn=54000
    window=100  # Change to 1 to see the noisy data, which is also fine!
    # I am only taking a rolling average to clean the data... For now we can also use direct measurements as well...
    p0 = data[st:fn].dp0_p.rolling(window=window).mean().values
    p1 = data[st:fn].dp1_p.rolling(window=window).mean().values
    p2 = data[st:fn].dp2_p.rolling(window=window).mean().values

    # This is the handmade calibration matrix, once I have real test results I can do this better...
    A = np.array([[1.15, 0.,  0., 6.0 , 6.0],
                  [0., 15.0,  0.,  0. , 0. ],
                  [0.,   0., 11.,  0. , 0. ]])

    p10= p1/p0
    p11= p10*p10
    p20= p2/p0
    p22= p20*p20

    vec = np.array([p0,p10,p20,p11,p22])

    # Need real labels, otherwise trying to estimate the bias doesnt make a lot of sense... But at least we will be ready for future...
    b = np.array([0.,0.,0.])


    X= A.dot(vec)

    # To get the airspeed out of dynamic pressure:
    rho=1.225
    q = X[0]+b[0]
    vel = np.sqrt(q/(0.5*rho))
    beta = X[1]+b[1]
    alpha = X[2]+b[2]

    def plot(signal,ylabel='Alpha'):
        fig=plt.figure(figsize=(18,5))
        plt.plot(signal);
        plt.ylabel(ylabel)
        plt.xlabel('Time [10-2 s]')
        plt.grid()

    plot(vel,'Speed [m/s]')
    plot(beta,'Beta [deg]')
    plot(alpha,'Alpha [deg]')
    plt.show()


def main():
    data = get_data(filename_3)
    plot_fit(data)


if __name__ == "__main__":
    main()


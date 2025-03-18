# traj_plot.py
#
# This file contains functions for plotting and generating trajectories
# of motor movement.
#
# Author: ME333, Jared Berry
# Date: 3/17/2025
#
import matplotlib.pyplot as plt
from statistics import mean

#
# Generate reference trajectory for given trajectory type
#
# Credit: Nick Marchuk
#
def gen_ref_trajectory(method):
    # Determine trajectory type
    if method == 'step':
        print('GENERATING STEP TRAJECTORY')
    elif method == 'cubic':
        print('GENERATING CUBIC TRAJECTORY')
    else:
        print('INVALID TRAJECTORY TYPE')
        return [-1]

    reflist = [0,0,1,90,2,45,3,45]
    
    # split: Convert string to list of substrings (separated by spaces)
    # map: Apply given function to each element of a list (float conversion)
    # list: Make sure final result is a list of floats
    refs_str = input('Enter times and angles, starting at t=0: ')
    reflist = list(map(float,refs_str.split())) # [t1, a1, ..., tn, an]

    # Check to see if the reflist is even and the odd values (times) are increasing
    if (len(reflist)%2!=0 or len(reflist) < 2 or reflist[0]!=0):
        print('Not a valid input: odd number of inputs or too short!\n')
        return [-1]
    dataok = 1
    for i in range(2, len(reflist), 2):
        if reflist[i] <= reflist[i-2]:
            dataok = 0
        if reflist[i-2] > 10 or reflist[i] > 10:
            print('Maximum trajectory time is 10 seconds!\n')
            return [-1]
    if dataok == 0:
        print('Not a valid input: time must increase!\n')
        return [-1]

    MOTOR_SERVO_RATE = 200 # The position control ISR is 200Hz
    dt = 1/MOTOR_SERVO_RATE # Time per control cycle

    numpos = int(len(reflist)/2)

    # Step trajectory
    if method == 'step':
        sample_list = [] # time
        angle_list = [] # position
        for i in range(0, len(reflist), 2):
            sample_list.append(reflist[i]*MOTOR_SERVO_RATE)
        for i in range(1, len(reflist), 2):
            angle_list.append(reflist[i])
        ref = [0]*int(sample_list[-1])
        last_sample = 0
        for i in range(1, numpos):
            for samp in range(int(last_sample), int(sample_list[i])):
                ref[samp] = angle_list[i-1]
            last_sample = sample_list[i]
        ref[samp] = angle_list[-1]
    
    # Cubic trajectory
    if method == 'cubic':
        ref = [] # store the output trajectory
        time_list = [] # time
        pos_list = [] # position
        for i in range(0, len(reflist), 2):
            time_list.append(reflist[i])
        for i in range(1, len(reflist), 2):
            pos_list.append(reflist[i])
        vel_list=[0]*numpos
        if numpos >= 3:
            for i in range(1, numpos-1):
                vel_list[i] = (pos_list[i+1]-pos_list[i-1])/(time_list[i+1]-time_list[i-1])
        #print(vel_list)
        refCtr = 0
        for i in range(0,numpos-1):
            timestart = time_list[i]
            timeend = time_list[i+1]
            deltaT = timeend-timestart
            posstart = pos_list[i]
            posend = pos_list[i+1]
            velstart = vel_list[i]
            velend = vel_list[i+1]
            a0 = posstart # calculate coeffs of traj pos = a0+a1*t+a2*t^2+a3*t^3
            a1 = velstart
            a2 = (3*posend - 3*posstart - 2*velstart*deltaT - velend*deltaT)/(deltaT*deltaT)
            a3 = (2*posstart + (velstart+velend)*deltaT - 2*posend)/(deltaT*deltaT*deltaT)
            while (refCtr)*dt < time_list[i+1]:
                tseg = (refCtr)*dt -time_list[i]
                ref.append(a0 + a1*tseg + a2*tseg*tseg + a3*tseg*tseg*tseg)
                refCtr = refCtr + 1

    return ref

def plot_trajectory(ser):
    """
    This function is called after menu command "o" (Execute trajectory).
    It reads the reference and actual trajectory arrays from the PIC,
    and plots them.

    :param ser: Access to serial port to interface with PIC32.
    """
    actual = []
    ref = []

    data_read = ser.read_until(b'\n',50)   # Read trajectory length
    data_text = str(data_read,'utf-8')
    traj_length = int(data_text)

    # Read current data arrays
    for sample in range(traj_length):
        # Read from PIC
        data_read = ser.read_until(b'\n',50)
        data_text = str(data_read,'utf-8')
        data = list(map(float,data_text.split()))   # [samp_num, actual, ref]

        # Create arrays for plotting
        actual.append(data[1])
        ref.append(data[2])

    # Score trajectory performance
    mean_list = []
    for i,j in zip(ref, actual):
        mean_list.append(abs(i-j))
    score = mean(mean_list)

    # Plot current data
    t = range(len(actual)) # Create time array from sample count
    plt.plot(t, actual, 'r*-', t, ref, 'b*-')
    plt.title(f'Score = {score}')
    plt.ylabel('Current (mA)')
    plt.xlabel('Sample')
    plt.show()

def plot_itest(ser):
    """
    This function is called after menu command "k" (Test current gains).
    It reads the reference and actual current data arrays from the PIC,
    and plots them.

    :param ser: Access to serial port to interface with PIC32.
    """
    sampnum = 0
    read_samples = 100
    curr_actual = []
    curr_ref = []

    # Read current data arrays
    while read_samples > 1:
        # Read from PIC
        data_read = ser.read_until(b'\n',50)
        data_text = str(data_read,'utf-8')
        data = list(map(float,data_text.split()))   # [samp_num, actual, ref]

        # Create arrays for plotting
        read_samples = int(data[0])
        curr_actual.append(data[1])
        curr_ref.append(data[2])
        sampnum = sampnum + 1

    # Plot current data
    t = range(len(curr_actual)) # Create time array from sample count
    plt.plot(t, curr_actual, 'r*-', t, curr_ref, 'b*-')
    plt.ylabel('Current (mA)')
    plt.xlabel('Sample')
    plt.show()
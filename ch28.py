# ch28.py
#
# This file contains the Python client code for the PIC32 motor driver interface
#
# Author: ME333, Jared Berry
# Date: 3/3/2025
#

# chapter 28 in python
# sudo apt-get install python3-pip
# python3 -m pip install pyserial
# sudo apt-get install python3-matplotlib

import serial
import matplotlib.pyplot as plt
ser = serial.Serial('com4',230400)

# Best Current Gains: Kp=0.16, Ki=0.06

def plot_itest():
    """
    This function is called after menu command "k" (Test current gains).
    It reads the reference and actual current data arrays from the PIC,
    and plots them.
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

def main():
    print('***ENTERING CLIENT***\n')
    print('\nOpening port: ')
    print(ser.name)
    has_quit = False

    # Run menu loop
    while not has_quit:
        print('\nPIC32 MOTOR DRIVER INTERFACE')

        # Display the menu options; this list will grow
        print(
            '\tb: Read current sensor (mA)'
            '\tc: Read encoder (counts)\n'
            '\td: Read encoder (degrees)'
            '\te: Reset encoder\n'
            '\tf: Set PWM (-100 to 100)'
            '\tg: Set current gains\n'
            '\th: Get current gains'
            '\t\tk: Test current gains\n'
            '\tp: Unpower the motor'
            '\t\tq: Quit\n'
            '\tr: Get mode'
        )

        # Read the user's choice
        selection = input('\nENTER COMMAND: ')
        selection_endline = selection+'\n'

        # Send the command to the PIC32
        ser.write(selection_endline.encode()); # .encode() turns the string into a char array

        # Take the appropriate action
        match selection:
            case 'b': # Read current (mA)
                a_str = ser.read_until(b'\n'); # Read current sensor
                a_flt = float(a_str) # turn it into a float
                print(f'Motor current (mA): {a_flt}\n') # print it to the screen
            case 'c': # Read encoder (counts)
                n_str = ser.read_until(b'\n'); # Read encoder count from PIC
                n_int = int(n_str) # turn it into an int
                print(f'Encoder count (counts): {n_int}\n') # print it to the screen
            case 'd': # Read encoder (degrees)
                n_str = ser.read_until(b'\n'); # Read encoder count from PIC
                n_int = int(n_str) # turn it into an int
                print(f'Encoder count (degrees): {n_int}\n') # print it to the screen
            case 'e': # Reset encoder
                pass
            case 'f': # Set PWM (-100 to 100)
                pwm_selection = input('ENTER PWM VALUE (-100 to 100): ')
                pwm_selection = pwm_selection+'\n'
                ser.write(pwm_selection.encode()); # Send command to PIC
            case 'g': # Set current gains
                kp_curr = input('ENTER PROPORTIONAL GAIN: ')
                ki_curr = input('ENTER INTEGRAL GAIN: ')
                kp_curr = kp_curr + '\n'
                ki_curr = ki_curr + '\n'
                ser.write(kp_curr.encode()); # Send gains to PIC
                ser.write(ki_curr.encode()); # Send gains to PIC
            case 'h': # Get current gains
                kp_curr_out = ser.read_until(b'\n'); 
                ki_curr_out = ser.read_until(b'\n'); 
                print(f'Kp={float(kp_curr_out)}, Ki={float(ki_curr_out)}\n')
            case 'k': # Test current gains
                plot_itest()
            case 'p': # Unpower the motor
                print('Powering down motor...\n')
            case 'q': # Quit client
                print('\n***EXITING CLIENT***')
                has_quit = True; # exit client
                ser.close() # close the port
            case 'r': # Get mode
                modes = {0:'IDLE', 1:'PWM', 2:'ITEST', 3:'HOLD', 4:'TRACK'}
                n_str = ser.read_until(b'\n'); # Read mode from PIC
                n_int = int(n_str)
                print(f'Mode: {modes[n_int]}\n')
            case _: # Default case, invalid selection
                print(f'Invalid Selection: {selection_endline}')

    return

if __name__ == "__main__":
    main()

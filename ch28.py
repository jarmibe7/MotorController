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
from traj_plot import plot_itest, gen_ref_trajectory, plot_trajectory
ser = serial.Serial('com4',230400)

# Best Current Gains: Kp=0.002, Ki=0.14, Kd=0
# Best Position Gains: Kp=100, Ki=0, Kd=4000

def main():
    print('***ENTERING CLIENT***\n')
    print('\nOpening port: ')
    print(ser.name)
    has_quit = False

    # Run menu loop
    while not has_quit:
        print('\nPIC32 MOTOR DRIVER INTERFACE')

        # Display the menu options this list will grow
        print(
            '\tb: Read current sensor (mA)'
            '\tc: Read encoder (counts)\n'
            '\td: Read encoder (degrees)'
            '\te: Reset encoder\n'
            '\tf: Set PWM (-100 to 100)'
            '\tg: Set current gains\n'
            '\th: Get current gains'
            '\t\ti: Set position gains\n'
            '\tj: Get position gains'
            '\t\tk: Test current gains\n'
            '\tl: Go to angle (deg)'
            '\t\tm: Load step trajectory\n'
            '\tn: Load cubic trajectory'
            '\to: Execute trajectory\n'
            '\tp: Unpower the motor'
            '\t\tq: Quit\n'
            '\tr: Get mode\n'
        )

        # Read the user's choice
        selection = input('\nENTER COMMAND: ')
        selection_endline = selection+'\n'

        # Send the command to the PIC32
        ser.write(selection_endline.encode()) # .encode() turns the string into a char array

        # Take the appropriate action
        match selection:
            case 'b': # Read current (mA)
                a_str = ser.read_until(b'\n') # Read current sensor
                a_flt = float(a_str) # turn it into a float
                print(f'Motor current (mA): {a_flt}\n') # print it to the screen
            case 'c': # Read encoder (counts)
                n_str = ser.read_until(b'\n') # Read encoder count from PIC
                n_int = int(n_str) # turn it into an int
                print(f'Encoder count (counts): {n_int}\n') # print it to the screen
            case 'd': # Read encoder (degrees)
                n_str = ser.read_until(b'\n') # Read encoder count from PIC
                n_int = int(n_str) # turn it into an int
                print(f'Encoder count (degrees): {n_int}\n') # print it to the screen
            case 'e': # Reset encoder
                print('Reseting motor encoder...\n')
            case 'f': # Set PWM (-100 to 100)
                pwm_selection = input('ENTER PWM VALUE (-100 to 100): ')
                pwm_selection = pwm_selection+'\n'
                ser.write(pwm_selection.encode()) # Send command to PIC
            case 'g': # Set current gains
                kp_curr = input('ENTER PROPORTIONAL GAIN: ')
                ki_curr = input('ENTER INTEGRAL GAIN: ')
                kd_curr = input('ENTER DERIVATIVE GAIN: ')
                kp_curr = kp_curr + '\n'
                ki_curr = ki_curr + '\n'
                kd_curr = kd_curr + '\n'
                ser.write(kp_curr.encode()) # Send gains to PIC
                ser.write(ki_curr.encode()) 
                ser.write(kd_curr.encode()) 
            case 'h': # Get current gains
                kp_curr_out = ser.read_until(b'\n') 
                ki_curr_out = ser.read_until(b'\n') 
                kd_curr_out = ser.read_until(b'\n') 
                print(f'Kp={float(kp_curr_out)}, Ki={float(ki_curr_out)}, Kd={float(kd_curr_out)}\n')
            case 'i': # Set position gains
                kp_pos = input('ENTER PROPORTIONAL GAIN: ')
                ki_pos = input('ENTER INTEGRAL GAIN: ')
                kd_pos = input('ENTER DERIVATIVE GAIN: ')
                kp_pos = kp_pos + '\n'
                ki_pos = ki_pos + '\n'
                kd_pos = kd_pos + '\n'
                ser.write(kp_pos.encode()) # Send gains to PIC
                ser.write(ki_pos.encode()) 
                ser.write(kd_pos.encode()) 
            case 'j': # Get position gains
                kp_pos_out = ser.read_until(b'\n') 
                ki_pos_out = ser.read_until(b'\n') 
                kd_pos_out = ser.read_until(b'\n') 
                print(f'Kp={float(kp_pos_out)}, Ki={float(ki_pos_out)}, Kd={float(kd_pos_out)}\n')
            case 'k': # Test current gains
                plot_itest(ser)
            case 'l': # Go to angle (deg)
                ang_selection = input('ENTER DESIRED ANGLE: ')
                ang_selection = ang_selection+'\n'
                ser.write(ang_selection.encode()) # Send command to PIC
            case 'm': # Load step trajectory
                ref = gen_ref_trajectory('step')
                t = range(len(ref))    # Display step trajectory
                plt.plot(t,ref,'r*-')
                plt.ylabel('Reference Motor Position')
                plt.xlabel('Sample Count')
                plt.show()
                ser.write((str(len(ref))+'\n').encode())    # Send trajectory to PIC
                for i in ref:
                    ser.write((str(i)+'\n').encode())   
            case 'n': # Load cubic trajectory
                ref = gen_ref_trajectory('cubic')
                t = range(len(ref))    # Display cubic trajectory
                plt.plot(t,ref,'r*-')
                plt.ylabel('Reference Motor Position')
                plt.xlabel('Sample Count')
                plt.show()
                ser.write((str(len(ref))+'\n').encode())    # Send trajectory to PIC
                for i in ref:
                    ser.write((str(i)+'\n').encode()) 
            case 'o': # Execute trajectory
                plot_trajectory(ser)
            case 'p': # Unpower the motor
                print('Powering down motor...\n')
            case 'q': # Quit client
                print('\n***EXITING CLIENT***')
                has_quit = True # Exit client
                ser.close() # Close the port
            case 'r': # Get mode
                modes = {0:'IDLE', 1:'PWM', 2:'ITEST', 3:'HOLD', 4:'TRACK'}
                n_str = ser.read_until(b'\n') # Read mode from PIC
                n_int = int(n_str)
                print(f'Mode: {modes[n_int]}\n')
            case _: # Default case, invalid selection
                print(f'Invalid Selection: {selection_endline}')

    return

if __name__ == "__main__":
    main()

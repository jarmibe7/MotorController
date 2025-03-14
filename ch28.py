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
ser = serial.Serial('com4',230400)

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
            '\tp: Unpower the motor\n'
            '\tq: Quit'
            '\t\t\t\tr: Get mode\n'
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
            case 'p':
                pass
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

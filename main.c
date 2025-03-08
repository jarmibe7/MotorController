// main.c
//
// This file contains the main setup/run code for the ME333 final project.
//
// Author: ME333, Jared Berry
// Date: 3/3/2025
//
// Command to check Pico: & "C:\Program Files\PuTTY\putty.exe" -serial COM6 -sercfg 230400,8,n,1,N

#include "nu32dip.h"
#include "encoder.h"  
// include other header files here

#define BUF_SIZE 200

int main() 
{
    char buffer[BUF_SIZE];
    NU32DIP_Startup(); // Cache on, min flash wait, interrupts on, LED/button init, UART init
    NU32DIP_YELLOW = 1;  // Turn off the LEDs
    NU32DIP_GREEN = 1;

    UART2_Startup(); // Initialize UART2

    __builtin_disable_interrupts();
    // In future, initialize modules or peripherals here
    __builtin_enable_interrupts();

    while(1)
    {
        NU32DIP_ReadUART1(buffer,BUF_SIZE); // We expect the next character to be a menu command
        NU32DIP_GREEN = 1;                   // Clear the error LED

        // Check for menu command
        switch (buffer[0]) {
            case 'c':                      // c: Read encoder value (counts)
            {
                WriteUART2("a");
                while(!get_encoder_flag()) {
                    ;
                }
                set_encoder_flag(0);
                char m[50];
                int p = get_encoder_count();
                sprintf(m,"%d\r\n",p);
                NU32DIP_WriteUART1(m);
                break;
            }
            case 'd':                      // d: Read encoder value (degrees)
            {
                WriteUART2("a");
                while(!get_encoder_flag()) {
                    ;
                }
                set_encoder_flag(0);
                char m[50];
                int p = get_encoder_count();
                int degrees = (int) ((float) p / 3.7111);
                sprintf(m,"%d\r\n",degrees);
                NU32DIP_WriteUART1(m);
                break;
            }
            case 'e':                      // e: Reset encoder value
            {
                WriteUART2("b");
                break;
            }
            case 'q':
            {
                // Handle q for quit. Later you may want to return to IDLE mode here. 
                break;
            }
            case 'r':                       // r: Get mode
            {

            }
            default:
            {
                NU32DIP_GREEN = 0;  // Turn on LED2 to indicate an error
                break;
            }
        }
    }

    return 0;
}

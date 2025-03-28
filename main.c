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
#include "utilities.h"
#include "current_control.h"
#include "ina219.h"
#include "position_control.h"





int main() 
{
    char buffer[BUF_SIZE];
    NU32DIP_Startup(); // Cache on, min flash wait, interrupts on, LED/button init, UART init
    NU32DIP_YELLOW = 1;  // Turn off the LEDs
    NU32DIP_GREEN = 1;

    __builtin_disable_interrupts();

    make_waveform();
    set_mode(IDLE); // Set initial mode to IDLE
    UART2_Startup(); // Initialize UART2
    Current_Control_Startup(); // Initialize current controller and PWM
    Position_Control_Startup(); // Initialize position controller
    INA219_Startup();

    __builtin_enable_interrupts();
    while(1)
    {
        NU32DIP_ReadUART1(buffer,BUF_SIZE); // We expect the next character to be a menu command
        NU32DIP_GREEN = 1;                   // Clear the error LED

        // Check for menu command
        switch (buffer[0]) {
            case 'b':                      // b: Read current sensor (mA)
            {
                float current = INA219_read_current();
                char m[50];
                sprintf(m,"%f\r\n",current);
                NU32DIP_WriteUART1(m);
                break;
            }
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
                int degrees = read_encoder_deg();
                char m[50];
                sprintf(m,"%d\r\n",degrees);
                NU32DIP_WriteUART1(m);
                break;
            }
            case 'e':                      // e: Reset encoder value
            {
                WriteUART2("b");
                break;
            }
            case 'f':                      // f: Set PWM (-100 to 100)
            {
                char pwmBuffer[BUF_SIZE];
                NU32DIP_ReadUART1(pwmBuffer,BUF_SIZE); // Read PWM value
                int pwm;
                int valid = sscanf(pwmBuffer, "%d", &pwm);
                if (valid != 1 || pwm < -100 || pwm > 100) {
                    NU32DIP_GREEN = 0;  // Error
                    break;
                }
                set_mode(PWM);  // Set mode to PWM
                set_pwm_dc(pwm);
                break;
            }
            case 'g':                       // g: Set current gains
            {
                float kp_in=0, ki_in=0, kd_in=0;
                char inp[BUF_SIZE];
                NU32DIP_ReadUART1(inp,BUF_SIZE); // Read Kp value
                int kp_val = sscanf(inp, "%f", &kp_in);
                NU32DIP_ReadUART1(inp,BUF_SIZE); // Read Ki value
                int ki_val = sscanf(inp, "%f", &ki_in);
                NU32DIP_ReadUART1(inp,BUF_SIZE); // Read Kd value
                int kd_val = sscanf(inp, "%f", &kd_in);
                if(kp_val != 1 || ki_val != 1 || kd_val != 1) {
                    NU32DIP_GREEN = 0;  // Error
                    break;
                }   
                set_curr_kp(kp_in);   // Call setter functions
                set_curr_ki(ki_in);
                set_curr_kd(kd_in);
                break;

            }
            case 'h':                       // h: Get current gains
            {
                float kp = get_curr_kp(); // Call getter functions
                float ki = get_curr_ki();
                float kd = get_curr_kd();
                char m[50];
                sprintf(m,"%f\r\n%f\r\n%f\r\n",kp,ki,kd);
                NU32DIP_WriteUART1(m);
                break;
            }
            case 'i':                       // i: Set position gains
            {
                float kp_in=0, ki_in=0, kd_in=0;
                char inp[BUF_SIZE];
                NU32DIP_ReadUART1(inp,BUF_SIZE); // Read Kp value
                int kp_val = sscanf(inp, "%f", &kp_in);
                NU32DIP_ReadUART1(inp,BUF_SIZE); // Read Ki value
                int ki_val = sscanf(inp, "%f", &ki_in);
                NU32DIP_ReadUART1(inp,BUF_SIZE); // Read Kd value
                int kd_val = sscanf(inp, "%f", &kd_in);
                if(kp_val != 1 || ki_val != 1 || kd_val != 1) {
                    NU32DIP_GREEN = 0;  // Error
                    break;
                }   
                set_pos_kp(kp_in);   // Call setter functions
                set_pos_ki(ki_in);
                set_pos_kd(kd_in);
                break;
            }
            case 'j':                       // j: Get position gains
            {
                float kp = get_pos_kp(); // Call getter functions
                float ki = get_pos_ki();
                float kd = get_pos_kd();
                char m[50];
                sprintf(m,"%f\r\n%f\r\n%f\r\n",kp,ki,kd);
                NU32DIP_WriteUART1(m);
                break;
            }
            case 'k':                       // k: Test current gains
            {
                set_mode(ITEST);    // Test current and send plot data
                send_curr_data();
                break;
            }
            case 'l':                       // l: Go to angle (deg)
            {
                char angBuffer[BUF_SIZE];
                NU32DIP_ReadUART1(angBuffer,BUF_SIZE); // Read PWM value
                int ang;
                int valid = sscanf(angBuffer, "%d", &ang);
                if (valid != 1) {
                    NU32DIP_GREEN = 0;  // Error
                    break;
                }
                set_mode(HOLD);  // Set mode to PWM
                set_angle(ang);
                break;
            }
            case 'm':                       // m: Load step trajectory
            {
                read_traj();
                break;
            }
            case 'n':                       // n: Load cubic trajectory
            {
                read_traj();
                break;
            }
            case 'o':                       // o: Exectue trajectory
            {
                set_mode(TRACK);
                while (get_mode() == TRACK) {
                    ;   // Wait until trajectory is done being followed
                }
                send_pos_data();
                break;
            }
            case 'p':                       // p: Unpower the motor
            {
                set_mode(IDLE);
                break;
            }
            case 'q':                       // q: Quit
            {
                set_mode(IDLE);  // Set mode to IDLE before menu exit
                break;
            }
            case 'r':                       // r: Get mode
            {
                char m[50];
                int curr_mode = (int) get_mode();
                sprintf(m,"%d\r\n",curr_mode);
                NU32DIP_WriteUART1(m);
                break;
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

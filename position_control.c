// position_control.c
//
// This file contains code for PID control of motor position.
//
// Author: Jared Berry
// Date: 03/16/2025
//
#include "NU32DIP.h"
#include "position_control.h"
#include "current_control.h"
#include "utilities.h"
#include "encoder.h"

static volatile int Angle = 0;              // Desired motor position

static volatile float Kp=0, Ki=0, Kd=0;     // Control gains
static volatile float Eint = 0;               // Integral of error

static volatile float REFarray[TRAJ_NUMSAMPS]; // Trajectory reference
static volatile float TRAJarray[TRAJ_NUMSAMPS]; // Actual followed trajectory
static volatile int TrajLength = 0;             // Actual length of trajectory

void __ISR(_TIMER_4_VECTOR, IPL5SOFT) PositionController(void) {
    LATBINV = 0x1000; // Debug output
    static int curr_ang = 0;
    static int error = 0;
    static float u = 0;
    static int prev_error = 0;
    static int traj_index = 0;

    switch (get_mode()) {
        case HOLD:
        {
            curr_ang = read_encoder_deg();  // Read encoder
            error = Angle - curr_ang;   // Calculate angle error
            Eint += error;  // Update integral error
            u = Kp*error + Ki*Eint + Kd*(error - prev_error); // Calculate control signal

            set_torque(u);

            if (Eint > 100.0f) {    // Prevent integrator wind up
                Eint = 100.0f; 
            } else if (Eint < -100.0f) {
                Eint = -100.0f;
            }

            prev_error = error;     // Update previous error for derivative term
            break;
        }
        case TRACK:
        {
            curr_ang = read_encoder_deg();  // Read encoder
            TRAJarray[traj_index] = (float) curr_ang;   // Store actual angle
            set_angle((int) REFarray[traj_index]); // Set ref angle
            error = Angle - curr_ang;   // Calculate angle error
            Eint += error;  // Update integral error
            u = Kp*error + Ki*Eint + Kd*(error - prev_error); // Calculate control signal

            set_torque(u);

            if (Eint > 100.0f) {    // Prevent integrator wind up
                Eint = 100.0f; 
            } else if (Eint < -100.0f) {
                Eint = -100.0f;
            }

            prev_error = error;     // Update previous error for derivative term

            traj_index++;
            if(traj_index == TrajLength) {   // If done with trajectory, hold position
                traj_index = 0;
                set_mode(HOLD);
            }
            break;
        }
    }
    IFS0bits.T4IF = 0;  // Clear interrupt flag
}

//
// Setup Timer4 for position control ISR
//
void Position_Control_Startup(void) {
    TRISBbits.TRISB12 = 0; // DEBUG
    //
    // Timer4 settings (Position Control ISR)
    //
    T4CONbits.TCKPS = 2; // Timer4 prescaler N=4
    PR4 = 60009; // period = (PR4+1) * N * 20.83 ns = 5 ms, 200 Hz
    TMR4 = 0; // initialize TMR4 count

    // Initialize Timer4 ISR
    IPC4bits.T4IP = 5;            // interrupt priority 5
    IPC4bits.T4IS = 0;            // subpriority 0
    IFS0bits.T4IF = 0;            // clear the int flag
    IEC0bits.T4IE = 1;            // enable Timer4

    T4CONbits.ON = 1; // turn on Timer4 (Position Controller)
    return;
}

//
// Read trajectory from client
//
void read_traj() {
    char trajBuffer[BUF_SIZE];
    NU32DIP_ReadUART1(trajBuffer,BUF_SIZE); // Trajectory length
    int valid = sscanf(trajBuffer, "%d", &TrajLength);
    if (valid != 1) {
        NU32DIP_GREEN = 0;  // Error
        return;
    }

    // Store trajectory in array
    float sample;
    for(int i=0; i < TrajLength; i++) {
        NU32DIP_ReadUART1(trajBuffer,BUF_SIZE); // Read next sample point
        valid = sscanf(trajBuffer, "%f", &sample);
        if (valid != 1) {
            NU32DIP_GREEN = 0;  // Error
            return;
        }
        REFarray[i] = sample;
    }
}

//
// Send plot data to Python
//
void send_pos_data() {
    char message[50];
    sprintf(message, "%d\r\n", TrajLength); // Send data length
    NU32DIP_WriteUART1(message);

    for (int i=0; i<TrajLength; i++) {  // Send plot data
        sprintf(message, "%d %f %f\r\n", i, TRAJarray[i], REFarray[i]);
        NU32DIP_WriteUART1(message);
    }
}

//
// Setters and getters
//
void set_angle(int ang) { Angle = ang; }

//
// Setters for position control gains
//
void set_pos_kp(float kp) { Kp = kp; }
void set_pos_ki(float ki) { Ki = ki; }
void set_pos_kd(float kd) { Kd = kd; }

//
// Getters for position control gains
//
float get_pos_kp() { return Kp; }
float get_pos_ki() { return Ki; }
float get_pos_kd() { return Kd; }
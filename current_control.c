// current_control.c
//
// This file contains code for PI control and PWM for
// controlling the current through the motor.
//
// Author: Jared Berry
// Date: 03/08/2025
//

#include "NU32DIP.h"
#include "current_control.h"
#include "ina219.h"
#include "utilities.h"

static volatile int PwmDC = 0; // PWM duty cycle
static volatile int PwmDirection = 0;  // PWM direction (0 for forward, 1 for reverse)

static volatile float Kp=0, Ki=0, Kd=0;       // Control gains
static volatile float Eint = 0;               // Integral of error

static volatile float Torque = 0;               // Desired torque from position controller

static volatile float ITEST_Waveform[ITEST_NUMSAMPS];     // Waveform
static volatile float CURRarray[ITEST_NUMSAMPS];      // Measured values to plot (from current sensor)
static volatile float REFarray[ITEST_NUMSAMPS];      // Reference values to plot (ref current);

// char m[50];
// sprintf(m,"%d\r\n",OC1RS);
// NU32DIP_WriteUART1(m);

void __ISR(_TIMER_3_VECTOR, IPL6SOFT) CurrentController(void) {
    // OC1RS = 600; // Set duty cycle to 25%
    // LATBINV = 0x800;    // Toggle RB11
    static int itest_samples = 0;
    static float current = 0;
    static float error = 0;
    static float prev_error = 0;
    static float u = 0;           // Control signal

    switch (get_mode()) {
        case IDLE:
        {
            OC1RS = 0; // Set duty cycle to 0%
            break;
        }
        case PWM:
        {
            OC1RS = PwmDC; // Set duty cycle to duty cycle
            LATBbits.LATB11 = PwmDirection; // Set motor direction
            break;
        }
        case ITEST:
        {
            itest_samples++;

            current = INA219_read_current();  // Read current sensor
            error = (float) ITEST_Waveform[itest_samples] - current;  // Calculate error
            Eint += error;  // Update integral of error
            u = Kp*error + Ki*Eint + Kd*(error - prev_error);  // Calculate control signal

            if (Eint > 150.0f) {    // Prevent integrator wind up
                Eint = 150.0f; 
            } else if (Eint < -150.0f) {
                Eint = -150.0f;
            }
            
            // char m[50]; // Debug output
            // sprintf(m,"%f\r\n",ITEST_Waveform[itest_samples]);
            // NU32DIP_WriteUART1(m);

            set_pwm_dc(u);  // Set the duty cycle and direction bit
            OC1RS = PwmDC;
            LATBbits.LATB11 = PwmDirection;

            // Save points to plot later
            CURRarray[itest_samples] = current;
            REFarray[itest_samples] = ITEST_Waveform[itest_samples];

            // If we are done testing set mode to IDLE
            if (itest_samples >= ITEST_NUMSAMPS - 1) {
                set_mode(IDLE);
                itest_samples = 0;
                Eint = 0;
            }
            break;
        }
        case HOLD:
        {
            current = INA219_read_current();   // Calculate necessary current for desired torque
            error = (float) Torque - current;  // Calculate error
            Eint += error;  // Update integral of error
            u = Kp*error + Ki*Eint + Kd*(error - prev_error);  // Calculate control signal

            // char m[50]; // Debug output
            // sprintf(m,"%f\r\n",Torque);
            // NU32DIP_WriteUART1(m);

            if (Eint > 150.0f) {    // Prevent integrator wind up
                Eint = 150.0f; 
            } else if (Eint < -150.0f) {
                Eint = -150.0f;
            }

            set_pwm_dc(u);  // Set the duty cycle and direction bit
            OC1RS = PwmDC;
            LATBbits.LATB11 = PwmDirection;
        }
        default:
        {
            NU32DIP_GREEN = 0;  // Turn on green LED to indicate an error
            break;
        }
    }


    IFS0bits.T3IF = 0;  // Clear interrupt flag
}

//
// Initialize SFRs for 5kHz current control ISR and
// 20 kHz PWM signal
//
void Current_Control_Startup(void) {
    __builtin_disable_interrupts();
    pwm_setup();
    current_controller_setup();
    TRISBbits.TRISB11 = 0; // Set RB11 as output for motor direction
    T2CONbits.ON = 1; // turn on Timer2 (PWM)
    OC1CONbits.ON = 1; // turn on OC1
    T3CONbits.ON = 1; // turn on Timer3 (Current Controller)
    __builtin_enable_interrupts();
}

//
// Setup SFRs for 20kHz PWM using Timer2 and OC1
//
static void pwm_setup(void) {
    //
    // Timer2 settings (PWM)
    //
    T2CONbits.TCKPS = 0; // Timer2 prescaler N=1
    PR2 = 2399; // period = (PR2+1) * N * 20.83 ns = 50 us, 20 kHz
    TMR2 = 0; // initialize TMR2 count

    // Activate OC1n pin RPB7R
    // RPB7Rbits.RPB7R = 0b0101;
    RPB7Rbits.RPB7R = 0b0101;

    // OC1 settings
    OC1CONbits.OCM = 0b110; // PWM mode without fault pin
    OC1CONbits.OCTSEL = 0; // Use Timer2
    OC1RS = 600; // duty cycle = OC1RS/(PR2+1) = 25%
    OC1R = 600; // initialize before turning OC1 on; afterward it is read-only
    
    return;
}

//
// Setup SFRs for 5kHz current control using Timer3
//
static void current_controller_setup(void) {
    //
    // Timer3 settings (Current Control ISR)
    //
    T3CONbits.TCKPS = 0; // Timer3 prescaler N=1
    PR3 = 9599; // period = (PR3+1) * N * 20.83 ns = 0.2 ms, 5 kHz
    TMR3 = 0; // initialize TMR3 count

    // Initialize Timer3 ISR
    IPC3bits.T3IP = 6;            // interrupt priority 6
    IPC3bits.T3IS = 0;            // subpriority 0
    IFS0bits.T3IF = 0;            // clear the int flag
    IEC0bits.T3IE = 1;            // enable Timer3

    return;
}

//
// Function to make reference current signal
//
void make_waveform() {
    // Waveform represents the desired motor current
    int i = 0;  // square wave center and amplitude
    float A = 200.0f;
    for (i = 0; i < ITEST_NUMSAMPS; ++i) {
        if ( (i < 25) || (i >= 50 && i < 75)) {
            ITEST_Waveform[i] = A;
        } 
        else {
            ITEST_Waveform[i] = -A;
        }
    }
}

//
// Send plot data to Python
//
void send_plot_data() {
    char message[50];
    for (int i=0; i<ITEST_NUMSAMPS; i++) {
        sprintf(message, "%d %f %f\r\n", ITEST_NUMSAMPS-i, CURRarray[i], REFarray[i]);
        NU32DIP_WriteUART1(message);
    }
}

// ---------------------------------------
//          Setters and Getters
// ---------------------------------------

//
// Setter for PWM duty cycle
//
void set_pwm_dc(int dc) {
    if (dc > 100.0f) {      // Make sure DC is in bounds
        dc = 100.0f;
    }
    else if (dc < -100.0f) {
        dc = -100.0f;
    }

    PwmDC = abs(dc);       // Convert DC input to percentage of PR2
    PwmDC = (int) (((float) PwmDC / 100.0f) * 2400);

    if (dc < 0) {           // Get direction bit
        PwmDirection = 1;
    } else {
        PwmDirection = 0;
    }
}

//
// Setter for torque
//
float set_torque(float tor) { Torque = tor; }

//
// Setters for current control gains
//
void set_curr_kp(float kp) { Kp = kp; }
void set_curr_ki(float ki) { Ki = ki; }
void set_curr_kd(float kd) { Kd = kd; }

//
// Getters for current control gains
//
float get_curr_kp() { return Kp; }
float get_curr_ki() { return Ki; }
float get_curr_kd() { return Kd; }


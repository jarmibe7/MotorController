#include "NU32DIP.h"
#include "current_control.h"
#include "utilities.h"

static volatile int pwm_dc = 0; // PWM duty cycle
static volatile int pwm_direction = 0;  // PWM direction (0 for forward, 1 for reverse)

void set_pwm_dc(int dc);

void __ISR(_TIMER_3_VECTOR, IPL6SOFT) CurrentController(void) {
    // OC1RS = 600; // Set duty cycle to 25%
    // LATBINV = 0x800;    // Toggle RB11

    switch (get_mode()) {
        case IDLE:
        {
            OC1RS = 0; // Set duty cycle to 0%
            break;
        }
        case PWM:
        {
            OC1RS = (int) (((float) pwm_dc / 100.0f) * 2400); // Set duty cycle to duty cycle
            // char m[50];
            // sprintf(m,"%d\r\n",OC1RS);
            // NU32DIP_WriteUART1(m);
            LATBbits.LATB11 = pwm_direction; // Set motor direction
            break;
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
    OC1RS = 600; // duty cycle = OC1RS/(PR2+1) = 75%
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
    IPC3bits.T3IP = 6;            // interrupt priority 5
    IPC3bits.T3IS = 0;            // subpriority 0
    IFS0bits.T3IF = 0;            // clear the int flag
    IEC0bits.T3IE = 1;            // enable Timer3

    return;
}

void set_pwm_dc(int dc) {
    pwm_dc = abs(dc);
    if (dc < 0) {
        pwm_direction = 1;
    } else {
        pwm_direction = 0;
    }
}
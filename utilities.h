#ifndef UTILITIES__H__
#define UTILITIES__H__

#include "nu32dip.h"

typedef enum {
    IDLE,
    PWM,
    ITEST,
    HOLD,
    TRACK
} Mode;

Mode get_mode();
void set_mode(Mode m);

#define ITEST_NUMSAMPS 100       // Number of points in ITEST reference
// TODO: Waveform container arrays

#endif // UTILITIES__H__
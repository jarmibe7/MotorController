#ifndef UTILITIES__H__
#define UTILITIES__H__

#include "nu32dip.h"

enum Mode {
    IDLE,
    PWM,
    ITEST,
    HOLD,
    TRACK
}

#define NUMSAMPS 1000       // Number of points in waveform
// TODO: Waveform container arrays

#endif // UTILITIES__H__
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
#define TRAJ_NUMSAMPS 2000       // Max numer of samples for referenc trajectories
#define BUF_SIZE 200             // Size for reading  inputs

#endif // UTILITIES__H__
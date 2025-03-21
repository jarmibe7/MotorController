// utilities.c
//
// Contain functions for controlling the mode of the motor controller.
//
// Author: Jared Berry
// Date: 03/08/2025
//
#include "utilities.h"



static volatile Mode mode;

Mode get_mode() {
    return mode;
}

void set_mode(Mode m) {
    mode = m;
}
#ifndef PPMATH_MOTOR_H
#define PPMATH_MOTOR_H

#include "ppmath.h"
#include "constants.h"

// Base calculation for the corresponding V per RPM, 
// taking the VFD VSI Voltage level for the max. operating frequency
// into account. The max. operating frequency is set in the VFD !
int vfd_calc_vsi(int rpm);

// safe VSI version for grinder
int vfd_calc_vsi_grinder(int rpm);

// safe VSI version for shredder
int vfd_calc_vsi_shredder(int rpm);

#endif
#include "ppmath_motor.h"
int vfd_calc_vsi(int rpm)
{
    return ((rpm * VFD_VSI_MAX_V_SPEED) / MOTOR_FREQ_CMAX);
}
int vfd_calc_vsi_grinder(int rpm){
    return VFD_VSI_SCALE *
        vfd_calc_vsi( 
            clamp<int>(rpm * RPM_GRINDER_SHREDDER_SCALE, RPM_GRINDING_MIN, RPM_GRINDING_MAX)
        );
}
int vfd_calc_vsi_shredder(int rpm){
    return VFD_VSI_SCALE *
        vfd_calc_vsi( 
            clamp<int>(rpm, RPM_SHREDDERING_MIN, RPM_SHREDDERING_MAX)
        );
}
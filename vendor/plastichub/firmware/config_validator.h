#ifndef CONFIG_VALIDATOR
#define CONFIG_VALIDATOR

#include "common/macros.h"

#if defined(ENABLE_AUTO_REVERSE_PIN) && defined(HAS_AUTO_REVERSE_MODE)
    #error "You can not have auto-revese toggle and a 3 pos mode switch"
#endif

#if DEBUG == true && USE_FIRMATA && DEBUG_BAUD_RATE !=FIRMATA_BAUD_RATE
    #error "Debug baud rate must match firmata baud rate"
#endif


#if defined(USE_UNO) && defined(IR_SPEED)
    #if IR_PIN !=2 && IR_PIN !=3
        #error "For speed computation on the UNO, you need to use pins D2 or D3!"
    #endif
#endif

#endif
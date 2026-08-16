# Omron VFD Registers

### State

The VFD's state is written into address 4 (defined in enums.h#MB_R_VFD_STATE)

**Values**

``` c
#define OMRON_STATE_ACCELERATING 4
#define OMRON_STATE_DECELERATING 2
#define OMRON_STATE_RUNNING 3
#define OMRON_STATE_STOPPED 1
```

### Status

The VFD's status is written into address 3 (defined in enums.h#MB_R_VFD_STATUS)

**Values**

``` c
#define OMRON_STATUS_STOPPED 2
#define OMRON_STATUS_RUNNING 0
```

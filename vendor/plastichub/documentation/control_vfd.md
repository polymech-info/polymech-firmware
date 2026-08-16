# VFD

## Start & Stop

Starts or stops the VFD, be aware that a target frequency has to be set as well

- **Address** : 5
- **Function** : 6 (WRITE_REGISTER)
- **Values** :
  - **On** : 1
  - **Off** : 2

**TCP Sequence for Start** 

```c
d2 8d 00 00 00 06 01 06 00 05 00 01
                   +  +  +    +
                   |  |  |    |
                   |  |  |    +----> Value (2 bytes) = 00 01
                   |  |  |
                   |  |  +----> Address (2 bytes) = 05 or 0x0005
                   |  |
                   |  +--> Function Code (Always 6)
                   |
                   +--> Slave - ID (Always 1)
```

**TCP Sequence for Stop** 

```c
d2 8d 00 00 00 06 01 06 00 05 00 02
                   +  +  +    +
                   |  |  |    |
                   |  |  |    +----> Value (2 bytes) = 00 02
                   |  |  |
                   |  |  +----> Address (2 bytes) = 05 or 0x0005
                   |  |
                   |  +--> Function Code (Always 6)
                   |
                   +--> Slave - ID (Always 1)
```

## Frequency

Sets the target frequency

- **Address** : 6
- **Function** : 6 (WRITE_REGISTER)
- **Values** : 1 - 50

**TCP Sequence for setting target frequency to 50 Hz** 

Remark : Please respect the max. main frequency setting on the inverter (settings)

```c
d2 8d 00 00 00 06 01 06 00 05 00 32
                   +  +  +    +
                   |  |  |    |
                   |  |  |    +----> Value (2 bytes) = 00 32 (for 50Hz)
                   |  |  |
                   |  |  +----> Address (2 bytes) = 06 or 0x0006
                   |  |
                   |  +--> Function Code (Always 6)
                   |
                   +--> Slave - ID (Always 1)
```

## Direction

Sets the rotation

- **Address** : 7
- **Function** : 6 (WRITE_REGISTER)
- **Values** :
  - **Forward** : 1
  - **Reverse** : 2
  - **Stop** : 3

**TCP Sequence for setting direction : Forward** 

```c
d2 8d 00 00 00 06 01 06 00 07 00 01
                   +  +  +    +
                   |  |  |    |
                   |  |  |    +----> Value (2 bytes) = 00 01
                   |  |  |
                   |  |  +----> Address (2 bytes) = 07 or 0x0006
                   |  |
                   |  +--> Function Code (Always 6)
                   |
                   +--> Slave - ID (Always 1)
```

**TCP Sequence for setting direction : Reverse** 

```c
d2 8d 00 00 00 06 01 06 00 07 00 02
                   +  +  +    +
                   |  |  |    |
                   |  |  |    +----> Value (2 bytes) = 00 02
                   |  |  |
                   |  |  +----> Address (2 bytes) = 07 or 0x0006
                   |  |
                   |  +--> Function Code (Always 6)
                   |
                   +--> Slave - ID (Always 1)
```

# PID Controllers

The PID controller target temperatures can be set as follows:

- PID 1 : Address 17
- PID 2 : Address 18
- PID 3 : Address 19

## Set Target temperature on PID 1 to 100Degc

- **Address** : 17 (```0x11```)
- **Function** : 6 (WRITE_REGISTER)
- **Values** : 0 - Max Temperature (300)

**TCP Sequence** 

```c
d2 8d 00 00 00 06 01 06 00 11 00 64
                   +  +  +    +
                   |  |  |    |
                   |  |  |    +----> Value (2 bytes) = 00 64 (100 Degc)
                   |  |  |
                   |  |  +----> Address (2 bytes) = 17 or 0x0011
                   |  |
                   |  +--> Function Code (Always 6)
                   |
                   +--> Slave - ID (Always 1)
```

## Set Target temperature on PID 2to 100Degc

- **Address** : 18 (```0x12```)
- **Function** : 6 (WRITE_REGISTER)
- **Values** : 0 - Max Temperature (300)

**TCP Sequence** 

```c
d2 8d 00 00 00 06 01 06 00 12 00 64
                   +  +  +    +
                   |  |  |    |
                   |  |  |    +----> Value (2 bytes) = 00 64 (100 Degc)
                   |  |  |
                   |  |  +----> Address (2 bytes) = 18 or 0x0012
                   |  |
                   |  +--> Function Code (Always 6)
                   |
                   +--> Slave - ID (Always 1)
```


## Set Target temperature on PID 3 to 100Degc

- **Address** : 18 (```0x13```)
- **Function** : 6 (WRITE_REGISTER)
- **Values** : 0 - Max Temperature (300)

**TCP Sequence** 

```c
d2 8d 00 00 00 06 01 06 00 13 00 64
                   +  +  +    +
                   |  |  |    |
                   |  |  |    +----> Value (2 bytes) = 00 64 (100 Degc)
                   |  |  |
                   |  |  +----> Address (2 bytes) = 18 or 0x0013
                   |  |
                   |  +--> Function Code (Always 6)
                   |
                   +--> Slave - ID (Always 1)
```

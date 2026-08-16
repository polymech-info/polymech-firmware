#ifndef NET_COMMONS_H
#define NET_COMMONS_H

enum class E_PRIORITY : uint8_t 
{
    E_PRIORITY_HIGHEST = 100,
    E_PRIORITY_MEDIUM = 60,
    E_PRIORITY_LOW = 40,
    E_PRIORITY_LOWEST = 20
};

enum E_NVC_TCP_OFFSET {
    E_NVC_ENABLED = 0, // Default coil for enabling/disabling the component
    E_NVC_USER = 1     // Start of user-defined registers
};

#endif
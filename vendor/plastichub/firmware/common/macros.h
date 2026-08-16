#ifndef MACROS_H
#define MACROS_H

#include "../types.h"

// Macros for adding
#define INC_0 1
#define INC_1 2
#define INC_2 3
#define INC_3 4
#define INC_4 5
#define INC_5 6
#define INC_6 7
#define INC_7 8
#define INC_8 9
#define INCREMENT_(n) INC_ ##n
#define INCREMENT(n) INCREMENT_(n)

// Macros for subtracting
#define DEC_1 0
#define DEC_2 1
#define DEC_3 2
#define DEC_4 3
#define DEC_5 4
#define DEC_6 5
#define DEC_7 6
#define DEC_8 7
#define DEC_9 8
#define DECREMENT_(n) DEC_ ##n
#define DECREMENT(n) DECREMENT_(n)

// compiler - & C quirks
#define FORCE_INLINE __attribute__((always_inline)) inline
#define _UNUSED      __attribute__((unused))

// fallback noop
#define NOOP do{} while(0)

//Option testing
#define _CAT(a, ...) a ## __VA_ARGS__
#define SWITCH_ENABLED_      1
#define ENABLED(b) _CAT(SWITCH_ENABLED_, b)

// time
#define PENDING(NOW,SOON) ((long)(NOW-(SOON))<0)
#define ELAPSED(NOW,SOON) (!PENDING(NOW,SOON))

#define MMM_TO_MMS(MM_M) ((MM_M)/60.0f)
#define MMS_TO_MMM(MM_S) ((MM_S)*60.0f)
#define HOUR_MS ((millis_t)1000 * (millis_t)(60 * 60))
#define MIN_MS ((millis_t)1000 * (millis_t)(60))
#define SECS ((millis_t)1000)

// bit masks
#undef _BV
#define _BV(b) (1 << (b))
#define TEST(n,b) !!((n)&_BV(b))
#define SBI(n,b) (n |= _BV(b))
#define CBI(n,b) (n &= ~_BV(b))
#define SET_BIT_TO(N,B,TF) do{ if (TF) SBI(N,B); else CBI(N,B); }while(0)

#define _BV32(b) (1UL << (b))
#define TEST32(n,b) !!((n)&_BV32(b))
#define SBI32(n,b) (n |= _BV32(b))
#define CBI32(n,b) (n &= ~_BV32(b))
#define SIGN(a) ((a>0)-(a<0))

// math basics

#define WITHIN(V,L,H) ((V) >= (L) && (V) <= (H))
#define NUMERIC(a) WITHIN(a, '0', '9')
#define DECIMAL(a) (NUMERIC(a) || a == '.')
#define NUMERIC_SIGNED(a) (NUMERIC(a) || (a) == '-' || (a) == '+')
#define DECIMAL_SIGNED(a) (DECIMAL(a) || (a) == '-' || (a) == '+')
#define COUNT(a) (sizeof(a)/sizeof(*a))
#define ZERO(a) memset(a,0,sizeof(a))
#define COPY(a,b) memcpy(a,b,MIN(sizeof(a),sizeof(b)))


// #define M_PI 3.14159265358979323846f
#define RADIANS(d) ((d)*M_PI/180.0f)
#define DEGREES(r) ((r)*180.0f/M_PI)
#define CEILING(x,y) (((x) + (y) - 1) / (y))

// Macros for initializing arrays
#define ARRAY_6(v1, v2, v3, v4, v5, v6, ...) { v1, v2, v3, v4, v5, v6 }
#define ARRAY_5(v1, v2, v3, v4, v5, ...)     { v1, v2, v3, v4, v5 }
#define ARRAY_4(v1, v2, v3, v4, ...)         { v1, v2, v3, v4 }
#define ARRAY_3(v1, v2, v3, ...)             { v1, v2, v3 }
#define ARRAY_2(v1, v2, ...)                 { v1, v2 }
#define ARRAY_1(v1, ...)                     { v1 }

#define _ARRAY_N(N, ...) ARRAY_ ##N(__VA_ARGS__)
#define ARRAY_N(N, ...) _ARRAY_N(N, __VA_ARGS__)

#define SPACE(A) " " << A << " "

#endif

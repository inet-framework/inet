// bleh
#ifndef CC_H
#define CC_H

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include "INETDefs.h"

#define PACK_STRUCT_STRUCT __attribute__ (( __packed__ ))
#define LWIP_PLATFORM_ASSERT(c) assert(c)
/*
#define LWIP_PLATFORM_DIAG(c) do { printf("%u] ", get_stack_id()); printf(c); } while (0)
*/
//#define LWIP_PLATFORM_DIAG(c) printf(c)
#define LWIP_PLATFORM_DIAG(c) do { ev<<"(LWIP):"; ev.printf c; } while (0)

#ifndef BYTE_ORDER
#define BYTE_ORDER LITTLE_ENDIAN
#endif

typedef intptr_t mem_ptr_t;

#ifndef u8_t
typedef uint8_t u8_t;
#endif

#ifndef u16_t
typedef uint16_t u16_t;
#endif

#ifndef u32_t
typedef uint32_t u32_t;
#endif

// ---

#ifndef s8_t
typedef int8_t s8_t;
#endif

#ifndef s16_t
typedef int16_t s16_t;
#endif

#ifndef s32_t
typedef int32_t s32_t;
#endif

#define U16_F   "hu"
#define X16_F   "hx"
#define S16_F   "hi"
#define S32_F   "i"
#define U32_F   "u"

#endif

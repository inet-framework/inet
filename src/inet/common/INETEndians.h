//
// Copyright (C) 2005 OpenSim Ltd.
// Copyright (C) 2010 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_INETENDIANS_H
#define __INET_INETENDIANS_H

//
// Get endianness macros defined
//
#if defined(_WIN32) /*MSVC and MinGW*/
#define LITTLE_ENDIAN    1
#define BIG_ENDIAN       2
#define BYTE_ORDER       LITTLE_ENDIAN   /* TODO at least on x86 */
#elif defined(__linux__)
#include <endian.h>
#define LITTLE_ENDIAN    __LITTLE_ENDIAN
#define BIG_ENDIAN       __BIG_ENDIAN
#define BYTE_ORDER       __BYTE_ORDER
#elif defined(__APPLE__)
#include <machine/endian.h>
#else /* fallback, including cases __FreeBSD__, __NetBSD__ and __OpenBSD__ */
// TODO this causes problems in FreeBSD, and probably not needed anyway: #define __BSD_VISIBLE
#include <machine/endian.h>
#endif // if defined(_WIN32)

#if !defined(LITTLE_ENDIAN) || !defined(BIG_ENDIAN) || !defined(BYTE_ORDER) || (BYTE_ORDER != LITTLE_ENDIAN && BYTE_ORDER != BIG_ENDIAN)
#error Endian macros (LITTLE_ENDIAN, BIG_ENDIAN, BYTE_ORDER) are not set up correctly -- please fix this header file and report it.
#endif // if !defined(LITTLE_ENDIAN) || !defined(BIG_ENDIAN) || !defined(BYTE_ORDER) || (BYTE_ORDER != LITTLE_ENDIAN && BYTE_ORDER != BIG_ENDIAN)

#if BYTE_ORDER == LITTLE_ENDIAN

#if !defined(htole16)
#define htole16(x)    (x)
#define htole32(x)    (x)
#define le16toh(x)    (x)
#define le32toh(x)    (x)
#endif // if !defined(htole16)

#else // if BYTE_ORDER == LITTLE_ENDIAN

#if defined(htole16)
#else
#if defined(__linux__)
#include <bits/byteswap.h>

#define htole16(x)    __bswap_16(x)
#define htole32(x)    __bswap_32(x)
#else // if defined(__linux__)
#define htole16(x)    ((((x) & 0x00FF) << 8) | (((x) & 0xFF00) >> 8))
#define htole32(x)    ((((x) & 0x000000FF) << 24) | (((x) & 0x0000FF00) << 8) | (((x) & 0x00FF0000) >> 8) | (((x) & 0xFF000000) >> 24))
#define le16toh(x)    htole16(x)
#define le32toh(x)    htole32(x)
#endif // if defined(__linux__)
#endif // if defined(htole16)

#endif // if BYTE_ORDER == LITTLE_ENDIAN

#endif


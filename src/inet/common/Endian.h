//
// Copyright (C) 2005 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_DEFS_H
#define __INET_DEFS_H

#include <stdint.h>    /* ISO C compliant platforms: Linux, Mac OS/X, recent MSVC versions */

#include "inet/common/INETDefs.h"

//
// Get endianness macros defined
//
#if defined(_WIN32)    /*MSVC and MinGW*/
# include <ws2tcpip.h>
# include <winsock2.h>
# define LITTLE_ENDIAN    1
# define BIG_ENDIAN       2
# define BYTE_ORDER       LITTLE_ENDIAN   /* XXX at least on x86 */
#elif defined(linux) || defined(__linux)
# include <endian.h>
# include <netinet/in.h>
# define LITTLE_ENDIAN    __LITTLE_ENDIAN
# define BIG_ENDIAN       __BIG_ENDIAN
# define BYTE_ORDER       __BYTE_ORDER
#elif defined(__APPLE__)
#include <machine/endian.h>
#include <netinet/in.h>
#else /* fallback, including cases __FreeBSD__, __NetBSD__ and __OpenBSD__ */
//XXX this causes problems in FreeBSD, and probably not needed anyway: #define __BSD_VISIBLE
#include <machine/endian.h>

#endif // if defined(_WIN32)

#if !defined(LITTLE_ENDIAN) || !defined(BIG_ENDIAN) || !defined(BYTE_ORDER) || (BYTE_ORDER != LITTLE_ENDIAN && BYTE_ORDER != BIG_ENDIAN)
#error Endian macros (LITTLE_ENDIAN, BIG_ENDIAN, BYTE_ORDER) are not set up correctly -- please fix this header file and report it.
#endif // if !defined(LITTLE_ENDIAN) || !defined(BIG_ENDIAN) || !defined(BYTE_ORDER) || (BYTE_ORDER != LITTLE_ENDIAN && BYTE_ORDER != BIG_ENDIAN)

#endif // ifndef __INET_DEFS_H


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

#ifndef __INET_PRECOMPILED_H
#define __INET_PRECOMPILED_H

// NOTE: All macros that modify the behavior of an omnet or system header file
// MUST be defined inside this header otherwise the precompiled header will be incorrectly built

// Use winsock2 instead of winsock on Windows (must be defined before platsep/sockets.h pulled in in precompiled.h)
#if !defined(WANT_WINSOCK2)
#define WANT_WINSOCK2
#endif

// reduce the size of the windows header (reduce the number of APIs in windows.h)
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif

// use GNU compatibility (see: https://www.gnu.org/software/libc/manual/html_node/Feature-Test-Macros.html)
// for asprintf() and other functions
#if !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif

// platdep/sockets MUST preceed omnetpp.h to work correctly 
#include <platdep/sockets.h>
#include <omnetpp.h>

// mingw gcc/clang defines "interface" as a macro in windows.h which should be avoided as it clashes with local variables
#undef interface

#endif // ifndef __INET_PRECOMPILED_H

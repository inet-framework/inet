//
// Copyright (C) 2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#ifndef _IPSUITE_DEFS_H__
#define _IPSUITE_DEFS_H__

//
// General definitions.
// Andras Varga
//

#include <omnetpp.h>

typedef unsigned long ulong;
typedef unsigned int uint32;

//FIXME Check below doesn't work:
// MSVC6.0 gives C1017: invalid integer constant expression
// GCC: some other weird error
//# if (sizeof(int)!=4)
//#  error unsigned int is not 32 bits -- modify uint32 definition in ipsuite_defs.h
//# endif


//
// Macro to prevent executing ev<< statements in Express mode.
// Compare ev/sec values with code compiled with #define EV ev.
// After 3.0a4 change to:
// #define EV ev.disabled()?ev:ev
//
#define EV ev.disable_tracing?ev:ev

#endif

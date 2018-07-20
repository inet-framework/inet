//
// Copyright (C) 2004 Andras Varga
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

#ifndef __INET_TCPSEGMENT_H
#define __INET_TCPSEGMENT_H

#include <list>

#include "inet/common/INETDefs.h"
#include "inet/transportlayer/tcp_common/TcpHeader_m.h"

namespace inet {

namespace tcp {

/** @name Comparing sequence numbers */
//@{
inline bool seqLess(uint32 a, uint32 b) { return a != b && (b - a) < (1UL << 31); }
inline bool seqLE(uint32 a, uint32 b) { return (b - a) < (1UL << 31); }
inline bool seqGreater(uint32 a, uint32 b) { return a != b && (a - b) < (1UL << 31); }
inline bool seqGE(uint32 a, uint32 b) { return (a - b) < (1UL << 31); }
inline uint32 seqMin(uint32 a, uint32 b) { return ((b - a) < (1UL << 31)) ? a : b; }
inline uint32 seqMax(uint32 a, uint32 b) { return ((a - b) < (1UL << 31)) ? a : b; }
//@}

} // namespace tcp

} // namespace inet

#endif // ifndef __INET_TCPSEGMENT_H


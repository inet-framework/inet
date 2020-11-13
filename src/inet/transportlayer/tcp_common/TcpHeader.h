//
// Copyright (C) 2004 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#ifndef __INET_TCPHEADER_H
#define __INET_TCPHEADER_H

#include <list>

#include "inet/transportlayer/tcp_common/TcpHeader_m.h"

namespace inet {

namespace tcp {

/** @name Comparing sequence numbers */
//@{
inline bool seqLess(uint32_t a, uint32_t b) { return a != b && (b - a) < (1UL << 31); }
inline bool seqLE(uint32_t a, uint32_t b) { return (b - a) < (1UL << 31); }
inline bool seqGreater(uint32_t a, uint32_t b) { return a != b && (a - b) < (1UL << 31); }
inline bool seqGE(uint32_t a, uint32_t b) { return (a - b) < (1UL << 31); }
inline uint32_t seqMin(uint32_t a, uint32_t b) { return ((b - a) < (1UL << 31)) ? a : b; }
inline uint32_t seqMax(uint32_t a, uint32_t b) { return ((a - b) < (1UL << 31)) ? a : b; }
//@}

} // namespace tcp

} // namespace inet

#endif


//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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


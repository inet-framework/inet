//
// Copyright (C) 2019 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_OSPFV2CHECKSUM_H
#define __INET_OSPFV2CHECKSUM_H

#include <vector>

#include "inet/routing/ospfv2/Ospfv2Packet_m.h"

namespace inet {

namespace ospfv2 {

INET_API void setOspfChecksum(const Ptr<Ospfv2Packet>& ospfPacket, ChecksumMode checksumMode);
INET_API void setLsaChecksum(Ospfv2Lsa& lsa, ChecksumMode checksumMode);
INET_API void setLsaHeaderChecksum(Ospfv2LsaHeader& lsaHeader, ChecksumMode checksumMode);

} // namespace ospfv2

} // namespace inet

#endif


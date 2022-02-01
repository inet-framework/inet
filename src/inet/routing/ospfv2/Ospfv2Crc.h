//
// Copyright (C) 2019 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_OSPFV2CRC_H
#define __INET_OSPFV2CRC_H

#include <vector>

#include "inet/routing/ospfv2/Ospfv2Packet_m.h"

namespace inet {

namespace ospfv2 {

INET_API void setOspfCrc(const Ptr<Ospfv2Packet>& ospfPacket, CrcMode crcMode);
INET_API void setLsaCrc(Ospfv2Lsa& lsa, CrcMode crcMode);
INET_API void setLsaHeaderCrc(Ospfv2LsaHeader& lsaHeader, CrcMode crcMode);

} // namespace ospfv2

} // namespace inet

#endif


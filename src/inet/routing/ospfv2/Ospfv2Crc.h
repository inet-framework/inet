//
// Copyright (C) 2019 OpenSim Ltd.
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
// author: Zoltan Bojthe
//

#ifndef __INET_OSPFV2CRC_H
#define __INET_OSPFV2CRC_H

#include <vector>

#include "inet/common/INETDefs.h"
#include "inet/routing/ospfv2/Ospfv2Packet_m.h"

namespace inet {

namespace ospfv2 {

INET_API void setOspfCrc(const Ptr<Ospfv2Packet>& ospfPacket, CrcMode crcMode);
INET_API void setLsaCrc(Ospfv2Lsa& lsa, CrcMode crcMode);
INET_API void setLsaHeaderCrc(Ospfv2LsaHeader& lsaHeader, CrcMode crcMode);

} // namespace ospfv2

} // namespace inet

#endif // ifndef __INET_OSPFV2CRC_H


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
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#ifndef __INET_PACKETEVENTTAG_H
#define __INET_PACKETEVENTTAG_H

#include "inet/common/packet/Packet.h"
#include "inet/common/PacketEventTag_m.h"

namespace inet {

INET_API void insertPacketEvent(const cModule *module, Packet *packet, int kind, simtime_t duration, PacketEvent *packetEvent = new PacketEvent());

} // namespace inet

#endif // ifndef __INET_PACKETEVENTTAG_H


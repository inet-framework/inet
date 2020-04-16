//
// Copyright (C) OpenSim Ltd.
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
// along with this program; if not, see http://www.gnu.org/licenses/.
//

#ifndef __INET_PACKETCOMPARATORFUNCTION_H
#define __INET_PACKETCOMPARATORFUNCTION_H

#include "inet/queueing/contract/IPacketComparatorFunction.h"

namespace inet {
namespace queueing {

typedef int (*PacketComparatorFunction)(Packet *packet1, Packet *packet2);

class INET_API CPacketComparatorFunction : public cObject, public IPacketComparatorFunction
{
  protected:
    PacketComparatorFunction packetComparatorFunction;

  public:
    CPacketComparatorFunction(PacketComparatorFunction packetComparatorFunction) : packetComparatorFunction(packetComparatorFunction) { }

    virtual CPacketComparatorFunction *dup() const override { return new CPacketComparatorFunction(packetComparatorFunction); }
    virtual int comparePackets(Packet *packet1, Packet *packet2) const override { return packetComparatorFunction(packet1, packet2); }
};

#define Register_Packet_Comparator_Function(name, function) \
    class INET_API name : public ::inet::queueing::CPacketComparatorFunction { public: name() : CPacketComparatorFunction(function) { } }; \
    Register_Class(name)

} // namespace queueing
} // namespace inet

#endif // ifndef __INET_PACKETCOMPARATORFUNCTION_H


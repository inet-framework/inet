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

#ifndef __INET_PACKETCLASSIFIERFUNCTION_H
#define __INET_PACKETCLASSIFIERFUNCTION_H

#include "inet/queueing/contract/IPacketClassifierFunction.h"

namespace inet {
namespace queueing {

typedef int (*PacketClassifierFunction)(Packet *packet);

class INET_API CPacketClassifierFunction : public cObject, public IPacketClassifierFunction
{
  protected:
    PacketClassifierFunction packetClassifierFunction;

  public:
    CPacketClassifierFunction(PacketClassifierFunction packetClassifierFunction) : packetClassifierFunction(packetClassifierFunction) { }

    virtual int classifyPacket(Packet *packet) const override { return packetClassifierFunction(packet); }
};

#define Register_Packet_Classifier_Function(name, function) \
    class INET_API name : public ::inet::queueing::CPacketClassifierFunction { public: name() : CPacketClassifierFunction(function) { } }; \
    Register_Class(name)

} // namespace queueing
} // namespace inet

#endif // ifndef __INET_PACKETCLASSIFIERFUNCTION_H


//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PACKETCLASSIFIERFUNCTION_H
#define __INET_PACKETCLASSIFIERFUNCTION_H

#include "inet/queueing/contract/IPacketClassifierFunction.h"

namespace inet {
namespace queueing {

typedef int (*PacketClassifierFunction)(Packet *packet);

class INET_API CPacketClassifierFunction : public cObject, public virtual IPacketClassifierFunction
{
  protected:
    PacketClassifierFunction packetClassifierFunction;

  public:
    CPacketClassifierFunction(PacketClassifierFunction packetClassifierFunction) : packetClassifierFunction(packetClassifierFunction) {}

    virtual int classifyPacket(Packet *packet) const override { return packetClassifierFunction(packet); }
};

#define Register_Packet_Classifier_Function(name, function) \
    class INET_API name : public ::inet::queueing::CPacketClassifierFunction { \
      public: name() : CPacketClassifierFunction(function) {} \
    }; \
    Register_Class(name)

} // namespace queueing
} // namespace inet

#endif


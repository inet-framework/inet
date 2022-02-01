//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PACKETFILTERFUNCTION_H
#define __INET_PACKETFILTERFUNCTION_H

#include "inet/queueing/contract/IPacketFilterFunction.h"

namespace inet {
namespace queueing {

typedef bool (*PacketFilterFunction)(const Packet *packet);

class INET_API CPacketFilterFunction : public cObject, public virtual IPacketFilterFunction
{
  protected:
    PacketFilterFunction packetFilterFunction;

  public:
    CPacketFilterFunction(PacketFilterFunction packetFilterFunction) : packetFilterFunction(packetFilterFunction) {}

    virtual bool matchesPacket(const Packet *packet) const override { return packetFilterFunction(packet); }
};

#define Register_Packet_Filter_Function(name, function) \
    class INET_API name : public ::inet::queueing::CPacketFilterFunction { \
      public: name() : CPacketFilterFunction(function) {} \
    }; \
    Register_Class(name)

} // namespace queueing
} // namespace inet

#endif


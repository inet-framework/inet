//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PACKETFILTERFUNCTION_H
#define __INET_PACKETFILTERFUNCTION_H

#include <functional>

#include "inet/queueing/contract/IPacketFilterFunction.h"

namespace inet {
namespace queueing {

typedef bool (*PacketFilterFunction)(const Packet *packet);

class INET_API CPacketFilterFunction : public cObject, public virtual IPacketFilterFunction
{
  protected:
    std::function<bool(const Packet *)> packetFilterFunction;

  public:
    CPacketFilterFunction(std::function<bool(const Packet *)> packetFilterFunction) : packetFilterFunction(packetFilterFunction) {}

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


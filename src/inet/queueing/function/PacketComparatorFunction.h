//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PACKETCOMPARATORFUNCTION_H
#define __INET_PACKETCOMPARATORFUNCTION_H

#include "inet/queueing/contract/IPacketComparatorFunction.h"

namespace inet {
namespace queueing {

typedef int (*PacketComparatorFunction)(Packet *packet1, Packet *packet2);

class INET_API CPacketComparatorFunction : public cObject, public virtual IPacketComparatorFunction
{
  protected:
    PacketComparatorFunction packetComparatorFunction;

  public:
    CPacketComparatorFunction(PacketComparatorFunction packetComparatorFunction) : packetComparatorFunction(packetComparatorFunction) {}

    virtual CPacketComparatorFunction *dup() const override { return new CPacketComparatorFunction(packetComparatorFunction); }
    virtual int comparePackets(Packet *packet1, Packet *packet2) const override { return packetComparatorFunction(packet1, packet2); }
};

#define Register_Packet_Comparator_Function(name, function) \
    class INET_API name : public ::inet::queueing::CPacketComparatorFunction { \
      public: name() : CPacketComparatorFunction(function) {} \
    }; \
    Register_Class(name)

} // namespace queueing
} // namespace inet

#endif


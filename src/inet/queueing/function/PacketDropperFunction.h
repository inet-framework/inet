//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PACKETDROPPERFUNCTION_H
#define __INET_PACKETDROPPERFUNCTION_H

#include "inet/queueing/contract/IPacketDropperFunction.h"

namespace inet {
namespace queueing {

typedef Packet *(*PacketDropperFunction)(IPacketCollection *packets);

class INET_API CPacketDropperFunction : public cObject, public virtual IPacketDropperFunction
{
  protected:
    PacketDropperFunction packetDropperFunction;

  public:
    CPacketDropperFunction(PacketDropperFunction packetDropperFunction) : packetDropperFunction(packetDropperFunction) {}

    virtual Packet *selectPacket(IPacketCollection *collection) const override;
};

#define Register_Packet_Dropper_Function(name, function) \
    class INET_API name : public ::inet::queueing::CPacketDropperFunction { \
      public: name() : CPacketDropperFunction(function) {} \
    }; \
    Register_Class(name)

} // namespace queueing
} // namespace inet

#endif


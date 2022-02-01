//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PROTOCOLCHECKER_H
#define __INET_PROTOCOLCHECKER_H

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/queueing/base/PacketFilterBase.h"

namespace inet {

using namespace inet::queueing;

class INET_API ProtocolChecker : public PacketFilterBase, public DefaultProtocolRegistrationListener
{
  protected:
    std::set<const Protocol *> protocols;

  protected:
    virtual void initialize(int stage) override;
    virtual void dropPacket(Packet *packet) override;

    virtual void handleRegisterProtocol(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive) override;

  public:
    virtual bool matchesPacket(const Packet *packet) const override;
};

} // namespace inet

#endif


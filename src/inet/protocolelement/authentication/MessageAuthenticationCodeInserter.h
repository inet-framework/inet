//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_MESSAGEAUTHENTICATIONCODEINSERTER_H
#define __INET_MESSAGEAUTHENTICATIONCODEINSERTER_H

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/queueing/base/PacketFlowBase.h"

namespace inet {

using namespace inet::queueing;

class INET_API MessageAuthenticationCodeInserter : public PacketFlowBase, public TransparentProtocolRegistrationListener
{
  protected:
    b headerLength = b(-1);

  protected:
    virtual void initialize(int stage) override;
    virtual void processPacket(Packet *packet) override;
    virtual cGate *getRegistrationForwardingGate(cGate *gate) override;
};

} // namespace inet

#endif


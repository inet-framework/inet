//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_STREAMIDENTIFIER_H
#define __INET_STREAMIDENTIFIER_H

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/queueing/base/PacketFlowBase.h"

namespace inet {

using namespace inet::queueing;

class INET_API StreamIdentifier : public PacketFlowBase, public TransparentProtocolRegistrationListener
{
  protected:
    bool hasSequenceNumbering = false;
    cValueArray *mapping = nullptr;

    std::map<std::string, int> sequenceNumbers;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleParameterChange(const char *name) override;
    virtual void processPacket(Packet *packet) override;
    virtual int incrementSequenceNumber(const char *stream);

    virtual cGate *getRegistrationForwardingGate(cGate *gate) override;
};

} // namespace inet

#endif


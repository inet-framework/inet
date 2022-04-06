//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ETHERNETFCSCHECKER_H
#define __INET_ETHERNETFCSCHECKER_H

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/protocolelement/checksum/base/FcsCheckerBase.h"

namespace inet {

using namespace inet::queueing;

class INET_API EthernetFcsChecker : public FcsCheckerBase, public TransparentProtocolRegistrationListener
{
  protected:
    bool popFcs = true;

  protected:
    virtual void initialize(int stage) override;
    virtual bool checkFcs(const Packet *packet, FcsMode fcsMode, uint32_t fcs) const override;
    virtual void processPacket(Packet *packet) override;
    virtual void dropPacket(Packet *packet) override;
    virtual cGate *getRegistrationForwardingGate(cGate *gate) override;

  public:
    virtual bool matchesPacket(const Packet *packet) const override;
};

} // namespace inet

#endif


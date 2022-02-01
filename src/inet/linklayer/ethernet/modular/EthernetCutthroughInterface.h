//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ETHERNETCUTTHROUGHINTERFACE_H
#define __INET_ETHERNETCUTTHROUGHINTERFACE_H

#include "inet/networklayer/common/NetworkInterface.h"

namespace inet {

using namespace inet::queueing;

class INET_API EthernetCutthroughInterface : public NetworkInterface
{
  protected:
    cGate *cutthroughInputGate = nullptr;
    cGate *cutthroughOutputGate = nullptr;
    queueing::IPassivePacketSink *cutthroughInputConsumer = nullptr;
    queueing::IPassivePacketSink *cutthroughOutputConsumer = nullptr;

  protected:
    virtual void initialize(int stage) override;

  public:
    virtual void pushPacketStart(Packet *packet, cGate *gate, bps datarate) override;
    virtual void pushPacketEnd(Packet *packet, cGate *gate) override;
};

} // namespace inet

#endif


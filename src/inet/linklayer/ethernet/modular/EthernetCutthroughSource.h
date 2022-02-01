//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ETHERNETCUTTHROUGHSOURCE_H
#define __INET_ETHERNETCUTTHROUGHSOURCE_H

#include "inet/common/ModuleRef.h"
#include "inet/common/ModuleRefByPar.h"
#include "inet/linklayer/ethernet/contract/IMacForwardingTable.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/protocolelement/common/PacketDestreamer.h"

namespace inet {

using namespace inet::queueing;

class INET_API EthernetCutthroughSource : public PacketDestreamer
{
  protected:
    cGate *cutthroughOutputGate = nullptr;
    ModuleRef<IPassivePacketSink> cutthroughConsumer;

    NetworkInterface *networkInterface = nullptr;
    ModuleRefByPar<IMacForwardingTable> macForwardingTable;

    bool cutthroughInProgress = false;
    cMessage *cutthroughTimer = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;

  public:
    virtual ~EthernetCutthroughSource() { cancelAndDelete(cutthroughTimer); }

    virtual void pushPacketStart(Packet *packet, cGate *gate, bps datarate) override;
    virtual void pushPacketEnd(Packet *packet, cGate *gate) override;
};

} // namespace inet

#endif


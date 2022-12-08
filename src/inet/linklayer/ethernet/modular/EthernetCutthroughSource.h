//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ETHERNETCUTTHROUGHSOURCE_H
#define __INET_ETHERNETCUTTHROUGHSOURCE_H

#include "inet/common/ModuleRef.h"
#include "inet/common/ModuleRefByPar.h"
#include "inet/common/packet/chunk/StreamBufferChunk.h"
#include "inet/common/packet/dissector/PacketDissector.h"
#include "inet/linklayer/ethernet/contract/IMacForwardingTable.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/protocolelement/common/PacketDestreamer.h"

namespace inet {

using namespace inet::queueing;

class INET_API EthernetCutthroughSource : public PacketDestreamer
{
  protected:
    class EthernetCutthroughHeaderSizeCallback : public PacketDissector::ICallback
    {
      public:
        b cutthroughSwitchingHeaderSize = b(0);

      public:
        virtual bool shouldDissectProtocolDataUnit(const Protocol *protocol) override;
        virtual void startProtocolDataUnit(const Protocol *protocol) override {}
        virtual void endProtocolDataUnit(const Protocol *protocol) override {}
        virtual void markIncorrect() override {}
        virtual void visitChunk(const Ptr<const Chunk>& chunk, const Protocol *protocol) override;
    };

    NetworkInterface *networkInterface = nullptr;
    ModuleRefByPar<IMacForwardingTable> macForwardingTable;

    b cutthroughSwitchingHeaderSize;
    bool cutthroughInProgress = false;
    cMessage *cutthroughTimer = nullptr;

    Ptr<StreamBufferChunk> cutthroughBuffer = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;

    virtual b getCutthroughSwitchingHeaderSize(Packet *packet) const;

  public:
    virtual ~EthernetCutthroughSource() { cancelAndDelete(cutthroughTimer); }

    virtual void pushPacketStart(Packet *packet, cGate *gate, bps datarate) override;
    virtual void pushPacketEnd(Packet *packet, cGate *gate) override;
};

} // namespace inet

#endif


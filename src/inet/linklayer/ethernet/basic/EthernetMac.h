//
// Copyright (C) 2006 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ETHERNETMAC_H
#define __INET_ETHERNETMAC_H

#include "inet/linklayer/ethernet/base/EthernetMacBase.h"

namespace inet {

/**
 * A simplified version of EthernetCsmaMac. Since modern Ethernets typically
 * operate over duplex links where's no contention, the original CSMA/CD
 * algorithm is no longer needed. This simplified implementation doesn't
 * contain CSMA/CD, frames are just simply queued up and sent out one by one.
 */
class INET_API EthernetMac : public EthernetMacBase
{
  public:
    EthernetMac();

    // IActivePacketSink:
    virtual void handleCanPullPacketChanged(cGate *gate) override;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void initializeStatistics() override;
    virtual void initializeFlags() override;
    virtual void handleMessageWhenUp(cMessage *msg) override;

    // finish
    virtual void finish() override;

    // event handlers
    virtual void handleEndIFGPeriod();
    virtual void handleEndTxPeriod();
    virtual void handleEndPausePeriod();
    virtual void handleSelfMessage(cMessage *msg) override;

    // helpers
    virtual void startFrameTransmission();
    virtual void handleUpperPacket(Packet *pk) override;
    virtual void processMsgFromNetwork(Signal *signal);
    virtual void processReceivedDataFrame(Packet *packet, const Ptr<const EthernetMacHeader>& frame);
    virtual void processPauseCommand(int pauseUnits);
    virtual void scheduleEndIFGPeriod();
    virtual void scheduleEndPausePeriod(int pauseUnits);
    virtual void beginSendFrames();

    // statistics
    simtime_t totalSuccessfulRxTime; // total duration of successful transmissions on channel
};

} // namespace inet

#endif


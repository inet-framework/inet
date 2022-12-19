//
// Copyright (C) 2004 OpenSim Ltd.
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IPVXTRAFGEN_H
#define __INET_IPVXTRAFGEN_H

#include <vector>

#include "inet/applications/base/ApplicationBase.h"
#include "inet/applications/generic/IpvxTrafSink.h"
#include "inet/common/Protocol.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/L3Address.h"

namespace inet {

/**
 * IP traffic generator application. See NED for more info.
 */
class INET_API IpvxTrafGen : public ApplicationBase
{
  protected:
    enum Kinds { START = 100, NEXT };

    // parameters: see the NED files for more info
    simtime_t startTime;
    simtime_t stopTime;
    cPar *sendIntervalPar = nullptr;
    cPar *packetLengthPar = nullptr;
    const Protocol *protocol = nullptr;
    std::vector<L3Address> destAddresses;
    int numPackets = 0;

    // state
    cMessage *timer = nullptr;

    // statistic
    int numSent = 0;
    int numReceived = 0;

  protected:
    virtual void scheduleNextPacket(simtime_t previous);
    virtual void cancelNextPacket();
    virtual bool isEnabled();

    virtual L3Address chooseDestAddr();
    virtual void sendPacket();

    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void refreshDisplay() const override;
    virtual void startApp();

    virtual void printPacket(Packet *msg);
    virtual void processPacket(Packet *msg);

    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

  public:
    IpvxTrafGen();
    virtual ~IpvxTrafGen();
};

} // namespace inet

#endif


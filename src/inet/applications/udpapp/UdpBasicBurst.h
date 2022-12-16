//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_UDPBASICBURST_H
#define __INET_UDPBASICBURST_H

#include <map>
#include <vector>

#include "inet/applications/base/ApplicationBase.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"

namespace inet {

/**
 * UDP application. See NED for more info.
 */
class INET_API UdpBasicBurst : public ApplicationBase, public UdpSocket::ICallback
{
  public:
    enum ChooseDestAddrMode {
        ONCE = 1, PER_BURST, PER_SEND
    };

  protected:
    enum SelfMsgKinds { START = 1, SEND, STOP };
    typedef std::map<int, int> SourceSequence;

    // parameters
    std::vector<L3Address> destAddresses;
    ChooseDestAddrMode chooseDestAddrMode = static_cast<ChooseDestAddrMode>(0);
    simtime_t delayLimit;
    simtime_t startTime;
    simtime_t stopTime;
    int localPort = -1, destPort = -1;
    int destAddrRNG = -1;

    // volatile parameters:
    cPar *messageLengthPar = nullptr;
    cPar *burstDurationPar = nullptr;
    cPar *sleepDurationPar = nullptr;
    cPar *sendIntervalPar = nullptr;

    // state
    UdpSocket socket;
    L3Address destAddr;
    SourceSequence sourceSequence;
    cMessage *timerNext = nullptr;
    simtime_t nextPkt;
    simtime_t nextBurst;
    simtime_t nextSleep;
    bool isSource = false;
    bool activeBurst = false;
    bool haveSleepDuration = false;
    bool dontFragment = false;

    // statistics:
    uint64_t& counter = SIMULATION_SHARED_COUNTER(counter); // counter for generating a global number for each packet

    int numSent = 0;
    int numReceived = 0;
    int numDeleted = 0;
    int numDuplicated = 0;

    static simsignal_t outOfOrderPkSignal;

  protected:
    // chooses random destination address
    virtual L3Address chooseDestAddr();
    virtual Packet *createPacket();
    virtual void processPacket(Packet *msg);
    virtual void generateBurst();

    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void finish() override;
    virtual void refreshDisplay() const override;

    virtual void processStart();
    virtual void processSend();
    virtual void processStop();

    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

    virtual void socketDataArrived(UdpSocket *socket, Packet *packet) override;
    virtual void socketErrorArrived(UdpSocket *socket, Indication *indication) override;
    virtual void socketClosed(UdpSocket *socket) override;

  public:
    UdpBasicBurst() {}
    ~UdpBasicBurst();
};

} // namespace inet

#endif


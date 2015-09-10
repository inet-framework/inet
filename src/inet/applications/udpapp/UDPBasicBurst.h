//
// Copyright (C) 2004 Andras Varga
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2011 Zoltan Bojthe
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#ifndef __INET_UDPBASICBURST_H
#define __INET_UDPBASICBURST_H

#include <vector>
#include <map>

#include "inet/common/INETDefs.h"

#include "inet/applications/base/ApplicationBase.h"
#include "inet/transportlayer/contract/udp/UDPSocket.h"

namespace inet {

/**
 * UDP application. See NED for more info.
 */
class INET_API UDPBasicBurst : public ApplicationBase
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
    ChooseDestAddrMode chooseDestAddrMode = (ChooseDestAddrMode)0;
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
    UDPSocket socket;
    L3Address destAddr;
    SourceSequence sourceSequence;
    cMessage *timerNext = nullptr;
    simtime_t nextPkt;
    simtime_t nextBurst;
    simtime_t nextSleep;
    bool isSource = false;
    bool activeBurst = false;
    bool haveSleepDuration = false;

    // statistics:
    static int counter;    // counter for generating a global number for each packet

    int numSent = 0;
    int numReceived = 0;
    int numDeleted = 0;
    int numDuplicated = 0;

    static simsignal_t sentPkSignal;
    static simsignal_t rcvdPkSignal;
    static simsignal_t outOfOrderPkSignal;
    static simsignal_t dropPkSignal;

  protected:
    // chooses random destination address
    virtual L3Address chooseDestAddr();
    virtual cPacket *createPacket();
    virtual void processPacket(cPacket *msg);
    virtual void generateBurst();

    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void finish() override;

    virtual void processStart();
    virtual void processSend();
    virtual void processStop();

    virtual bool handleNodeStart(IDoneCallback *doneCallback) override;
    virtual bool handleNodeShutdown(IDoneCallback *doneCallback) override;
    virtual void handleNodeCrash() override;

  public:
    UDPBasicBurst() {}
    ~UDPBasicBurst();
};

} // namespace inet

#endif // ifndef __INET_UDPBASICBURST_H


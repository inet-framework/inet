//
// Copyright (C) 2004 Andras Varga
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2011 Zoltan Bojthe
// Copyright (C) 2013 Universidad de Malaga
// Copyright (C) 2021 Universidad de Malaga
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


#ifndef __INET_UDPBASICFLOODING_H
#define __INET_UDPBASICFLOODING_H

#include <map>
#include <vector>

#include "inet/applications/base/ApplicationBase.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"
#include "inet/applications/common/AddressModule.h"

namespace inet {

/**
 * UDP application. See NED for more info.
 */
class INET_API UdpBasicFlooding :  public ApplicationBase, public UdpSocket::ICallback
{
 public:
  enum ChooseDestAddrMode {
      ONCE = 1, PER_BURST, PER_SEND
  };

  protected:
    enum SelfMsgKinds { START = 1, SEND, STOP };
    AddressModule * addressModule = nullptr;
    UdpSocket socket;
    int localPort = -1;
    int destPort = -1;

    int destAddrRNG = -1;
    int myId = -1;

    typedef std::map<int,int> SourceSequence;
    SourceSequence sourceSequence;
    simtime_t delayLimit;
    cMessage *timerNext = nullptr;
    simtime_t stopTime;
    simtime_t startTime;
    simtime_t nextPkt;
    simtime_t nextBurst;
    simtime_t nextSleep;
    bool activeBurst;
    bool isSource;
    bool haveSleepDuration;
    std::vector<int> outputInterfaceMulticastBroadcast;

    static int counter; // counter for generating a global number for each packet

    int numSent = 0;
    int numReceived = 0;
    int numDeleted = 0;
    int numDuplicated = 0;
    int numFlood = 0;

    // volatile parameters:
    cPar *messageLengthPar = nullptr;
    cPar *burstDurationPar = nullptr;
    cPar *sleepDurationPar = nullptr;
    cPar *sendIntervalPar = nullptr;

    ChooseDestAddrMode chooseDestAddrMode = static_cast<ChooseDestAddrMode>(0);
    //statistics:
    static simsignal_t sentPkSignal;
    static simsignal_t rcvdPkSignal;
    static simsignal_t floodPkSignal;
    static simsignal_t outOfOrderPkSignal;
    static simsignal_t dropPkSignal;

    int destAddr;
    // chooses random destination address
    virtual Packet *createPacket();
    virtual void processPacket(Packet *msg);
    virtual void generateBurst();

  protected:
    virtual void initialize(int stage)  override;
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void finish() override;
    virtual bool sendBroadcast(const L3Address &dest, Packet *pkt);
    virtual void processConfigure();

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
    UdpBasicFlooding();
    virtual ~UdpBasicFlooding();
};

}

#endif

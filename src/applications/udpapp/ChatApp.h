
/*
  Copyright Leonardo Maccari, 2011. This software has been developed
  for the PAF-FPE Project financed by EU and Provincia di Trento.
  See www.pervacy.eu or contact me at leonardo.maccari@unitn.it

  I'd be greatful if:
  - you keep the above copyright notice
  - you cite pervacy.eu if you reuse this code

  This file is part of ChatApp.

  ChatApp is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  ChatApp is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with ChatApp.  If not, see <http://www.gnu.org/licenses/>.

  ChatApp is based on UDPBasicBurst by Alfonso Ariza and Andras Varga

*/


#ifndef __INET_CHATAPP_H
#define __INET_CHATAPP_H

#include <vector>
#include <map>

#include "INETDefs.h"

#include "IPvXAddress.h"
#include "UDPBasicApp.h"
#include "AddressGenerator.h"


typedef struct Burst {
    simtime_t stopTime;
	IPvXAddress destAddr;

    friend std::ostream& operator<< (std::ostream &out, Burst &burst);

    Burst() : stopTime(0) , destAddr() {}
    Burst(simtime_t stop) : stopTime(stop) , destAddr() {}
    Burst(simtime_t stop, IPvXAddress target) : stopTime(stop) , destAddr(target) {}
} Burst;

std::ostream& operator<< (std::ostream &out, Burst &burst)
{
	out << "(" << burst.destAddr << ", " <<
	        burst.stopTime << ")";
    return out;
}

typedef struct MsgContext {
    bool start;
    IPvXAddress target;
} MsgContext;
/**
 * UDP application. See NED for more info.
 */
class INET_API ChatApp : public UDPBasicApp
{
  public:
    enum ChooseDestAddrMode
    {
        ONCE = 1, PER_BURST, PER_SEND
    };

    protected:
    int localPort, destPort;

    AddressGenerator * addressGeneratorModule;
    ChooseDestAddrMode chooseDestAddrMode;
    IPvXAddress destAddr;
    int networkSize;
    typedef std::map<IPvXAddress, Burst> BurstList;
    BurstList burstList;

    typedef std::map<int,int> SourceSequence;
    SourceSequence sourceSequence;
    simtime_t delayLimit;
    cMessage *timerNext;
    simtime_t stopTime;
    simtime_t nextPkt;
    simtime_t nextBurst;
    simtime_t nextSleep;
    int intervalUpperBound;
    bool activeBurst;
    bool isSource;
    bool haveSleepDuration;
    int graceTime;
    static int counter; // counter for generating a global number for each packet

    double maxSimTime; // not to go beyond simulation time

    int numSent;
    int numReceived;
    int numDeleted;
    int numDuplicated;
    int numUDPErrors;


    // volatile parameters:
    cPar *messageLengthPar;
    cPar *burstDurationPar;
    cPar *burstIntervalPar;
    cPar *messageFreqPar;

    //statistics:
    bool debugStats;
    static simsignal_t sentPkSignal;
    static simsignal_t numUDPErrorsSignal;
    static simsignal_t rcvdPkSignal;
    static simsignal_t outOfOrderPkSignal;
    static simsignal_t dropPkSignal;
    static simsignal_t endToEndDelaySignal;
    static simsignal_t sendInterval;
    static simsignal_t bDuration;
    static simsignal_t burstInterval;
    static simsignal_t messageSize;
    static simsignal_t startedSessions;
    static simsignal_t answeredSessions;
    static simsignal_t targetStatisticsSignal;



    // chooses random destination address
    virtual IPvXAddress chooseDestAddr();
    void chooseDestAddr(IPvXAddress& checkAddr);
    virtual cPacket *createPacket();
    virtual void processPacket(cPacket *msg);

    IPvXAddress generateBurst(Burst*);

  protected:
    virtual int numInitStages() const {return 4;}
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);
    virtual void finish();

  public:
    ChatApp();
    ~ChatApp();
};

#endif


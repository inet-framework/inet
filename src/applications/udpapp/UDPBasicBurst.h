//
// Copyright (C) 2004 Andras Varga
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
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

#include "INETDefs.h"

#include "IPvXAddress.h"
#include "UDPAppBase.h"

class MMapBoard;


/**
 * UDP application. See NED for more info.
 */

class INET_API UDPBasicBurst : public UDPAppBase
{
  protected:
    std::string nodeName;
    int localPort, destPort;
    int msgByteLength;

    std::vector<IPvXAddress> destAddresses;

    typedef std::map<int,int> SurceSequence;
    SurceSequence sourceSequence;
    cStdDev *pktDelay;
    double meanDelay;
    simtime_t limitDelay;
    int randGenerator;
    cMessage timerNext;
    simtime_t endSend;
    simtime_t nextPkt;
    simtime_t timeBurst;
    bool activeBurst;
    IPvXAddress destAddr;
    bool isSink;
    bool offDisable;
    MMapBoard *mmap;
    int * numShare;

    static int counter; // counter for generating a global number for each packet

    int numSent;
    int numReceived;
    int numDeleted;

    // chooses random destination address
    virtual IPvXAddress chooseDestAddr();
    virtual cPacket *createPacket();
    virtual void sendPacket();
    virtual void processPacket(cPacket *msg);
    virtual void generateBurst();

    virtual void sendToUDPDelayed(cPacket *, int srcPort,
                                  const IPvXAddress& destAddr, int destPort, double delay);

  protected:
    virtual int numInitStages() const {return 4;}
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);
    virtual void finish();

  public:
    UDPBasicBurst() {pktDelay = new cStdDev("burst pkt delay"); numShare = NULL;}
    ~UDPBasicBurst();
};

#endif


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


#ifndef __CBRAPP_H__
#define __CBRAPP_H__

#include <vector>
#include <omnetpp.h>

#include "IPControlInfo_m.h"


/**
 * Consumes and prints packets received from the UDP module. See NED for more info.
 */
class IPTrafSink : public cSimpleModule
{
  protected:
    int numReceived;

    virtual void printPacket(cMessage *msg);
    virtual void processPacket(cMessage *msg);

  public:
    Module_Class_Members(IPTrafSink, cSimpleModule, 0);
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};


/**
 * UDP application. See NED for more info.
 */
class IPTrafGen : public IPTrafSink
{
  protected:
    std::string nodeName;
    int protocol;
    int msgLength;
    int numPackets;
    std::vector<IPAddress> destAddresses;

    static int counter; // counter for generating a global number for each packet

    int numSent;

    // chooses random destination address
    virtual IPAddress chooseDestAddr();
    virtual void sendPacket();

  public:
    Module_Class_Members(IPTrafGen, IPTrafSink, 0);
    virtual int numInitStages() const {return 4;}
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);
};

#endif



//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004,2005 Andras Varga
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


//
// Author: Jochen Reber
// Rewrite: Andras Varga 2004,2005
//

#ifndef __UDPPROCESSING_H__
#define __UDPPROCESSING_H__

#include <map>
#include "IPControlInfo_m.h"
#include "UDPControlInfo_m.h"


const char *ERROR_IP_ADDRESS = "10.0.0.255";
const int UDP_HEADER_BYTES = 8;


/**
 * Implements the UDP protocol: encapsulates/decapsulates user data into/from UDP.
 * More info in the NED file.
 */
class INET_API UDP : public cSimpleModule
{
  protected:
    // if false, all incoming packets are sent up on gate 0
    bool dispatchByPort;

    // Maps UDP ports to "from_application"/"to_application" gate indices.
    typedef std::map<int,int> IntMap;
    IntMap port2indexMap;

    int numSent;
    int numPassedUp;
    int numDroppedWrongPort;
    int numDroppedBadChecksum;

  protected:
    // utility: show current statistics above the icon
    void updateDisplayString();

    // utility: look up destPort in applTable
    int findAppGateForPort(int destPort);

    // bind UDP port to gate index
    void bind(int gateIndex, int udpPort);

    // remove binding
    void unbind(int gateIndex, int udpPort);

    // process packets coming from IP
    virtual void processMsgFromIP(UDPPacket *udpPacket);

    // process packets from application
    virtual void processMsgFromApp(cMessage *appData);

    // process commands from application
    virtual void processCommandFromApp(cMessage *msg);

  public:
    Module_Class_Members(UDP, cSimpleModule, 0);
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};

#endif


//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004 Andras Varga
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
// Cleanup and rewrite: Andras Varga 2004
//

#ifndef __UDPPROCESSING_H__
#define __UDPPROCESSING_H__

#include "IPControlInfo_m.h"
#include "UDPControlInfo_m.h"


const char *ERROR_IP_ADDRESS = "10.0.0.255";
const int UDP_HEADER_BYTES = 16;


/**
 * Implements the UDP protocol: encapsulates/decapsulates user data into/from UDP.
 * More info in the NED file.
 */
class UDPProcessing : public cSimpleModule
{
  protected:
    // if false, all incoming packets are sent up on gate 0
    bool dispatchByPort;

    // Maps "from_application" input gate indices to "local_port" parameters of the
    // modules (applications) that are connected there.
    struct UDPApplicationTable
    {
        int size;
        int *port;  // array: port[index]=local_port
    };
    UDPApplicationTable applTable;

    int numSent;
    int numPassedUp;
    int numDroppedWrongPort;
    int numDroppedBadChecksum;

  protected:
    // utility: show current statistics above the icon
    void updateDisplayString();

    // utility: look up destPort in applTable
    virtual int findAppGateForPort(int destPort);

    // process packets coming from IP
    virtual void processMsgFromIp(UDPPacket *udpPacket);

    // process packets from application
    virtual void processMsgFromApp(cMessage *appData);

  public:
    Module_Class_Members(UDPProcessing, cSimpleModule, 0);
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};

#endif


//
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


//
// author: Jochen Reber
//

#ifndef __UDPPROCESSING_H__
#define __UDPPROCESSING_H__

#include "IPInterfacePacket.h"
#include "UDPInterfacePacket_m.h"
#include "basic_consts.h"


const char *ERROR_IP_ADDRESS = "10.0.0.255";
const int UDP_HEADER_BYTES = 16;


/**
 * Implements the UDP protocol: encapsulates/decapsulates user data into/from UDP.
 * More info in the NED file.
 */
class UDPProcessing : public cSimpleModule
{
  protected:

    // Maps "from_application" input gate indices to "local_port" parameters of the
    // modules (applications) that are connected there.
    struct UDPApplicationTable
    {
        int size;
        int *port;  // array: port[index]=local_port
    };
    // FIXME change to std::map<int,int>
    UDPApplicationTable applTable;

    int numSent;
    int numPassedUp;
    int numDroppedWrongPort;
    int numDroppedWrongChecksum;

    virtual void processMsgFromIp(IPInterfacePacket *packet);
    virtual void processMsgFromApp(UDPInterfacePacket *packet);

  public:
    Module_Class_Members(UDPProcessing, cSimpleModule, 0);
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};

#endif


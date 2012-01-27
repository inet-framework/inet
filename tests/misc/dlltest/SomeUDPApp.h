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


#ifndef __SOMEUDPAPP_H__
#define __SOMEUDPAPP_H__

#include <vector>

#include "INETDefs.h"

#include "UDPAppBase.h"


/**
 * A copy of UDPBasicApp, just for testing.
 * NOTE that this class is NOT declared INET_API!
 */
class SomeUDPApp : public UDPAppBase
{
  protected:
    std::string nodeName;
    int localPort, destPort;
    int msgLength;
    std::vector<IPvXAddress> destAddresses;

    static int counter; // counter for generating a global number for each packet

    int numSent;
    int numReceived;

    // chooses random destination address
    virtual IPvXAddress chooseDestAddr();
    virtual void sendPacket();
    virtual void processPacket(cMessage *msg);

  protected:
    virtual int numInitStages() const {return 4;}
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);
};

#endif



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

/*
    author: Jochen Reber
*/

#ifndef __UDPAPP_H__
#define __UDPAPP_H__

#include <vector>
#include <omnetpp.h>

#include "basic_consts.h"
#include "IPInterfacePacket.h"


/**
 * UDP server app. See NED for more info.
 */
class UDPServerApp : public cSimpleModule
{
  protected:
    int numSent;
    int numReceived;
  public:
    Module_Class_Members(UDPServerApp, cSimpleModule, 0);
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};

/**
 * UDP client app. See NED for more info.
 */
class UDPClientApp : public cSimpleModule
{
  protected:
    std::string nodeName;
    int localPort, destPort;
    int msgLength;
    std::vector<IPAddress> destAddresses;
    int counter;

    int numSent;
    int numReceived;

    // chooses random destination address
    IPAddress chooseDestAddr();

  public:
    Module_Class_Members(UDPClientApp, cSimpleModule, 0);
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};

#endif



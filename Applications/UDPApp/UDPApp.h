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

#include <omnetpp.h>

#include "basic_consts.h"
#include "IPInterfacePacket.h"


/**
 * Base class for UDPServer and UDPClient
 */
class UDPAppBase: public cSimpleModule
{
  protected:
    opp_string nodeName;
    int localPort, destPort;
    int msgLength;
    simtime_t msgFreq;
    int destType;

  public:
    Module_Class_Members(UDPAppBase, cSimpleModule, 0);
    virtual void initialize();

};

/**
 * Receives packet, prints out content, reply to request
 */
class UDPServer: public UDPAppBase
{
  public:
    Module_Class_Members(UDPServer, UDPAppBase, 0);
    virtual void handleMessage(cMessage *msg);
};

/**
 * FIXME ...
 */
class UDPClient: public UDPAppBase
{
  protected:
    int contCtr;

    // chooses random destination address
    void chooseDestAddr(IPAddress& dest);

  public:
    Module_Class_Members(UDPClient, UDPAppBase, ACTIVITY_STACK_SIZE);
    virtual void initialize();
    virtual void activity();

};

#endif



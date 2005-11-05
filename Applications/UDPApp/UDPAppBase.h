//
// Copyright (C) 2005 Andras Varga
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


#ifndef __UDPAPPBASE_H__
#define __UDPAPPBASE_H__

#include <omnetpp.h>
#include "IPvXAddress.h"


/**
 * Contains a few utility functions as protected methods, for sending
 * and receiving UDP packets.
 */
class INET_API UDPAppBase : public cSimpleModule
{
  protected:
    /**
     * Tells UDP we want to get all packets arriving on the given port
     */
    virtual void bindToPort(int port);

    /**
     * Sends a packet over UDP
     */
    virtual void sendToUDP(cMessage *msg, int srcPort, const IPvXAddress& destAddr, int destPort);

    /**
     * Prints a brief about packets having an attached UDPControlInfo
     * (i.e. those which just arrived from UDP, or about to be send to UDP).
     */
    virtual void printPacket(cMessage *msg);
};


#endif



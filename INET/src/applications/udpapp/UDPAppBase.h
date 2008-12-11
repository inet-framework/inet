//
// Copyright (C) 2005 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//


#ifndef __INET_UDPAPPBASE_H
#define __INET_UDPAPPBASE_H

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
    virtual void sendToUDP(cPacket *msg, int srcPort, const IPvXAddress& destAddr, int destPort);

    /**
     * Prints a brief about packets having an attached UDPControlInfo
     * (i.e. those which just arrived from UDP, or about to be send to UDP).
     */
    virtual void printPacket(cPacket *msg);
};


#endif



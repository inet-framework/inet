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


#ifndef __ICMPv6_H__
#define __ICMPv6_H__

#include <omnetpp.h>
#include "IPv6Datagram.h"
#include "ICMPv6Message_m.h"


/**
 * ICMPv6 implementation.
 */
class INET_API ICMPv6 : public cSimpleModule
{
  public:
    Module_Class_Members(ICMPv6, cSimpleModule, 0);

    /**
     * This method can be called from other modules to send an ICMPv6 error packet.
     */
    void sendErrorMessage(IPv6Datagram *datagram, ICMPv6Type type, int code);

  protected:
    /**
     * Initialization
     */
    virtual void initialize();

    /**
     * Processing of ICMPv6 messages.
     */
    virtual void handleMessage(cMessage *msg);
};


#endif



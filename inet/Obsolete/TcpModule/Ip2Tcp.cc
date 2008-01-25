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

#include <omnetpp.h>
#include "TransportPacket.h"
#include "IPControlInfo_m.h"

#include "IPAddress.h"



class Ip2Tcp: public cSimpleModule
{
  public:
    Module_Class_Members(Ip2Tcp, cSimpleModule, 0);
    virtual void handleMessage(cMessage *msg);
};


Define_Module(Ip2Tcp);


void Ip2Tcp::handleMessage(cMessage *msg)
{
    TransportPacket *tpacket = check_and_cast<TransportPacket *>(msg);
    bool ipv6 = false; // FIXME

    // get the source and destination address
    IPControlInfo *controlInfo = check_and_cast<IPControlInfo *>(msg->removeControlInfo());
    IPAddress src_addr = controlInfo->srcAddr();
    IPAddress dest_addr = controlInfo->destAddr();
    delete controlInfo;

    cMessage *tcpmessage = new cMessage (*tpacket);

    // from this one we obtain the port numbers
    tcpmessage->addPar("src_port") = tpacket->sourcePort();
    tcpmessage->addPar("dest_port") = tpacket->destinationPort();

    // set kind here from tpacket
    tcpmessage->setKind(tpacket->msgKind());

    if (!ipv6)
    {
       tcpmessage->addPar("src_addr") = src_addr.getInt();
       tcpmessage->addPar("dest_addr") = dest_addr.getInt();
    }
    else
    {
       tcpmessage->addPar("src_addr") = src_addr.str().c_str();
       tcpmessage->addPar("dest_addr") = dest_addr.str().c_str();
    }

    delete tpacket;

    // send the message to the TCP_layer
    send(tcpmessage, "out");
}



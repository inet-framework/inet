//
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

#include <omnetpp.h>
#include "TransportPacket.h"
#include "IPControlInfo_m.h"

#include "IPAddress.h"


class Tcp2Ip: public cSimpleModule
{
  public:
    Module_Class_Members(Tcp2Ip, cSimpleModule, 0);
    virtual void handleMessage(cMessage *msg);
};


Define_Module(Tcp2Ip);


void Tcp2Ip::handleMessage(cMessage *msg)
{
    // put the message into a TransportPacket instance
    TransportPacket *tpacket = new TransportPacket();
    (cMessage&)(*tpacket) = *msg;  // copy parameters, length, etc

    // set kind of the transport packet
    // this comes from tcpmodule and could be ACK_SEG, DATA etc.
    tpacket->setMsgKind(msg->kind());

    // set source and destination port
    tpacket->setSourcePort(msg->hasPar("src_port") ? (int)msg->par("src_port") : 255);
    tpacket->setDestinationPort(msg->hasPar("dest_port") ? (int)msg->par("dest_port") : 255);

    // add control info to tpacket
    IPControlInfo *controlInfo = new IPControlInfo();
    controlInfo->setDestAddr(IPAddress((int)msg->par("dest_addr")));
    controlInfo->setSrcAddr(IPAddress((int)msg->par("src_addr")));
    controlInfo->setProtocol(IP_PROT_TCP);
    tpacket->setControlInfo(controlInfo);

    // we don't set other values now
    delete msg;

    // send out to IP
    send(tpacket, "out");
}


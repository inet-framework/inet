//
// Copyright (C) 2004 Andras Varga
// Copyright (C) 2005 Christian Dankbar, Irene Ruengeler, Michael Tuexen
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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

// This file is based on the PPP.cc of INET written by Andras Varga.

#define WANT_WINSOCK2

#include <platdep/sockets.h>
#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include "InterfaceTable.h"
#include "InterfaceTableAccess.h"
#include "ExtInterface.h"
#include "IPSerializer.h"


Define_Module(ExtInterface);

void ExtInterface::initialize(int stage)
{
    // subscribe at scheduler for external messages
    if(stage == 0)
    {
        if(dynamic_cast<cSocketRTScheduler *>(simulation.getScheduler()) != NULL)
        {
            rtScheduler = check_and_cast<cSocketRTScheduler *>(simulation.getScheduler());
            //device = ev.config()->getAsString("Capture", "device", "lo0");
            device = par("device");
            //const char *filter = ev.config()->getAsString("Capture", "filter-string", "ip");
            const char *filter = par("filterString");
            rtScheduler->setInterfaceModule(this, device, filter);
            connected = true;
        }
        else
        {
            // this simulation run works without external interface..
            connected = false;
        }
    }

    if (stage == 3)
    {
        // update display string when addresses have been autoconfigured etc.
        updateDisplayString();
        return;
    }

    // all initialization is done in the first stage
    if (stage != 0)
        return;

    numSent = numRcvd = numDropped = 0;
    WATCH(numSent);
    WATCH(numRcvd);
    WATCH(numDropped);

    // register our interface entry in RoutingTable
    interfaceEntry = registerInterface();

    // if not connected, make it gray
    if (ev.isGUI() && !connected)
    {
        getDisplayString().setTagArg("i",1,"#707070");
        getDisplayString().setTagArg("i",2,"100");
    }
}

InterfaceEntry *ExtInterface::registerInterface()
{
    InterfaceEntry *e = new InterfaceEntry();

    // interface name: our module name without special characters ([])
    char *interfaceName = new char[strlen(getFullName())+1];
    char *d=interfaceName;
    for (const char *s=getFullName(); *s; s++)
    if (isalnum(*s))
        *d++ = *s;
    *d = '\0';
    e->setName(interfaceName);
    delete [] interfaceName;

    e->setMtu(par("mtu"));
    e->setMulticast(true);
    e->setPointToPoint(true);
    IInterfaceTable *ift = InterfaceTableAccess().get();
    ift->addInterface(e, this);
    return e;
}

void ExtInterface::handleMessage(cMessage *msg)
{

    if(dynamic_cast<ExtFrame *>(msg) != NULL)
    {
        // incoming real packet from wire (captured by pcap)
        uint32 packetLength;
        ExtFrame *rawPacket = check_and_cast<ExtFrame *>(msg);

        packetLength = rawPacket->getDataArraySize();
        for(uint32 i=0; i < packetLength; i++)
            buffer[i] = rawPacket->getData(i);

        IPDatagram *ipPacket = new IPDatagram("ip-from-wire");
        IPSerializer().parse(buffer, packetLength, (IPDatagram *)ipPacket);
        EV << "Delivering an IP packet from "
           << ipPacket->getSrcAddress()
           << " to "
           << ipPacket->getDestAddress()
           << " and length of"
           << ipPacket->getByteLength()
           << " bytes to IP layer.\n";
        send(ipPacket, "netwOut");
        numRcvd++;
    }
    else
    {
        memset(buffer, 0, 1<<16);
        IPDatagram *ipPacket = check_and_cast<IPDatagram *>(msg);

        if ((ipPacket->getTransportProtocol() != IP_PROT_ICMP) &&
            (ipPacket->getTransportProtocol() != IPPROTO_SCTP) &&
            (ipPacket->getTransportProtocol() != IPPROTO_TCP) &&
            (ipPacket->getTransportProtocol() != IPPROTO_UDP))
        {
            EV << "Can not send packet. Protocol " << ipPacket->getTransportProtocol() << " is not supported.\n";
            numDropped++;
            delete(msg);
            return;
        }

        if(connected)
        {
            struct sockaddr_in addr;
            addr.sin_family      = AF_INET;
#if !defined(linux) && !defined(_WIN32)
            addr.sin_len         = sizeof(struct sockaddr_in);
#endif
            addr.sin_port        = 0;
            addr.sin_addr.s_addr = htonl(ipPacket->getDestAddress().getInt());
            int32 packetLength = IPSerializer().serialize(ipPacket,buffer, sizeof(buffer));
            EV << "Delivering an IP packet from "
               << ipPacket->getSrcAddress()
               << " to "
               << ipPacket->getDestAddress()
               << " and length of "
               << ipPacket->getByteLength()
               << " bytes to link layer.\n";
            rtScheduler->sendBytes(buffer, packetLength, (struct sockaddr *) &addr, sizeof(struct sockaddr_in));
            numSent++;
        }
        else
        {
            EV << "Interface is not connected, dropping packet " << msg << endl;
            numDropped++;
        }
    }
    delete(msg);
    if (ev.isGUI())
        updateDisplayString();
}

void ExtInterface::displayBusy()
{
    getDisplayString().setTagArg("i",1, "yellow");
    gate("physOut")->getDisplayString().setTagArg("ls",0,"yellow");
    gate("physOut")->getDisplayString().setTagArg("ls",1,"3");
}

void ExtInterface::displayIdle()
{
    getDisplayString().setTagArg("i",1,"");
    gate("physOut")->getDisplayString().setTagArg("ls",0,"black");
    gate("physOut")->getDisplayString().setTagArg("ls",1,"1");
}

void ExtInterface::updateDisplayString()
{
    char buf[80];
    if (ev.disable_tracing)
        getDisplayString().setTagArg("t",0,"");
    if(connected)
        sprintf(buf, "pcap device: %s\nrcv:%d snt:%d", device, numRcvd, numSent);
    else
        sprintf(buf, "not connected");
    getDisplayString().setTagArg("t", 0, buf);
}

void ExtInterface::finish()
{
    std::cout << getFullPath() << ": " << numSent << " packets sent, " <<
            numRcvd << " packets received, " << numDropped <<" packets dropped.\n";
}


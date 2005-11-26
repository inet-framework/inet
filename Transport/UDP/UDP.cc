//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004,2005 Andras Varga
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
// Author: Jochen Reber
// Rewrite: Andras Varga 2004,2005
//

#include <omnetpp.h>
#include <string.h>
#include "UDPPacket.h"
#include "UDP.h"
#include "IPControlInfo_m.h"
#include "IPv6ControlInfo_m.h"
//#include "ICMPAccess.h"
//#include "ICMPv6Access.h"


Define_Module( UDP );

static std::ostream & operator<<(std::ostream & os, const UDP::SockDesc& sd)
{
    os << "sockId=" << sd.sockId;
    os << " appGateIndex=" << sd.appGateIndex;
    os << " localPort=" << sd.localPort;
    if (sd.remotePort!=0)
        os << " remotePort=" << sd.remotePort;
    if (!sd.localAddr.isUnspecified())
        os << " localAddr=" << sd.localAddr;
    if (!sd.remoteAddr.isUnspecified())
        os << " remoteAddr=" << sd.remoteAddr;
    if (sd.interfaceId!=-1)
        os << " interfaceId=" << sd.interfaceId;

    return os;
}

static std::ostream & operator<<(std::ostream & os, const UDP::SockDescList& list)
{
    for (UDP::SockDescList::const_iterator i=list.begin(); i!=list.end(); ++i)
        os << "sockId=" << (*i)->sockId << " ";
    return os;
}

//--------

UDP::~UDP()
{
    for (SocketsByIdMap::iterator i=socketsByIdMap.begin(); i!=socketsByIdMap.end(); ++i)
        delete i->second;
}

void UDP::initialize()
{
    WATCH_PTRMAP(socketsByIdMap);
    WATCH_MAP(socketsByPortMap);

    nextEphemeralPort = 1024;

    numSent = 0;
    numPassedUp = 0;
    numDroppedWrongPort = 0;
    numDroppedBadChecksum = 0;
    WATCH(numSent);
    WATCH(numPassedUp);
    WATCH(numDroppedWrongPort);
    WATCH(numDroppedBadChecksum);
}

void UDP::bind(int gateIndex, UDPControlInfo *ctrl)
{
    // XXX checks could be added, of when the bind should be allowed to proceed

    // create and fill in SockDesc
    SockDesc *sd = new SockDesc();
    sd->sockId = ctrl->sockId();
    sd->appGateIndex = gateIndex;
    sd->localAddr = ctrl->srcAddr();
    sd->remoteAddr = ctrl->destAddr();
    sd->localPort = ctrl->srcPort();
    sd->remotePort = ctrl->destPort();
    sd->interfaceId = ctrl->interfaceId();

    if (sd->localPort==0)
        sd->localPort = getEphemeralPort();

    sd->onlyLocalPortIsSet = sd->localAddr.isUnspecified() &&
                             sd->remoteAddr.isUnspecified() &&
                             sd->remotePort==0 &&
                             sd->interfaceId==-1;

    // add to socketsByIdMap
    ASSERT(socketsByIdMap.find(sd->sockId)==socketsByIdMap.end());
    socketsByIdMap[sd->sockId] = sd;

    // add to socketsByPortMap
    SockDescList& list = socketsByPortMap[sd->localPort]; // create if doesn't exist
    list.push_back(sd);
}

void UDP::unbind(int sockId)
{
    // remove from socketsByIdMap
    SocketsByIdMap::iterator it = socketsByIdMap.find(sockId);
    if (it==socketsByIdMap.end())
        error("socket id=%d doesn't exist (already closed?)", sockId);
    SockDesc *sd = it->second;
    socketsByIdMap.erase(it);

    // remove from socketsByPortMap
    SockDescList& list = socketsByPortMap[sd->localPort];
    for (SockDescList::iterator it=list.begin(); it!=list.end(); ++it)
        if (*it == sd)
            {list.erase(it); break;}

    delete sd;
}

short UDP::getEphemeralPort()
{
    if (nextEphemeralPort==5000)
        error("Ephemeral port range 1024..4999 exhausted (port number reuse not implemented)");
    return nextEphemeralPort++;
}

void UDP::handleMessage(cMessage *msg)
{
    // received from IP layer
    if (msg->arrivedOn("from_ip") || msg->arrivedOn("from_ipv6"))
    {
        processMsgFromIP(check_and_cast<UDPPacket *>(msg));
    }
    else // received from application layer
    {
        if (msg->kind()==UDP_C_DATA)
            processMsgFromApp(msg);
        else
            processCommandFromApp(msg);
    }

    if (ev.isGUI())
        updateDisplayString();
}

void UDP::updateDisplayString()
{
    char buf[80];
    sprintf(buf, "passed up: %d pks\nsent: %d pks", numPassedUp, numSent);
    if (numDroppedWrongPort>0)
    {
        sprintf(buf+strlen(buf), "\ndropped (no app): %d pks", numDroppedWrongPort);
        displayString().setTagArg("i",1,"red");
    }
    displayString().setTagArg("t",0,buf);
}

bool UDP::matchesSocket(UDPPacket *udp, IPControlInfo *ctrl, SockDesc *sd)
{
    // Note: OVERLOADED FUNCTION, IPv4 version!
    if (sd->remotePort!=0 && sd->remotePort!=udp->sourcePort())
        return false;
    if (!sd->localAddr.isUnspecified() && sd->localAddr.get4()!=ctrl->destAddr())
        return false;
    if (!sd->remoteAddr.isUnspecified() && sd->remoteAddr.get4()!=ctrl->srcAddr())
        return false;
    if (sd->interfaceId!=-1 && sd->interfaceId!=ctrl->interfaceId())
        return false;
    return true;
}

bool UDP::matchesSocket(UDPPacket *udp, IPv6ControlInfo *ctrl, SockDesc *sd)
{
    // Note: OVERLOADED FUNCTION, IPv6 VERSION!
    if (sd->remotePort!=0 && sd->remotePort!=udp->sourcePort())
        return false;
    if (!sd->localAddr.isUnspecified() && sd->localAddr.get6()!=ctrl->destAddr())
        return false;
    if (!sd->remoteAddr.isUnspecified() && sd->remoteAddr.get6()!=ctrl->srcAddr())
        return false;
    //if (sd->interfaceId!=-1 && sd->interfaceId!=ctrl->interfaceId()) FIXME IPv6 should fill in interfaceId!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //    return false;
    return true;
}

void UDP::sendUp(cMessage *payload, UDPPacket *udpHeader, IPControlInfo *ctrl, SockDesc *sd)
{
    // Note: OVERLOADED FUNCTION, IPv4 VERSION!
    // send payload with UDPControlInfo up to the application
    UDPControlInfo *udpControlInfo = new UDPControlInfo();
    udpControlInfo->setSockId(sd->sockId);
    udpControlInfo->setUserId(sd->userId);
    udpControlInfo->setSrcAddr(ctrl->srcAddr());
    udpControlInfo->setDestAddr(ctrl->destAddr());
    udpControlInfo->setSrcPort(udpHeader->sourcePort());
    udpControlInfo->setDestPort(udpHeader->destinationPort());
    udpControlInfo->setInterfaceId(ctrl->interfaceId());

    cMessage *copy = (cMessage *)payload->dup();
    copy->setControlInfo(copy);
    send(copy, "to_app", sd->appGateIndex);
    numPassedUp++;
}

void UDP::sendUp(cMessage *payload, UDPPacket *udpHeader, IPv6ControlInfo *ctrl, SockDesc *sd)
{
    // Note: OVERLOADED FUNCTION, IPv6 VERSION!
    UDPControlInfo *udpControlInfo = new UDPControlInfo();
    udpControlInfo->setSockId(sd->sockId);
    udpControlInfo->setUserId(sd->userId);
    udpControlInfo->setSrcAddr(ctrl->srcAddr());
    udpControlInfo->setDestAddr(ctrl->destAddr());
    udpControlInfo->setSrcPort(udpHeader->sourcePort());
    udpControlInfo->setDestPort(udpHeader->destinationPort());
    //udpControlInfo->setInterfaceId(ctrl->interfaceId());  FIXME add interfaceId to IPv6ControlInfo!!!

    cMessage *copy = (cMessage *)payload->dup();
    copy->setControlInfo(copy);
    send(copy, "to_app", sd->appGateIndex);
    numPassedUp++;
}

void UDP::processMsgFromIP(UDPPacket *udpPacket)
{
    // simulate checksum: discard packet if it has bit error
    if (udpPacket->hasBitError())
    {
        delete udpPacket;
        numDroppedBadChecksum++;
        return;
    }

    int destPort = udpPacket->destinationPort();

    // send back ICMP error if no socket is bound to that port
    SocketsByPortMap::iterator it = socketsByPortMap.find(destPort);
    if (it==socketsByPortMap.end())
    {
        delete udpPacket;
        numDroppedWrongPort++;
        // FIXME send back ICMP?
        return;
    }
    SockDescList& list = it->second;

    cPolymorphic *ctrl = udpPacket->removeControlInfo();
    cMessage *payload = udpPacket->decapsulate();
    int matches = 0;

    // deliver a copy of the packet to each matching socket
    if (dynamic_cast<IPControlInfo *>(ctrl)!=NULL)
    {
        IPControlInfo *ctrl4 = (IPControlInfo *)ctrl;
        for (SockDescList::iterator it=list.begin(); it!=list.end(); ++it)
        {
            SockDesc *sd = *it;
            if (sd->onlyLocalPortIsSet || matchesSocket(udpPacket, ctrl4, sd))
            {
                sendUp(payload, udpPacket, ctrl4, sd);
                matches++;
            }
        }
    }
    else if (dynamic_cast<IPv6ControlInfo *>(udpPacket->controlInfo())!=NULL)
    {
        IPv6ControlInfo *ctrl6 = (IPv6ControlInfo *)ctrl;
        for (SockDescList::iterator it=list.begin(); it!=list.end(); ++it)
        {
            SockDesc *sd = *it;
            if (sd->onlyLocalPortIsSet || matchesSocket(udpPacket, ctrl6, sd))
            {
                sendUp(payload, udpPacket, ctrl6, sd);
                matches++;
            }
        }
    }
    else
    {
        error("(%s)%s arrived from lower layer without control info", udpPacket->className(), udpPacket->name());
    }

    // send back ICMP error if there is no matching socket
    if (matches==0)
    {
        numDroppedWrongPort++;
        // FIXME send back ICMP?
    }

    delete payload;
    delete udpPacket;
    delete ctrl;
}


void UDP::processMsgFromApp(cMessage *appData)
{
    UDPControlInfo *udpControlInfo = check_and_cast<UDPControlInfo *>(appData->removeControlInfo());

    UDPPacket *udpPacket = new UDPPacket(appData->name());
    udpPacket->setByteLength(UDP_HEADER_BYTES);
    udpPacket->encapsulate(appData);

    // set source and destination port
    udpPacket->setSourcePort(udpControlInfo->srcPort());
    udpPacket->setDestinationPort(udpControlInfo->destPort());

    if (!udpControlInfo->destAddr().isIPv6())
    {
        // send to IPv4
        IPControlInfo *ipControlInfo = new IPControlInfo();
        ipControlInfo->setProtocol(IP_PROT_UDP);
        ipControlInfo->setSrcAddr(udpControlInfo->srcAddr().get4());
        ipControlInfo->setDestAddr(udpControlInfo->destAddr().get4());
        ipControlInfo->setInterfaceId(udpControlInfo->interfaceId());
        udpPacket->setControlInfo(ipControlInfo);
        delete udpControlInfo;

        send(udpPacket,"to_ip");
    }
    else
    {
        // send to IPv6
        IPv6ControlInfo *ipControlInfo = new IPv6ControlInfo();
        ipControlInfo->setProtocol(IP_PROT_UDP);
        ipControlInfo->setSrcAddr(udpControlInfo->srcAddr().get6());
        ipControlInfo->setDestAddr(udpControlInfo->destAddr().get6());
        // ipControlInfo->setInterfaceId(udpControlInfo->InterfaceId()); FIXME extend IPv6 with this!!!
        udpPacket->setControlInfo(ipControlInfo);
        delete udpControlInfo;

        send(udpPacket,"to_ipv6");
    }
    numSent++;
}

void UDP::processCommandFromApp(cMessage *msg)
{
    UDPControlInfo *udpControlInfo = check_and_cast<UDPControlInfo *>(msg->removeControlInfo());
    switch (msg->kind())
    {
        case UDP_C_BIND:
            bind(msg->arrivalGate()->index(), udpControlInfo);
            break;
        case UDP_C_UNBIND:
            unbind(udpControlInfo->sockId());
            break;
        default:
            error("unknown command code (message kind) %d received from app", msg->kind());
    }

    delete udpControlInfo;
    delete msg;
}



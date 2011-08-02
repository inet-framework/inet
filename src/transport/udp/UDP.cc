//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004-2011 Andras Varga
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


#include <string.h>
#include "UDP.h"
#include "UDPPacket.h"
#include "IInterfaceTable.h"
#include "InterfaceTableAccess.h"
#include "InterfaceEntry.h"

#ifdef WITH_IPv4
#include "ICMPAccess.h"
#include "ICMPMessage_m.h"
#include "IPv4ControlInfo.h"
#include "IPv4Datagram_m.h"
#include "IPv4InterfaceData.h"
#endif

#ifdef WITH_IPv6
#include "ICMPv6Access.h"
#include "ICMPv6Message_m.h"
#include "IPv6ControlInfo.h"
#include "IPv6Datagram_m.h"
#include "IPv6InterfaceData.h"
#endif


#define EPHEMERAL_PORTRANGE_START 1024
#define EPHEMERAL_PORTRANGE_END   5000


Define_Module(UDP);

simsignal_t UDP::rcvdPkSignal = SIMSIGNAL_NULL;
simsignal_t UDP::sentPkSignal = SIMSIGNAL_NULL;
simsignal_t UDP::passedUpPkSignal = SIMSIGNAL_NULL;
simsignal_t UDP::droppedPkWrongPortSignal = SIMSIGNAL_NULL;
simsignal_t UDP::droppedPkBadChecksumSignal = SIMSIGNAL_NULL;

static std::ostream & operator<<(std::ostream & os, const UDP::SockDesc& sd)
{
    os << "sockId=" << sd.sockId;
    os << " appGateIndex=" << sd.appGateIndex;
    os << " localPort=" << sd.localPort;
    if (sd.remotePort != -1)
        os << " remotePort=" << sd.remotePort;
    if (!sd.localAddr.isUnspecified())
        os << " localAddr=" << sd.localAddr;
    if (!sd.remoteAddr.isUnspecified())
        os << " remoteAddr=" << sd.remoteAddr;
    if (sd.multicastOutputInterfaceId!=-1)
        os << " interfaceId=" << sd.multicastOutputInterfaceId;

    return os;
}

static std::ostream & operator<<(std::ostream & os, const UDP::SockDescList& list)
{
    for (UDP::SockDescList::const_iterator i=list.begin(); i!=list.end(); ++i)
        os << "sockId=" << (*i)->sockId << " ";
    return os;
}

//--------

UDP::SockDesc::SockDesc(int sockId_, int appGateIndex_) {
    sockId = sockId_;
    appGateIndex = appGateIndex_;
    onlyLocalPortIsSet = false; // for now
    localPort = -1;
    remotePort = -1;
    isBroadcast = false;
    multicastOutputInterfaceId = -1;
    ttl = -1;
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

    lastEphemeralPort = EPHEMERAL_PORTRANGE_START;
    icmp = NULL;
    icmpv6 = NULL;

    numSent = 0;
    numPassedUp = 0;
    numDroppedWrongPort = 0;
    numDroppedBadChecksum = 0;
    WATCH(numSent);
    WATCH(numPassedUp);
    WATCH(numDroppedWrongPort);
    WATCH(numDroppedBadChecksum);
    rcvdPkSignal = registerSignal("rcvdPk");
    sentPkSignal = registerSignal("sentPk");
    passedUpPkSignal = registerSignal("passedUpPk");
    droppedPkWrongPortSignal = registerSignal("droppedPkWrongPort");
    droppedPkBadChecksumSignal = registerSignal("droppedPkBadChecksum");
}

void UDP::handleMessage(cMessage *msg)
{
    // received from IP layer
    if (msg->arrivedOn("ipIn") || msg->arrivedOn("ipv6In"))
    {
        if (dynamic_cast<UDPPacket *>(msg) != NULL)
            processUDPPacket((UDPPacket *)msg);
        else
            processICMPError(PK(msg));  // assume it's an ICMP error
    }
    else // received from application layer
    {
        if (msg->getKind()==UDP_C_DATA)
            processPacketFromApp(PK(msg));
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
    if (numDroppedWrongPort > 0)
    {
        sprintf(buf+strlen(buf), "\ndropped (no app): %d pks", numDroppedWrongPort);
        getDisplayString().setTagArg("i", 1, "red");
    }
    getDisplayString().setTagArg("t", 0, buf);
}

void UDP::processCommandFromApp(cMessage *msg)
{
    switch (msg->getKind())
    {
        case UDP_C_BIND: {
            UDPBindCommand *ctrl = check_and_cast<UDPBindCommand*>(msg->getControlInfo());
            bind(ctrl->getSockId(), msg->getArrivalGate()->getIndex(), ctrl->getLocalAddr(), ctrl->getLocalPort());
            break;
        }
        case UDP_C_CONNECT: {
            UDPConnectCommand *ctrl = check_and_cast<UDPConnectCommand*>(msg->getControlInfo());
            connect(ctrl->getSockId(), msg->getArrivalGate()->getIndex(), ctrl->getRemoteAddr(), ctrl->getRemotePort());
            break;
        }
        case UDP_C_CLOSE: {
            UDPCloseCommand *ctrl = check_and_cast<UDPCloseCommand*>(msg->getControlInfo());
            close(ctrl->getSockId());
            break;
        }
        case UDP_C_SETOPTION: {
            UDPSetOptionCommand *ctrl = check_and_cast<UDPSetOptionCommand *>(msg->getControlInfo());
            if (dynamic_cast<UDPSetTimeToLiveCommand*>(ctrl))
                setTimeToLive(ctrl->getSockId(), ((UDPSetTimeToLiveCommand*)ctrl)->getTtl());
            else if (dynamic_cast<UDPSetBroadcastCommand*>(ctrl))
                setTimeToLive(ctrl->getSockId(), ((UDPSetBroadcastCommand*)ctrl)->getBroadcast());
            else if (dynamic_cast<UDPSetMulticastInterfaceCommand*>(ctrl))
                setMulticastOutputInterface(ctrl->getSockId(), ((UDPSetMulticastInterfaceCommand*)ctrl)->getInterfaceId());
            else if (dynamic_cast<UDPJoinMulticastGroupCommand*>(ctrl))
                joinMulticastGroup(ctrl->getSockId(), ((UDPJoinMulticastGroupCommand*)ctrl)->getMulticastAddr(), ((UDPJoinMulticastGroupCommand*)ctrl)->getInterfaceId());
            else if (dynamic_cast<UDPLeaveMulticastGroupCommand*>(ctrl))
                leaveMulticastGroup(ctrl->getSockId(), ((UDPLeaveMulticastGroupCommand*)ctrl)->getMulticastAddr());
            else
                throw cRuntimeError("unknown subclass of UDPSetOptionCommand received from app: %s", ctrl->getClassName());
            break;
        }
        default: {
            throw cRuntimeError("unknown command code (message kind) %d received from app", msg->getKind());
        }
    }

    delete msg; // also deletes control info in it
}

void UDP::processPacketFromApp(cPacket *appData)
{
    UDPSendCommand *ctrl = check_and_cast<UDPSendCommand *>(appData->removeControlInfo());

    SocketsByIdMap::iterator it = socketsByIdMap.find(ctrl->getSockId());
    if (it == socketsByIdMap.end())
        error("send: no socket with sockId=%d", ctrl->getSockId());

    SockDesc *sd = it->second;
    const IPvXAddress& destAddr = ctrl->getDestAddr().isUnspecified() ? sd->remoteAddr : ctrl->getDestAddr();
    ushort destPort = ctrl->getDestPort() == -1 ? sd->remotePort : ctrl->getDestPort();
    if (destAddr.isUnspecified() || destPort == -1)
        error("send: missing destination address or port when sending over unconnected port");

    int interfaceId = -1;
    if (destAddr.isMulticast())
    {
        std::map<IPvXAddress,int>::iterator it = sd->multicastAddrs.find(destAddr);
        interfaceId = (it != sd->multicastAddrs.end() && it->second != -1) ? it->second : sd->multicastOutputInterfaceId;
    }
    sendDown(appData, sd->localAddr, sd->localPort, destAddr, destPort, interfaceId, sd->ttl);
    delete ctrl; // cannot be deleted earlier, due to destAddr
}

void UDP::processUDPPacket(UDPPacket *udpPacket)
{
    emit(rcvdPkSignal, udpPacket);

    // simulate checksum: discard packet if it has bit error
    EV << "Packet " << udpPacket->getName() << " received from network, dest port " << udpPacket->getDestinationPort() << "\n";

    if (udpPacket->hasBitError())
    {
        EV << "Packet has bit error, discarding\n";
        emit(droppedPkBadChecksumSignal, udpPacket);
        numDroppedBadChecksum++;
        delete udpPacket;

        return;
    }

    IPvXAddress srcAddr;
    IPvXAddress destAddr;
    bool isMulticast, isBroadcast;
    int srcPort = udpPacket->getSourcePort();
    int destPort = udpPacket->getDestinationPort();
    int interfaceId;
    int ttl;

    cObject *ctrl = udpPacket->removeControlInfo();
#ifdef WITH_IPv4
    if (dynamic_cast<IPv4ControlInfo *>(ctrl)!=NULL)
    {
        IPv4ControlInfo *ctrl4 = (IPv4ControlInfo *)ctrl;
        srcAddr = ctrl4->getSrcAddr();
        destAddr = ctrl4->getDestAddr();
        interfaceId = ctrl4->getInterfaceId();
        ttl = ctrl4->getTimeToLive();
        isMulticast = ctrl4->getDestAddr().isMulticast();
        isBroadcast = ctrl4->getDestAddr() == IPv4Address::ALLONES_ADDRESS;  // note: we cannot recognize other broadcast addresses (where the host part is all-ones), because here we don't know the netmask
    }
    else
#endif
#ifdef WITH_IPv6
    if (dynamic_cast<IPv6ControlInfo *>(ctrl)!=NULL)
    {
        IPv6ControlInfo *ctrl6 = (IPv6ControlInfo *)ctrl;
        srcAddr = ctrl6->getSrcAddr();
        destAddr = ctrl6->getDestAddr();
        interfaceId = ctrl6->getInterfaceId();
        ttl = ctrl6->getHopLimit();
        isMulticast = ctrl6->getDestAddr().isMulticast();
        isBroadcast = false;  // IPv6 has no broadcast, just various multicasts
    }
    else
#endif
    if (ctrl == NULL)
    {
        error("(%s)%s arrived from lower layer without control info",
                udpPacket->getClassName(), udpPacket->getName());
    }
    else
    {
        error("(%s)%s arrived from lower layer with unrecognized control info %s",
                udpPacket->getClassName(), udpPacket->getName(), ctrl->getClassName());
    }

    cPacket *payload = udpPacket->decapsulate();

    if (!isMulticast && !isBroadcast)
    {
        // unicast packet, there must be only one socket listening
        SockDesc *sd = findSocketForUnicastPacket(destAddr, destPort, srcAddr, srcPort);
        if (!sd)
        {
            EV << "No socket registered on port " << destPort << "\n";
            processUndeliverablePacket(udpPacket, ctrl);
            return;
        }
        else
        {
            sendUp(payload, sd, srcAddr, srcPort, destAddr, destPort, interfaceId, ttl);
            delete udpPacket;
            delete ctrl;
        }
    }
    else
    {
        // multicast packet: find all matching sockets, and send up a copy to each
        std::vector<SockDesc*> sds = findSocketsForMcastBcastPacket(destAddr, destPort, srcAddr, srcPort, isMulticast, isBroadcast);
        if (sds.empty())
        {
            EV << "No socket registered on port " << destPort << "\n";
            processUndeliverablePacket(udpPacket, ctrl);
            return;
        }
        else
        {
            int i;
            for (i = 0; i < sds.size()-1; i++)
                sendUp(payload->dup(), sds[i], srcAddr, srcPort, destAddr, destPort, interfaceId, ttl); // dup() to all but the last one
            sendUp(payload, sds[i], srcAddr, srcPort, destAddr, destPort, interfaceId, ttl);  // send original to last socket
            delete udpPacket;
            delete ctrl;
        }

    }
}

void UDP::processICMPError(cPacket *pk)
{
    // extract details from the error message, then try to notify socket that sent bogus packet
    int type, code;
    IPvXAddress localAddr, remoteAddr;
    ushort localPort, remotePort;

#ifdef WITH_IPv4
    if (dynamic_cast<ICMPMessage *>(pk))
    {
        ICMPMessage *icmpMsg = (ICMPMessage *)pk;
        type = icmpMsg->getType();
        code = icmpMsg->getCode();
        // Note: we must NOT use decapsulate() because payload in ICMP is conceptually truncated
        IPv4Datagram *datagram = check_and_cast<IPv4Datagram *>(icmpMsg->getEncapsulatedPacket());
        UDPPacket *packet = check_and_cast<UDPPacket *>(datagram->getEncapsulatedPacket());
        localAddr = datagram->getSrcAddress();
        remoteAddr = datagram->getDestAddress();
        localPort = packet->getSourcePort();
        remotePort = packet->getDestinationPort();
        delete icmpMsg;
    }
    else
#endif
#ifdef WITH_IPv6
    if (dynamic_cast<ICMPv6Message *>(pk))
    {
        ICMPv6Message *icmpMsg = (ICMPv6Message *)pk;
        type = icmpMsg->getType();
        code = -1; // FIXME this is dependent on getType()...
        // Note: we must NOT use decapsulate() because payload in ICMP is conceptually truncated
        IPv6Datagram *datagram = check_and_cast<IPv6Datagram *>(icmpMsg->getEncapsulatedPacket());
        UDPPacket *packet = check_and_cast<UDPPacket *>(datagram->getEncapsulatedPacket());
        localAddr = datagram->getSrcAddress();
        remoteAddr = datagram->getDestAddress();
        localPort = packet->getSourcePort();
        remotePort = packet->getDestinationPort();
        delete icmpMsg;
    }
    else
#endif
    {
        throw cRuntimeError("Unrecognized packet (%s)%s: not an ICMP error message", pk->getClassName(), pk->getName());
    }

    EV << "ICMP error received: type=" << type << " code=" << code
       << " about packet " << localAddr << ":" << localPort << " > "
       << remoteAddr << ":" << remotePort << "\n";

    // identify socket and report error to it
    SockDesc *sd = findSocketByLocalAddress(localAddr, localPort);
    if (!sd)
    {
        EV << "No socket on that local port, ignoring ICMP error\n";
        return;
    }

    // send UDP_I_ERROR to socket
    EV << "Source socket is sockId=" << sd->sockId << ", notifying.\n";
    sendUpErrorIndication(sd, localAddr, localPort, remoteAddr, remotePort);
}

void UDP::processUndeliverablePacket(UDPPacket *udpPacket, cObject *ctrl)
{
    emit(droppedPkWrongPortSignal, udpPacket);
    numDroppedWrongPort++;

    // send back ICMP PORT_UNREACHABLE
#ifdef WITH_IPv4
    if (dynamic_cast<IPv4ControlInfo *>(ctrl) != NULL)
    {
        if (!icmp)
            icmp = ICMPAccess().get();

        IPv4ControlInfo *ctrl4 = (IPv4ControlInfo *)ctrl;

        if (!ctrl4->getDestAddr().isMulticast())
            icmp->sendErrorMessage(udpPacket, ctrl4, ICMP_DESTINATION_UNREACHABLE, ICMP_DU_PORT_UNREACHABLE);
    }
    else
#endif
#ifdef WITH_IPv6
    if (dynamic_cast<IPv6ControlInfo *>(udpPacket->getControlInfo()) != NULL)
    {
        if (!icmpv6)
            icmpv6 = ICMPv6Access().get();

        IPv6ControlInfo *ctrl6 = (IPv6ControlInfo *)ctrl;

        if (!ctrl6->getDestAddr().isMulticast())
            icmpv6->sendErrorMessage(udpPacket, ctrl6, ICMPv6_DESTINATION_UNREACHABLE, PORT_UNREACHABLE);
    }
    else
#endif
    {
        error("(%s)%s arrived from lower layer without control info", udpPacket->getClassName(), udpPacket->getName()); //FIXME rather: with unrecognized control info
    }
}

void UDP::bind(int sockId, int gateIndex, const IPvXAddress& localAddr, int localPort)
{
    createSocket(sockId, gateIndex, localAddr, localPort);
}

void UDP::connect(int sockId, int gateIndex, const IPvXAddress& remoteAddr, int remotePort)
{
    if (remoteAddr.isUnspecified())
        error("connect: unspecified remote address");
    if (remotePort<=0 || remotePort>65535)
        error("connect: invalid remote port number %d", remotePort);

    SocketsByIdMap::iterator it = socketsByIdMap.find(sockId);
    SockDesc *sd;
    if (it != socketsByIdMap.end())
        sd = it->second;
    else
        sd = createSocket(sockId, gateIndex, IPvXAddress(), getEphemeralPort());

    sd->remoteAddr = remoteAddr;
    sd->remotePort = remotePort;
    sd->onlyLocalPortIsSet = false;

    EV << "Socket connected: " << *sd << "\n";
}

UDP::SockDesc *UDP::createSocket(int sockId, int gateIndex, const IPvXAddress& localAddr, int localPort)
{
    // validate sockId
    if (sockId == -1)
        error("sockId in BIND or CONNECT message not filled in");
    if (socketsByIdMap.find(sockId) != socketsByIdMap.end())
        error("Cannot create socket: sockId=%d is not unique (already taken)", sockId);

    if (localPort<-1 || localPort>65535) // -1: ephemeral port
        error("connect: invalid local port number %d", localPort);

    // do not allow two apps to bind to the same address/port combination
    SockDesc *existing = findSocketByLocalAddress(localAddr, localPort);
    if (existing != NULL)
        error("bind: local address/port %s:%u already taken", localAddr.str().c_str(), localPort);

    // create and fill in SockDesc
    SockDesc *sd = new SockDesc(sockId, gateIndex);
    sd->localAddr = localAddr;
    sd->localPort = localPort == -1 ? getEphemeralPort() : localPort;
    sd->onlyLocalPortIsSet = sd->localAddr.isUnspecified();

    // add to socketsByIdMap
    socketsByIdMap[sockId] = sd;

    // add to socketsByPortMap
    SockDescList& list = socketsByPortMap[sd->localPort]; // create if doesn't exist
    list.push_back(sd);

    EV << "Socket created: " << *sd << "\n";
    return sd;
}

void UDP::close(int sockId)
{
    // remove from socketsByIdMap
    SocketsByIdMap::iterator it = socketsByIdMap.find(sockId);
    if (it==socketsByIdMap.end())
        error("socket id=%d doesn't exist (already closed?)", sockId);
    SockDesc *sd = it->second;
    socketsByIdMap.erase(it);

    EV << "Closing socket: " << *sd << "\n";

    // remove from socketsByPortMap
    SockDescList& list = socketsByPortMap[sd->localPort];
    for (SockDescList::iterator it = list.begin(); it != list.end(); ++it)
        if (*it == sd)
            {list.erase(it); break;}
    if (list.empty())
        socketsByPortMap.erase(sd->localPort);
    delete sd;
}

ushort UDP::getEphemeralPort()
{
    // start at the last allocated port number + 1, and search for an unused one
    ushort searchUntil = lastEphemeralPort++;
    if (lastEphemeralPort == EPHEMERAL_PORTRANGE_END) // wrap
        lastEphemeralPort = EPHEMERAL_PORTRANGE_START;

    while (socketsByPortMap.find(lastEphemeralPort) != socketsByPortMap.end())
    {
        if (lastEphemeralPort == searchUntil) // got back to starting point?
            error("Ephemeral port range %d..%d exhausted, all ports occupied", EPHEMERAL_PORTRANGE_START, EPHEMERAL_PORTRANGE_END);
        lastEphemeralPort++;
        if (lastEphemeralPort == EPHEMERAL_PORTRANGE_END) // wrap
            lastEphemeralPort = EPHEMERAL_PORTRANGE_START;
    }

    // found a free one, return it
    return lastEphemeralPort;
}

UDP::SockDesc *UDP::findSocketByLocalAddress(const IPvXAddress& localAddr, ushort localPort)
{
    SocketsByPortMap::iterator it = socketsByPortMap.find(localPort);
    if (it == socketsByPortMap.end())
        return NULL;

    SockDescList& list = it->second;
    for (SockDescList::iterator it = list.begin(); it != list.end(); ++it)
    {
        SockDesc *sd = *it;
        if (sd->localAddr.isUnspecified() || sd->localAddr == localAddr)
            return sd;
    }
    return NULL;
}

UDP::SockDesc *UDP::findSocketForUnicastPacket(const IPvXAddress& localAddr, ushort localPort, const IPvXAddress& remoteAddr, ushort remotePort)
{
    SocketsByPortMap::iterator it = socketsByPortMap.find(localPort);
    if (it == socketsByPortMap.end())
        return NULL;

    SockDescList& list = it->second;
    for (SockDescList::iterator it = list.begin(); it != list.end(); ++it)
    {
        SockDesc *sd = *it;
        if (sd->onlyLocalPortIsSet || (
                (sd->remotePort == -1 || sd->remotePort == remotePort) &&
                (sd->localAddr.isUnspecified() || sd->localAddr == localAddr) &&
                (sd->remoteAddr.isUnspecified() || sd->remoteAddr == remoteAddr)
        ))
            return sd;
    }
    return NULL;
}

std::vector<UDP::SockDesc*> UDP::findSocketsForMcastBcastPacket(const IPvXAddress& localAddr, ushort localPort, const IPvXAddress& remoteAddr, ushort remotePort, bool isMulticast, bool isBroadcast)
{
    ASSERT(isMulticast || isBroadcast);
    std::vector<SockDesc*> result;
    SocketsByPortMap::iterator it = socketsByPortMap.find(localPort);
    if (it == socketsByPortMap.end())
        return result;

    SockDescList& list = it->second;
    for (SockDescList::iterator it = list.begin(); it != list.end(); ++it)
    {
        SockDesc *sd = *it;
        if (isBroadcast)
        {
            if (sd->isBroadcast)
            {
                if ((sd->remotePort == -1 || sd->remotePort == remotePort) &&
                    (sd->remoteAddr.isUnspecified() || sd->remoteAddr == remoteAddr))
                    result.push_back(sd);
            }
        }
        else if (isMulticast)
        {
            if (sd->multicastAddrs.find(localAddr) != sd->multicastAddrs.end())
            {
                if ((sd->remotePort == -1 || sd->remotePort == remotePort) &&
                    (sd->remoteAddr.isUnspecified() || sd->remoteAddr == remoteAddr))
                    result.push_back(sd);
            }
        }
    }
    return result;
}

void UDP::sendUp(cPacket *payload, SockDesc *sd, const IPvXAddress& srcAddr, ushort srcPort, const IPvXAddress& destAddr, ushort destPort, int interfaceId, int ttl)
{
    EV << "Sending payload up to socket sockId=" << sd->sockId << "\n";

    // send payload with UDPControlInfo up to the application
    UDPDataIndication *udpCtrl = new UDPDataIndication();
    udpCtrl->setSockId(sd->sockId);
    udpCtrl->setSrcAddr(srcAddr);
    udpCtrl->setDestAddr(destAddr);
    udpCtrl->setSrcPort(srcPort);
    udpCtrl->setDestPort(destPort);
    udpCtrl->setInterfaceId(interfaceId);
    udpCtrl->setTtl(ttl);
    payload->setControlInfo(udpCtrl);
    payload->setKind(UDP_I_DATA);

    emit(passedUpPkSignal, payload);
    send(payload, "appOut", sd->appGateIndex);
    numPassedUp++;
}

void UDP::sendUpErrorIndication(SockDesc *sd, const IPvXAddress& localAddr, ushort localPort, const IPvXAddress& remoteAddr, ushort remotePort)
{
    cMessage *notifyMsg = new cMessage("ERROR", UDP_I_ERROR);
    UDPErrorIndication *udpCtrl = new UDPErrorIndication();
    udpCtrl->setSockId(sd->sockId);
    udpCtrl->setSrcAddr(localAddr);
    udpCtrl->setDestAddr(remoteAddr);
    udpCtrl->setSrcPort(sd->localPort);
    udpCtrl->setDestPort(remotePort);
    notifyMsg->setControlInfo(udpCtrl);

    send(notifyMsg, "appOut", sd->appGateIndex);
}

void UDP::sendDown(cPacket *appData, const IPvXAddress& srcAddr, ushort srcPort, const IPvXAddress& destAddr, ushort destPort, int interfaceId, int ttl)
{
    if (destAddr.isUnspecified())
        error("send: unspecified destination address");
    if (destPort<=0 || destPort>65535)
        error("send invalid remote port number %d", destPort);

    UDPPacket *udpPacket = createUDPPacket(appData->getName());
    udpPacket->setByteLength(UDP_HEADER_BYTES);
    udpPacket->encapsulate(appData);

    // set source and destination port
    udpPacket->setSourcePort(srcPort);
    udpPacket->setDestinationPort(destPort);

    if (!destAddr.isIPv6())
    {
#ifdef WITH_IPv4
        // send to IPv4
        EV << "Sending app packet " << appData->getName() << " over IPv4.\n";
        IPv4ControlInfo *ipControlInfo = new IPv4ControlInfo();
        ipControlInfo->setProtocol(IP_PROT_UDP);
        ipControlInfo->setSrcAddr(srcAddr.get4());
        ipControlInfo->setDestAddr(destAddr.get4());
        ipControlInfo->setInterfaceId(interfaceId);
        ipControlInfo->setTimeToLive(ttl);
        udpPacket->setControlInfo(ipControlInfo);

        emit(sentPkSignal, udpPacket);
        send(udpPacket, "ipOut");
#else
        throw cRuntimeError("Cannot send packet over IPv4: INET compiled without the IPv4 feature");
#endif
    }
    else
    {
#ifdef WITH_IPv6
        // send to IPv6
        EV << "Sending app packet " << appData->getName() << " over IPv6.\n";
        IPv6ControlInfo *ipControlInfo = new IPv6ControlInfo();
        ipControlInfo->setProtocol(IP_PROT_UDP);
        ipControlInfo->setSrcAddr(srcAddr.get6());
        ipControlInfo->setDestAddr(destAddr.get6());
        ipControlInfo->setInterfaceId(interfaceId);
        ipControlInfo->setHopLimit(ttl);
        udpPacket->setControlInfo(ipControlInfo);

        emit(sentPkSignal, udpPacket);
        send(udpPacket, "ipv6Out");
#else
        throw cRuntimeError("Cannot send packet over IPv6: INET compiled without the IPv6 feature");
#endif
    }
    numSent++;
}

UDPPacket *UDP::createUDPPacket(const char *name)
{
    return new UDPPacket(name);
}

UDP::SockDesc *UDP::getSocketById(int sockId)
{
    SocketsByIdMap::iterator it = socketsByIdMap.find(sockId);
    if (it==socketsByIdMap.end())
        error("socket id=%d doesn't exist (already closed?)", sockId);
    return it->second;
}

void UDP::setTimeToLive(int sockId, int ttl)
{
    SockDesc *sd = getSocketById(sockId);
    sd->ttl = ttl;
}

void UDP::setBroadcast(int sockId, bool broadcast)
{
    SockDesc *sd = getSocketById(sockId);
    sd->isBroadcast = broadcast;
}

void UDP::setMulticastOutputInterface(int sockId, int interfaceId)
{
    SockDesc *sd = getSocketById(sockId);
    sd->multicastOutputInterfaceId = interfaceId;
}

void UDP::joinMulticastGroup(int sockId, const IPvXAddress& multicastAddr, int interfaceId)
{
    SockDesc *sd = getSocketById(sockId);
    ASSERT(multicastAddr.isMulticast());
    sd->multicastAddrs[multicastAddr] = interfaceId;

    // add the multicast address to the selected interface or all interfaces
    IInterfaceTable *ift = InterfaceTableAccess().get(this);
    if (interfaceId != -1)
    {
        InterfaceEntry *ie = ift->getInterfaceById(interfaceId);
        if (!ie)
            error("Interface id=%d does not exist", interfaceId);
        addMulticastAddressToInterface(ie, multicastAddr);
    }
    else
    {
        int n = ift->getNumInterfaces();
        for (int i = 0; i < n; i++)
            addMulticastAddressToInterface(ift->getInterface(i), multicastAddr);
    }
}

void UDP::addMulticastAddressToInterface(InterfaceEntry *ie, const IPvXAddress& multicastAddr)
{
    if (!multicastAddr.isIPv6())
    {
#ifdef WITH_IPv4
        ie->ipv4Data()->joinMulticastGroup(multicastAddr.get4());
#endif
    }
    else
    {
#ifdef WITH_IPv6
        ie->ipv6Data()->assignAddress(multicastAddr.get6(), false, SimTime::getMaxTime(), SimTime::getMaxTime());
#endif
    }
}

void UDP::leaveMulticastGroup(int sockId, const IPvXAddress& multicastAddr)
{
    SockDesc *sd = getSocketById(sockId);
    sd->multicastAddrs.erase(multicastAddr);
    // note: we cannot remove the address from the interface, because someone else may still use it
}


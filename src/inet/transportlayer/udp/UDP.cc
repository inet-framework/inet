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

#include <algorithm>
#include <string>
#include "inet/transportlayer/udp/UDP.h"
#include "inet/transportlayer/udp/UDPPacket.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/common/InterfaceEntry.h"
#include "inet/networklayer/common/IPSocket.h"
#include "inet/networklayer/contract/ipv4/IPv4ControlInfo.h"
#include "inet/networklayer/contract/ipv6/IPv6ControlInfo.h"
#include "inet/networklayer/contract/generic/GenericNetworkProtocolControlInfo.h"
#include "inet/networklayer/contract/IL3AddressType.h"
#include "inet/common/ModuleAccess.h"

#ifdef WITH_IPv4
#include "inet/networklayer/ipv4/ICMPMessage.h"
#include "inet/networklayer/ipv4/IPv4Datagram.h"
#include "inet/networklayer/ipv4/IPv4InterfaceData.h"
#include "inet/networklayer/ipv4/ICMP.h"
#endif // ifdef WITH_IPv4

#ifdef WITH_IPv6
#include "inet/networklayer/icmpv6/ICMPv6Message_m.h"
#include "inet/networklayer/ipv6/IPv6Datagram.h"
#include "inet/networklayer/ipv6/IPv6InterfaceData.h"
#include "inet/networklayer/icmpv6/ICMPv6.h"
#endif // ifdef WITH_IPv6

#include "inet/common/lifecycle/NodeOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"

namespace inet {

Define_Module(UDP);

simsignal_t UDP::rcvdPkSignal = registerSignal("rcvdPk");
simsignal_t UDP::sentPkSignal = registerSignal("sentPk");
simsignal_t UDP::passedUpPkSignal = registerSignal("passedUpPk");
simsignal_t UDP::droppedPkWrongPortSignal = registerSignal("droppedPkWrongPort");
simsignal_t UDP::droppedPkBadChecksumSignal = registerSignal("droppedPkBadChecksum");

bool UDP::MulticastMembership::isSourceAllowed(L3Address sourceAddr)
{
    auto it = std::find(sourceList.begin(), sourceList.end(), sourceAddr);
    return (filterMode == UDP_INCLUDE_MCAST_SOURCES && it != sourceList.end()) ||
           (filterMode == UDP_EXCLUDE_MCAST_SOURCES && it == sourceList.end());
}

static std::ostream& operator<<(std::ostream& os, const UDP::SockDesc& sd)
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
    if (sd.multicastOutputInterfaceId != -1)
        os << " interfaceId=" << sd.multicastOutputInterfaceId;
    if (sd.multicastLoop != DEFAULT_MULTICAST_LOOP)
        os << " multicastLoop=" << sd.multicastLoop;

    return os;
}

static std::ostream& operator<<(std::ostream& os, const UDP::SockDescList& list)
{
    for (UDP::SockDescList::const_iterator i = list.begin(); i != list.end(); ++i)
        os << "sockId=" << (*i)->sockId << " ";
    return os;
}

//--------

UDP::SockDesc::SockDesc(int sockId_, int appGateIndex_)
{
    sockId = sockId_;
    appGateIndex = appGateIndex_;
}

UDP::SockDesc::~SockDesc()
{
    for(auto it = multicastMembershipTable.begin();
            it != multicastMembershipTable.end();
            ++it)
        delete (*it);
}

//--------
UDP::UDP()
{
}

UDP::~UDP()
{
    clearAllSockets();
}

void UDP::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        WATCH_PTRMAP(socketsByIdMap);
        WATCH_MAP(socketsByPortMap);

        lastEphemeralPort = EPHEMERAL_PORTRANGE_START;
        ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        icmp = nullptr;
        icmpv6 = nullptr;

        numSent = 0;
        numPassedUp = 0;
        numDroppedWrongPort = 0;
        numDroppedBadChecksum = 0;
        WATCH(numSent);
        WATCH(numPassedUp);
        WATCH(numDroppedWrongPort);
        WATCH(numDroppedBadChecksum);

        isOperational = false;
    }
    else if (stage == INITSTAGE_TRANSPORT_LAYER) {
        IPSocket ipSocket(gate("ipOut"));
        ipSocket.registerProtocol(IP_PROT_UDP);

        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
        isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;
    }
}

void UDP::handleMessage(cMessage *msg)
{
    if (!isOperational)
        throw cRuntimeError("Message '%s' received when UDP is OFF", msg->getName());

    // received from IP layer
    if (msg->arrivedOn("ipIn")) {
        if (dynamic_cast<UDPPacket *>(msg) != nullptr)
            processUDPPacket((UDPPacket *)msg);
        else
            processICMPError(PK(msg)); // assume it's an ICMP error
    }
    else {    // received from application layer
        if (msg->getKind() == UDP_C_DATA)
            processPacketFromApp(PK(msg));
        else
            processCommandFromApp(msg);
    }

    if (hasGUI())
        updateDisplayString();
}

void UDP::updateDisplayString()
{
    char buf[80];
    sprintf(buf, "passed up: %d pks\nsent: %d pks", numPassedUp, numSent);
    if (numDroppedWrongPort > 0) {
        sprintf(buf + strlen(buf), "\ndropped (no app): %d pks", numDroppedWrongPort);
        getDisplayString().setTagArg("i", 1, "red");
    }
    getDisplayString().setTagArg("t", 0, buf);
}

void UDP::processCommandFromApp(cMessage *msg)
{
    switch (msg->getKind()) {
        case UDP_C_BIND: {
            UDPBindCommand *ctrl = check_and_cast<UDPBindCommand *>(msg->getControlInfo());
            bind(ctrl->getSockId(), msg->getArrivalGate()->getIndex(), ctrl->getLocalAddr(), ctrl->getLocalPort());
            break;
        }

        case UDP_C_CONNECT: {
            UDPConnectCommand *ctrl = check_and_cast<UDPConnectCommand *>(msg->getControlInfo());
            connect(ctrl->getSockId(), msg->getArrivalGate()->getIndex(), ctrl->getRemoteAddr(), ctrl->getRemotePort());
            break;
        }

        case UDP_C_CLOSE: {
            UDPCloseCommand *ctrl = check_and_cast<UDPCloseCommand *>(msg->getControlInfo());
            close(ctrl->getSockId());
            break;
        }

        case UDP_C_SETOPTION: {
            UDPSetOptionCommand *ctrl = check_and_cast<UDPSetOptionCommand *>(msg->getControlInfo());
            SockDesc *sd = getOrCreateSocket(ctrl->getSockId(), msg->getArrivalGate()->getIndex());

            if (dynamic_cast<UDPSetTimeToLiveCommand *>(ctrl))
                setTimeToLive(sd, ((UDPSetTimeToLiveCommand *)ctrl)->getTtl());
            else if (dynamic_cast<UDPSetTypeOfServiceCommand *>(ctrl))
                setTypeOfService(sd, ((UDPSetTypeOfServiceCommand *)ctrl)->getTos());
            else if (dynamic_cast<UDPSetBroadcastCommand *>(ctrl))
                setBroadcast(sd, ((UDPSetBroadcastCommand *)ctrl)->getBroadcast());
            else if (dynamic_cast<UDPSetMulticastInterfaceCommand *>(ctrl))
                setMulticastOutputInterface(sd, ((UDPSetMulticastInterfaceCommand *)ctrl)->getInterfaceId());
            else if (dynamic_cast<UDPSetMulticastLoopCommand *>(ctrl))
                setMulticastLoop(sd, ((UDPSetMulticastLoopCommand *)ctrl)->getLoop());
            else if (dynamic_cast<UDPSetReuseAddressCommand *>(ctrl))
                setReuseAddress(sd, ((UDPSetReuseAddressCommand *)ctrl)->getReuseAddress());
            else if (dynamic_cast<UDPJoinMulticastGroupsCommand *>(ctrl)) {
                UDPJoinMulticastGroupsCommand *cmd = (UDPJoinMulticastGroupsCommand *)ctrl;
                std::vector<L3Address> addresses;
                std::vector<int> interfaceIds;
                for (int i = 0; i < (int)cmd->getMulticastAddrArraySize(); i++)
                    addresses.push_back(cmd->getMulticastAddr(i));
                for (int i = 0; i < (int)cmd->getInterfaceIdArraySize(); i++)
                    interfaceIds.push_back(cmd->getInterfaceId(i));
                joinMulticastGroups(sd, addresses, interfaceIds);
            }
            else if (dynamic_cast<UDPLeaveMulticastGroupsCommand *>(ctrl)) {
                UDPLeaveMulticastGroupsCommand *cmd = (UDPLeaveMulticastGroupsCommand *)ctrl;
                std::vector<L3Address> addresses;
                for (int i = 0; i < (int)cmd->getMulticastAddrArraySize(); i++)
                    addresses.push_back(cmd->getMulticastAddr(i));
                leaveMulticastGroups(sd, addresses);
            }
            else if (dynamic_cast<UDPBlockMulticastSourcesCommand *>(ctrl)) {
                UDPBlockMulticastSourcesCommand *cmd = (UDPBlockMulticastSourcesCommand *)ctrl;
                InterfaceEntry *ie = ift->getInterfaceById(cmd->getInterfaceId());
                std::vector<L3Address> sourceList;
                for (int i = 0; i < (int)cmd->getSourceListArraySize(); i++)
                    sourceList.push_back(cmd->getSourceList(i));
                blockMulticastSources(sd, ie, cmd->getMulticastAddr(), sourceList);
            }
            else if (dynamic_cast<UDPUnblockMulticastSourcesCommand *>(ctrl)) {
                UDPUnblockMulticastSourcesCommand *cmd = (UDPUnblockMulticastSourcesCommand *)ctrl;
                InterfaceEntry *ie = ift->getInterfaceById(cmd->getInterfaceId());
                std::vector<L3Address> sourceList;
                for (int i = 0; i < (int)cmd->getSourceListArraySize(); i++)
                    sourceList.push_back(cmd->getSourceList(i));
                leaveMulticastSources(sd, ie, cmd->getMulticastAddr(), sourceList);
            }
            else if (dynamic_cast<UDPJoinMulticastSourcesCommand *>(ctrl)) {
                UDPJoinMulticastSourcesCommand *cmd = (UDPJoinMulticastSourcesCommand *)ctrl;
                InterfaceEntry *ie = ift->getInterfaceById(cmd->getInterfaceId());
                std::vector<L3Address> sourceList;
                for (int i = 0; i < (int)cmd->getSourceListArraySize(); i++)
                    sourceList.push_back(cmd->getSourceList(i));
                joinMulticastSources(sd, ie, cmd->getMulticastAddr(), sourceList);
            }
            else if (dynamic_cast<UDPLeaveMulticastSourcesCommand *>(ctrl)) {
                UDPLeaveMulticastSourcesCommand *cmd = (UDPLeaveMulticastSourcesCommand *)ctrl;
                InterfaceEntry *ie = ift->getInterfaceById(cmd->getInterfaceId());
                std::vector<L3Address> sourceList;
                for (int i = 0; i < (int)cmd->getSourceListArraySize(); i++)
                    sourceList.push_back(cmd->getSourceList(i));
                leaveMulticastSources(sd, ie, cmd->getMulticastAddr(), sourceList);
            }
            else if (dynamic_cast<UDPSetMulticastSourceFilterCommand *>(ctrl)) {
                UDPSetMulticastSourceFilterCommand *cmd = (UDPSetMulticastSourceFilterCommand *)ctrl;
                InterfaceEntry *ie = ift->getInterfaceById(cmd->getInterfaceId());
                std::vector<L3Address> sourceList;
                for (int i = 0; i < (int)cmd->getSourceListArraySize(); i++)
                    sourceList.push_back(cmd->getSourceList(i));
                setMulticastSourceFilter(sd, ie, cmd->getMulticastAddr(), (UDPSourceFilterMode)cmd->getFilterMode(), sourceList);
            }
            else
                throw cRuntimeError("Unknown subclass of UDPSetOptionCommand received from app: %s", ctrl->getClassName());
            break;
        }

        default: {
            throw cRuntimeError("Unknown command code (message kind) %d received from app", msg->getKind());
        }
    }

    delete msg;    // also deletes control info in it
}

void UDP::processPacketFromApp(cPacket *appData)
{
    UDPSendCommand *ctrl = check_and_cast<UDPSendCommand *>(appData->removeControlInfo());

    SockDesc *sd = getOrCreateSocket(ctrl->getSockId(), appData->getArrivalGate()->getIndex());
    const L3Address& destAddr = ctrl->getDestAddr().isUnspecified() ? sd->remoteAddr : ctrl->getDestAddr();
    int destPort = ctrl->getDestPort() == -1 ? sd->remotePort : ctrl->getDestPort();
    if (destAddr.isUnspecified() || destPort == -1)
        throw cRuntimeError("send: missing destination address or port when sending over unconnected port");

    const L3Address& srcAddr = ctrl->getSrcAddr().isUnspecified() ? sd->localAddr : ctrl->getSrcAddr();
    int interfaceId = ctrl->getInterfaceId();
    if (interfaceId == -1 && destAddr.isMulticast()) {
        auto membership = sd->findFirstMulticastMembership(destAddr);
        interfaceId = (membership != sd->multicastMembershipTable.end() && (*membership)->interfaceId != -1) ? (*membership)->interfaceId : sd->multicastOutputInterfaceId;
    }
    sendDown(appData, srcAddr, sd->localPort, destAddr, destPort, interfaceId, sd->multicastLoop, sd->ttl, sd->typeOfService);

    delete ctrl;    // cannot be deleted earlier, due to destAddr
}

void UDP::processUDPPacket(UDPPacket *udpPacket)
{
    emit(rcvdPkSignal, udpPacket);

    // simulate checksum: discard packet if it has bit error
    EV_INFO << "Packet " << udpPacket->getName() << " received from network, dest port " << udpPacket->getDestinationPort() << "\n";

    if (udpPacket->hasBitError()) {
        EV_WARN << "Packet has bit error, discarding\n";
        emit(droppedPkBadChecksumSignal, udpPacket);
        numDroppedBadChecksum++;
        delete udpPacket;

        return;
    }

    L3Address srcAddr;
    L3Address destAddr;
    bool isMulticast, isBroadcast;
    int srcPort = udpPacket->getSourcePort();
    int destPort = udpPacket->getDestinationPort();
    int interfaceId;
    int ttl;
    unsigned char tos;

    cObject *ctrl = udpPacket->removeControlInfo();
    if (dynamic_cast<IPv4ControlInfo *>(ctrl) != nullptr) {
        IPv4ControlInfo *ctrl4 = (IPv4ControlInfo *)ctrl;
        srcAddr = ctrl4->getSrcAddr();
        destAddr = ctrl4->getDestAddr();
        interfaceId = ctrl4->getInterfaceId();
        ttl = ctrl4->getTimeToLive();
        tos = ctrl4->getTypeOfService();
        isMulticast = ctrl4->getDestAddr().isMulticast();
        isBroadcast = ctrl4->getDestAddr().isLimitedBroadcastAddress();    // note: we cannot recognize other broadcast addresses (where the host part is all-ones), because here we don't know the netmask
    }
    else if (dynamic_cast<IPv6ControlInfo *>(ctrl) != nullptr) {
        IPv6ControlInfo *ctrl6 = (IPv6ControlInfo *)ctrl;
        srcAddr = ctrl6->getSrcAddr();
        destAddr = ctrl6->getDestAddr();
        interfaceId = ctrl6->getInterfaceId();
        ttl = ctrl6->getHopLimit();
        tos = ctrl6->getTrafficClass();
        isMulticast = ctrl6->getDestAddr().isMulticast();
        isBroadcast = false;    // IPv6 has no broadcast, just various multicasts
    }
    else if (dynamic_cast<GenericNetworkProtocolControlInfo *>(ctrl) != nullptr) {
        GenericNetworkProtocolControlInfo *ctrlGeneric = (GenericNetworkProtocolControlInfo *)ctrl;
        srcAddr = ctrlGeneric->getSourceAddress();
        destAddr = ctrlGeneric->getDestinationAddress();
        interfaceId = ctrlGeneric->getInterfaceId();
        ttl = ctrlGeneric->getHopLimit();
        tos = 0;    // TODO: ctrlGeneric->getTrafficClass();
        isMulticast = ctrlGeneric->getDestinationAddress().isMulticast();
        isBroadcast = false;    // IPv6 has no broadcast, just various multicasts
    }
    else if (ctrl == nullptr) {
        throw cRuntimeError("(%s)%s arrived from lower layer without control info",
                udpPacket->getClassName(), udpPacket->getName());
    }
    else {
        throw cRuntimeError("(%s)%s arrived from lower layer with unrecognized control info %s",
                udpPacket->getClassName(), udpPacket->getName(), ctrl->getClassName());
    }

    if (!isMulticast && !isBroadcast) {
        // unicast packet, there must be only one socket listening
        SockDesc *sd = findSocketForUnicastPacket(destAddr, destPort, srcAddr, srcPort);
        if (!sd) {
            EV_WARN << "No socket registered on port " << destPort << "\n";
            processUndeliverablePacket(udpPacket, ctrl);
            return;
        }
        else {
            cPacket *payload = udpPacket->decapsulate();
            sendUp(payload, sd, srcAddr, srcPort, destAddr, destPort, interfaceId, ttl, tos);
            delete udpPacket;
            delete ctrl;
        }
    }
    else {
        // multicast packet: find all matching sockets, and send up a copy to each
        std::vector<SockDesc *> sds = findSocketsForMcastBcastPacket(destAddr, destPort, srcAddr, srcPort, isMulticast, isBroadcast);
        if (sds.empty()) {
            EV_WARN << "No socket registered on port " << destPort << "\n";
            processUndeliverablePacket(udpPacket, ctrl);
            return;
        }
        else {
            cPacket *payload = udpPacket->decapsulate();
            unsigned int i;
            for (i = 0; i < sds.size() - 1; i++) // sds.size() >= 1
                sendUp(payload->dup(), sds[i], srcAddr, srcPort, destAddr, destPort, interfaceId, ttl, tos); // dup() to all but the last one
            sendUp(payload, sds[i], srcAddr, srcPort, destAddr, destPort, interfaceId, ttl, tos);    // send original to last socket
            delete udpPacket;
            delete ctrl;
        }
    }
}

void UDP::processICMPError(cPacket *pk)
{
    // extract details from the error message, then try to notify socket that sent bogus packet
    int type, code;
    L3Address localAddr, remoteAddr;
    ushort localPort, remotePort;

#ifdef WITH_IPv4
    if (dynamic_cast<ICMPMessage *>(pk)) {
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
#endif // ifdef WITH_IPv4
#ifdef WITH_IPv6
    if (dynamic_cast<ICMPv6Message *>(pk)) {
        ICMPv6Message *icmpMsg = (ICMPv6Message *)pk;
        type = icmpMsg->getType();
        code = -1;    // FIXME this is dependent on getType()...
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
#endif // ifdef WITH_IPv6
    {
        throw cRuntimeError("Unrecognized packet (%s)%s: not an ICMP error message", pk->getClassName(), pk->getName());
    }

    EV_WARN << "ICMP error received: type=" << type << " code=" << code
            << " about packet " << localAddr << ":" << localPort << " > "
            << remoteAddr << ":" << remotePort << "\n";

    // identify socket and report error to it
    SockDesc *sd = findSocketForUnicastPacket(localAddr, localPort, remoteAddr, remotePort);
    if (!sd) {
        EV_WARN << "No socket on that local port, ignoring ICMP error\n";
        return;
    }

    // send UDP_I_ERROR to socket
    EV_DETAIL << "Source socket is sockId=" << sd->sockId << ", notifying.\n";
    sendUpErrorIndication(sd, localAddr, localPort, remoteAddr, remotePort);
}

void UDP::processUndeliverablePacket(UDPPacket *udpPacket, cObject *ctrl)
{
    emit(droppedPkWrongPortSignal, udpPacket);
    numDroppedWrongPort++;

    // send back ICMP PORT_UNREACHABLE
    if (dynamic_cast<IPv4ControlInfo *>(ctrl) != nullptr) {
#ifdef WITH_IPv4
        IPv4ControlInfo *ctrl4 = (IPv4ControlInfo *)ctrl;
        if (!icmp)
            icmp = getModuleFromPar<ICMP>(par("icmpModule"), this);
        icmp->sendErrorMessage(udpPacket, ctrl4, ICMP_DESTINATION_UNREACHABLE, ICMP_DU_PORT_UNREACHABLE);
#else // ifdef WITH_IPv4
        delete udpPacket;
        delete ctrl;
#endif // ifdef WITH_IPv4
    }
    else if (dynamic_cast<IPv6ControlInfo *>(ctrl) != nullptr) {
#ifdef WITH_IPv6
        IPv6ControlInfo *ctrl6 = (IPv6ControlInfo *)ctrl;
        if (!icmpv6)
            icmpv6 = getModuleFromPar<ICMPv6>(par("icmpv6Module"), this);
        icmpv6->sendErrorMessage(udpPacket, ctrl6, ICMPv6_DESTINATION_UNREACHABLE, PORT_UNREACHABLE);
#else // ifdef WITH_IPv6
        delete udpPacket;
        delete ctrl;
#endif // ifdef WITH_IPv6
    }
    else if (dynamic_cast<GenericNetworkProtocolControlInfo *>(ctrl) != nullptr) {
        delete udpPacket;
        delete ctrl;
    }
    else if (ctrl == nullptr) {
        throw cRuntimeError("(%s)%s arrived from lower layer without control info",
                udpPacket->getClassName(), udpPacket->getName());
    }
    else {
        throw cRuntimeError("(%s)%s arrived from lower layer with unrecognized control info %s",
                udpPacket->getClassName(), udpPacket->getName(), ctrl->getClassName());
    }
}

void UDP::bind(int sockId, int gateIndex, const L3Address& localAddr, int localPort)
{
    if (sockId == -1)
        throw cRuntimeError("sockId in BIND message not filled in");

    if (localPort < -1 || localPort > 65535) // -1: ephemeral port
        throw cRuntimeError("bind: invalid local port number %d", localPort);

    auto it = socketsByIdMap.find(sockId);
    SockDesc *sd = it != socketsByIdMap.end() ? it->second : nullptr;

    // to allow two sockets to bind to the same address/port combination
    // both of them must have reuseAddr flag set
    SockDesc *existing = findFirstSocketByLocalAddress(localAddr, localPort);
    if (existing != nullptr && (!sd || !sd->reuseAddr || !existing->reuseAddr))
        throw cRuntimeError("bind: local address/port %s:%u already taken", localAddr.str().c_str(), localPort);

    if (sd) {
        if (sd->isBound)
            throw cRuntimeError("bind: socket is already bound (sockId=%d)", sockId);

        sd->isBound = true;
        sd->localAddr = localAddr;
        if (localPort != -1 && sd->localPort != localPort) {
            socketsByPortMap[sd->localPort].remove(sd);
            sd->localPort = localPort;
            socketsByPortMap[sd->localPort].push_back(sd);
        }
    }
    else {
        sd = createSocket(sockId, gateIndex, localAddr, localPort);
        sd->isBound = true;
    }
}

void UDP::connect(int sockId, int gateIndex, const L3Address& remoteAddr, int remotePort)
{
    if (remoteAddr.isUnspecified())
        throw cRuntimeError("connect: unspecified remote address");
    if (remotePort <= 0 || remotePort > 65535)
        throw cRuntimeError("connect: invalid remote port number %d", remotePort);

    SockDesc *sd = getOrCreateSocket(sockId, gateIndex);
    sd->remoteAddr = remoteAddr;
    sd->remotePort = remotePort;
    sd->onlyLocalPortIsSet = false;

    EV_INFO << "Socket connected: " << *sd << "\n";
}

UDP::SockDesc *UDP::createSocket(int sockId, int gateIndex, const L3Address& localAddr, int localPort)
{
    // create and fill in SockDesc
    SockDesc *sd = new SockDesc(sockId, gateIndex);
    sd->isBound = false;
    sd->localAddr = localAddr;
    sd->localPort = localPort == -1 ? getEphemeralPort() : localPort;
    sd->onlyLocalPortIsSet = sd->localAddr.isUnspecified();

    // add to socketsByIdMap
    socketsByIdMap[sockId] = sd;

    // add to socketsByPortMap
    SockDescList& list = socketsByPortMap[sd->localPort];    // create if doesn't exist
    list.push_back(sd);

    EV_INFO << "Socket created: " << *sd << "\n";
    return sd;
}

void UDP::close(int sockId)
{
    // remove from socketsByIdMap
    auto it = socketsByIdMap.find(sockId);
    if (it == socketsByIdMap.end())
        throw cRuntimeError("socket id=%d doesn't exist (already closed?)", sockId);
    SockDesc *sd = it->second;
    socketsByIdMap.erase(it);

    EV_INFO << "Closing socket: " << *sd << "\n";

    // remove from socketsByPortMap
    SockDescList& list = socketsByPortMap[sd->localPort];
    for (auto it = list.begin(); it != list.end(); ++it)
        if (*it == sd) {
            list.erase(it);
            break;
        }
    if (list.empty())
        socketsByPortMap.erase(sd->localPort);
    delete sd;
}

void UDP::clearAllSockets()
{
    EV_INFO << "Clear all sockets\n";

    for (auto it = socketsByPortMap.begin(); it != socketsByPortMap.end(); ++it) {
        it->second.clear();
    }
    socketsByPortMap.clear();
    for (auto it = socketsByIdMap.begin(); it != socketsByIdMap.end(); ++it)
        delete it->second;
    socketsByIdMap.clear();
}

ushort UDP::getEphemeralPort()
{
    // start at the last allocated port number + 1, and search for an unused one
    ushort searchUntil = lastEphemeralPort++;
    if (lastEphemeralPort == EPHEMERAL_PORTRANGE_END) // wrap
        lastEphemeralPort = EPHEMERAL_PORTRANGE_START;

    while (socketsByPortMap.find(lastEphemeralPort) != socketsByPortMap.end()) {
        if (lastEphemeralPort == searchUntil) // got back to starting point?
            throw cRuntimeError("Ephemeral port range %d..%d exhausted, all ports occupied", EPHEMERAL_PORTRANGE_START, EPHEMERAL_PORTRANGE_END);
        lastEphemeralPort++;
        if (lastEphemeralPort == EPHEMERAL_PORTRANGE_END) // wrap
            lastEphemeralPort = EPHEMERAL_PORTRANGE_START;
    }

    // found a free one, return it
    return lastEphemeralPort;
}

UDP::SockDesc *UDP::findFirstSocketByLocalAddress(const L3Address& localAddr, ushort localPort)
{
    auto it = socketsByPortMap.find(localPort);
    if (it == socketsByPortMap.end())
        return nullptr;

    SockDescList& list = it->second;
    for (auto it = list.begin(); it != list.end(); ++it) {
        SockDesc *sd = *it;
        if (sd->localAddr.isUnspecified() || sd->localAddr == localAddr)
            return sd;
    }
    return nullptr;
}

UDP::SockDesc *UDP::findSocketForUnicastPacket(const L3Address& localAddr, ushort localPort, const L3Address& remoteAddr, ushort remotePort)
{
    auto it = socketsByPortMap.find(localPort);
    if (it == socketsByPortMap.end())
        return nullptr;

    // select the socket bound to ANY_ADDR only if there is no socket bound to localAddr
    SockDescList& list = it->second;
    SockDesc *socketBoundToAnyAddress = nullptr;
    for (SockDescList::reverse_iterator it = list.rbegin(); it != list.rend(); ++it) {
        SockDesc *sd = *it;
        if (sd->onlyLocalPortIsSet || (
                (sd->remotePort == -1 || sd->remotePort == remotePort) &&
                (sd->localAddr.isUnspecified() || sd->localAddr == localAddr) &&
                (sd->remoteAddr.isUnspecified() || sd->remoteAddr == remoteAddr)))
        {
            if (sd->localAddr.isUnspecified())
                socketBoundToAnyAddress = sd;
            else
                return sd;
        }
    }
    return socketBoundToAnyAddress;
}

std::vector<UDP::SockDesc *> UDP::findSocketsForMcastBcastPacket(const L3Address& localAddr, ushort localPort, const L3Address& remoteAddr, ushort remotePort, bool isMulticast, bool isBroadcast)
{
    ASSERT(isMulticast || isBroadcast);
    std::vector<SockDesc *> result;
    auto it = socketsByPortMap.find(localPort);
    if (it == socketsByPortMap.end())
        return result;

    SockDescList& list = it->second;
    for (auto it = list.begin(); it != list.end(); ++it) {
        SockDesc *sd = *it;
        if (isBroadcast) {
            if (sd->isBroadcast) {
                if ((sd->remotePort == -1 || sd->remotePort == remotePort) &&
                    (sd->remoteAddr.isUnspecified() || sd->remoteAddr == remoteAddr))
                    result.push_back(sd);
            }
        }
        else if (isMulticast) {
            auto membership = sd->findFirstMulticastMembership(localAddr);
            if (membership != sd->multicastMembershipTable.end()) {
                if ((sd->remotePort == -1 || sd->remotePort == remotePort) &&
                    (sd->remoteAddr.isUnspecified() || sd->remoteAddr == remoteAddr) &&
                    (*membership)->isSourceAllowed(remoteAddr))
                    result.push_back(sd);
            }
        }
    }
    return result;
}

void UDP::sendUp(cPacket *payload, SockDesc *sd, const L3Address& srcAddr, ushort srcPort, const L3Address& destAddr, ushort destPort, int interfaceId, int ttl, unsigned char tos)
{
    EV_INFO << "Sending payload up to socket sockId=" << sd->sockId << "\n";

    // send payload with UDPControlInfo up to the application
    UDPDataIndication *udpCtrl = new UDPDataIndication();
    udpCtrl->setSockId(sd->sockId);
    udpCtrl->setSrcAddr(srcAddr);
    udpCtrl->setDestAddr(destAddr);
    udpCtrl->setSrcPort(srcPort);
    udpCtrl->setDestPort(destPort);
    udpCtrl->setInterfaceId(interfaceId);
    udpCtrl->setTtl(ttl);
    udpCtrl->setTypeOfService(tos);
    payload->setControlInfo(udpCtrl);
    payload->setKind(UDP_I_DATA);

    emit(passedUpPkSignal, payload);
    send(payload, "appOut", sd->appGateIndex);
    numPassedUp++;
}

void UDP::sendUpErrorIndication(SockDesc *sd, const L3Address& localAddr, ushort localPort, const L3Address& remoteAddr, ushort remotePort)
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

void UDP::sendDown(cPacket *appData, const L3Address& srcAddr, ushort srcPort, const L3Address& destAddr, ushort destPort,
        int interfaceId, bool multicastLoop, int ttl, unsigned char tos)
{
    if (destAddr.isUnspecified())
        throw cRuntimeError("send: unspecified destination address");
    if (destPort <= 0 || destPort > 65535)
        throw cRuntimeError("send invalid remote port number %d", destPort);

    UDPPacket *udpPacket = createUDPPacket(appData->getName());
    udpPacket->setByteLength(UDP_HEADER_BYTES);
    udpPacket->encapsulate(appData);

    // set source and destination port
    udpPacket->setSourcePort(srcPort);
    udpPacket->setDestinationPort(destPort);

    if (destAddr.getType() == L3Address::IPv4) {
        // send to IPv4
        EV_INFO << "Sending app packet " << appData->getName() << " over IPv4.\n";
        IPv4ControlInfo *ipControlInfo = new IPv4ControlInfo();
        ipControlInfo->setProtocol(IP_PROT_UDP);
        ipControlInfo->setSrcAddr(srcAddr.toIPv4());
        ipControlInfo->setDestAddr(destAddr.toIPv4());
        ipControlInfo->setInterfaceId(interfaceId);
        ipControlInfo->setMulticastLoop(multicastLoop);
        ipControlInfo->setTimeToLive(ttl);
        ipControlInfo->setTypeOfService(tos);
        udpPacket->setControlInfo(ipControlInfo);

        emit(sentPkSignal, udpPacket);
        send(udpPacket, "ipOut");
    }
    else if (destAddr.getType() == L3Address::IPv6) {
        // send to IPv6
        EV_INFO << "Sending app packet " << appData->getName() << " over IPv6.\n";
        IPv6ControlInfo *ipControlInfo = new IPv6ControlInfo();
        ipControlInfo->setProtocol(IP_PROT_UDP);
        ipControlInfo->setSrcAddr(srcAddr.toIPv6());
        ipControlInfo->setDestAddr(destAddr.toIPv6());
        ipControlInfo->setInterfaceId(interfaceId);
        ipControlInfo->setMulticastLoop(multicastLoop);
        ipControlInfo->setHopLimit(ttl);
        ipControlInfo->setTrafficClass(tos);
        udpPacket->setControlInfo(ipControlInfo);

        emit(sentPkSignal, udpPacket);
        send(udpPacket, "ipOut");
    }
    else {
        // send to generic
        EV_INFO << "Sending app packet " << appData->getName() << endl;
        IL3AddressType *addressType = destAddr.getAddressType();
        INetworkProtocolControlInfo *ipControlInfo = addressType->createNetworkProtocolControlInfo();
        ipControlInfo->setTransportProtocol(IP_PROT_UDP);
        ipControlInfo->setSourceAddress(srcAddr);
        ipControlInfo->setDestinationAddress(destAddr);
        ipControlInfo->setInterfaceId(interfaceId);
        //ipControlInfo->setMulticastLoop(multicastLoop);
        ipControlInfo->setHopLimit(ttl);
        //ipControlInfo->setTrafficClass(tos);
        udpPacket->setControlInfo(dynamic_cast<cObject *>(ipControlInfo));

        emit(sentPkSignal, udpPacket);
        send(udpPacket, "ipOut");
    }
    numSent++;
}

UDPPacket *UDP::createUDPPacket(const char *name)
{
    return new UDPPacket(name);
}

UDP::SockDesc *UDP::getSocketById(int sockId)
{
    auto it = socketsByIdMap.find(sockId);
    if (it == socketsByIdMap.end())
        throw cRuntimeError("socket id=%d doesn't exist (already closed?)", sockId);
    return it->second;
}

UDP::SockDesc *UDP::getOrCreateSocket(int sockId, int gateIndex)
{
    // validate sockId
    if (sockId == -1)
        throw cRuntimeError("sockId in UDP command not filled in");

    auto it = socketsByIdMap.find(sockId);
    if (it != socketsByIdMap.end())
        return it->second;

    return createSocket(sockId, gateIndex, L3Address(), -1);
}

void UDP::setTimeToLive(SockDesc *sd, int ttl)
{
    sd->ttl = ttl;
}

void UDP::setTypeOfService(SockDesc *sd, int typeOfService)
{
    sd->typeOfService = typeOfService;
}

void UDP::setBroadcast(SockDesc *sd, bool broadcast)
{
    sd->isBroadcast = broadcast;
}

void UDP::setMulticastOutputInterface(SockDesc *sd, int interfaceId)
{
    sd->multicastOutputInterfaceId = interfaceId;
}

void UDP::setMulticastLoop(SockDesc *sd, bool loop)
{
    sd->multicastLoop = loop;
}

void UDP::setReuseAddress(SockDesc *sd, bool reuseAddr)
{
    sd->reuseAddr = reuseAddr;
}

void UDP::joinMulticastGroups(SockDesc *sd, const std::vector<L3Address>& multicastAddresses, const std::vector<int> interfaceIds)
{
    int multicastAddressesLen = multicastAddresses.size();
    int interfaceIdsLen = interfaceIds.size();
    for (int k = 0; k < multicastAddressesLen; k++) {
        const L3Address& multicastAddr = multicastAddresses[k];
        int interfaceId = k < interfaceIdsLen ? interfaceIds[k] : -1;
        ASSERT(multicastAddr.isMulticast());

        MulticastMembership *membership = sd->findMulticastMembership(multicastAddr, interfaceId);
        if (membership)
            throw cRuntimeError("UPD::joinMulticastGroups(): %s group on interface %s is already joined.",
                    multicastAddr.str().c_str(), ift->getInterfaceById(interfaceId)->getFullName());

        membership = new MulticastMembership();
        membership->interfaceId = interfaceId;
        membership->multicastAddress = multicastAddr;
        membership->filterMode = UDP_EXCLUDE_MCAST_SOURCES;
        sd->addMulticastMembership(membership);

        // add the multicast address to the selected interface or all interfaces
        if (interfaceId != -1) {
            InterfaceEntry *ie = ift->getInterfaceById(interfaceId);
            if (!ie)
                throw cRuntimeError("Interface id=%d does not exist", interfaceId);
            ASSERT(ie->isMulticast());
            addMulticastAddressToInterface(ie, multicastAddr);
        }
        else {
            int n = ift->getNumInterfaces();
            for (int i = 0; i < n; i++) {
                InterfaceEntry *ie = ift->getInterface(i);
                if (ie->isMulticast())
                    addMulticastAddressToInterface(ie, multicastAddr);
            }
        }
    }
}

void UDP::addMulticastAddressToInterface(InterfaceEntry *ie, const L3Address& multicastAddr)
{
    ASSERT(ie && ie->isMulticast());
    ASSERT(multicastAddr.isMulticast());

    if (multicastAddr.getType() == L3Address::IPv4) {
#ifdef WITH_IPv4
        ie->ipv4Data()->joinMulticastGroup(multicastAddr.toIPv4());
#endif // ifdef WITH_IPv4
    }
    else if (multicastAddr.getType() == L3Address::IPv6) {
#ifdef WITH_IPv6
        ie->ipv6Data()->assignAddress(multicastAddr.toIPv6(), false, SimTime::getMaxTime(), SimTime::getMaxTime());
#endif // ifdef WITH_IPv6
    }
    else
        ie->joinMulticastGroup(multicastAddr);
}

void UDP::leaveMulticastGroups(SockDesc *sd, const std::vector<L3Address>& multicastAddresses)
{
    std::vector<L3Address> empty;

    for (unsigned int i = 0; i < multicastAddresses.size(); i++) {
        auto it = sd->findFirstMulticastMembership(multicastAddresses[i]);
        while (it != sd->multicastMembershipTable.end()) {
            MulticastMembership *membership = *it;
            if (membership->multicastAddress != multicastAddresses[i])
                break;
            it = sd->multicastMembershipTable.erase(it);

            McastSourceFilterMode oldFilterMode = membership->filterMode == UDP_INCLUDE_MCAST_SOURCES ?
                MCAST_INCLUDE_SOURCES : MCAST_EXCLUDE_SOURCES;

            if (membership->interfaceId != -1) {
                InterfaceEntry *ie = ift->getInterfaceById(membership->interfaceId);
                ie->changeMulticastGroupMembership(membership->multicastAddress,
                        oldFilterMode, membership->sourceList, MCAST_INCLUDE_SOURCES, empty);
            }
            else {
                for (int j = 0; j < ift->getNumInterfaces(); ++j) {
                    InterfaceEntry *ie = ift->getInterface(j);
                    if (ie->isMulticast())
                        ie->changeMulticastGroupMembership(membership->multicastAddress,
                                oldFilterMode, membership->sourceList, MCAST_INCLUDE_SOURCES, empty);
                }
            }
            delete membership;
        }
    }
}

void UDP::blockMulticastSources(SockDesc *sd, InterfaceEntry *ie, L3Address multicastAddress, const std::vector<L3Address>& sourceList)
{
    ASSERT(ie && ie->isMulticast());
    ASSERT(multicastAddress.isMulticast());

    MulticastMembership *membership = sd->findMulticastMembership(multicastAddress, ie->getInterfaceId());
    if (!membership)
        throw cRuntimeError("UDP::blockMulticastSources(): not a member of %s group on interface '%s'",
                multicastAddress.str().c_str(), ie->getFullName());

    if (membership->filterMode != UDP_EXCLUDE_MCAST_SOURCES)
        throw cRuntimeError("UDP::blockMulticastSources(): socket was not joined to all sources of %s group on interface '%s'",
                multicastAddress.str().c_str(), ie->getFullName());

    std::vector<L3Address> oldSources(membership->sourceList);
    std::vector<L3Address>& excludedSources = membership->sourceList;
    bool changed = false;
    for (unsigned int i = 0; i < sourceList.size(); ++i) {
        const L3Address& sourceAddress = sourceList[i];
        auto it = std::find(excludedSources.begin(), excludedSources.end(), sourceAddress);
        if (it != excludedSources.end()) {
            excludedSources.push_back(sourceAddress);
            changed = true;
        }
    }

    if (changed) {
        ie->changeMulticastGroupMembership(multicastAddress, MCAST_EXCLUDE_SOURCES, oldSources, MCAST_EXCLUDE_SOURCES, excludedSources);
    }
}

void UDP::unblockMulticastSources(SockDesc *sd, InterfaceEntry *ie, L3Address multicastAddress, const std::vector<L3Address>& sourceList)
{
    ASSERT(ie && ie->isMulticast());
    ASSERT(multicastAddress.isMulticast());

    MulticastMembership *membership = sd->findMulticastMembership(multicastAddress, ie->getInterfaceId());
    if (!membership)
        throw cRuntimeError("UDP::unblockMulticastSources(): not a member of %s group in interface '%s'",
                multicastAddress.str().c_str(), ie->getFullName());

    if (membership->filterMode != UDP_EXCLUDE_MCAST_SOURCES)
        throw cRuntimeError("UDP::unblockMulticastSources(): socket was not joined to all sources of %s group on interface '%s'",
                multicastAddress.str().c_str(), ie->getFullName());

    std::vector<L3Address> oldSources(membership->sourceList);
    std::vector<L3Address>& excludedSources = membership->sourceList;
    bool changed = false;
    for (unsigned int i = 0; i < sourceList.size(); ++i) {
        const L3Address& sourceAddress = sourceList[i];
        auto it = std::find(excludedSources.begin(), excludedSources.end(), sourceAddress);
        if (it != excludedSources.end()) {
            excludedSources.erase(it);
            changed = true;
        }
    }

    if (changed) {
        ie->changeMulticastGroupMembership(multicastAddress, MCAST_EXCLUDE_SOURCES, oldSources, MCAST_EXCLUDE_SOURCES, excludedSources);
    }
}

void UDP::joinMulticastSources(SockDesc *sd, InterfaceEntry *ie, L3Address multicastAddress, const std::vector<L3Address>& sourceList)
{
    ASSERT(ie && ie->isMulticast());
    ASSERT(multicastAddress.isMulticast());

    MulticastMembership *membership = sd->findMulticastMembership(multicastAddress, ie->getInterfaceId());
    if (!membership) {
        membership = new MulticastMembership();
        membership->interfaceId = ie->getInterfaceId();
        membership->multicastAddress = multicastAddress;
        membership->filterMode = UDP_INCLUDE_MCAST_SOURCES;
        sd->addMulticastMembership(membership);
    }

    if (membership->filterMode == UDP_EXCLUDE_MCAST_SOURCES)
        throw cRuntimeError("UDP::joinMulticastSources(): socket was joined to all sources of %s group on interface '%s'",
                multicastAddress.str().c_str(), ie->getFullName());

    std::vector<L3Address> oldSources(membership->sourceList);
    std::vector<L3Address>& includedSources = membership->sourceList;
    bool changed = false;
    for (unsigned int i = 0; i < sourceList.size(); ++i) {
        const L3Address& sourceAddress = sourceList[i];
        auto it = std::find(includedSources.begin(), includedSources.end(), sourceAddress);
        if (it != includedSources.end()) {
            includedSources.push_back(sourceAddress);
            changed = true;
        }
    }

    if (changed) {
        ie->changeMulticastGroupMembership(multicastAddress, MCAST_INCLUDE_SOURCES, oldSources, MCAST_INCLUDE_SOURCES, includedSources);
    }
}

void UDP::leaveMulticastSources(SockDesc *sd, InterfaceEntry *ie, L3Address multicastAddress, const std::vector<L3Address>& sourceList)
{
    ASSERT(ie && ie->isMulticast());
    ASSERT(multicastAddress.isMulticast());

    MulticastMembership *membership = sd->findMulticastMembership(multicastAddress, ie->getInterfaceId());
    if (!membership)
        throw cRuntimeError("UDP::leaveMulticastSources(): not a member of %s group in interface '%s'",
                multicastAddress.str().c_str(), ie->getFullName());

    if (membership->filterMode == UDP_EXCLUDE_MCAST_SOURCES)
        throw cRuntimeError("UDP::leaveMulticastSources(): socket was joined to all sources of %s group on interface '%s'",
                multicastAddress.str().c_str(), ie->getFullName());

    std::vector<L3Address> oldSources(membership->sourceList);
    std::vector<L3Address>& includedSources = membership->sourceList;
    bool changed = false;
    for (unsigned int i = 0; i < sourceList.size(); ++i) {
        const L3Address& sourceAddress = sourceList[i];
        auto it = std::find(includedSources.begin(), includedSources.end(), sourceAddress);
        if (it != includedSources.end()) {
            includedSources.erase(it);
            changed = true;
        }
    }

    if (changed) {
        ie->changeMulticastGroupMembership(multicastAddress, MCAST_EXCLUDE_SOURCES, oldSources, MCAST_EXCLUDE_SOURCES, includedSources);
    }

    if (includedSources.empty())
        sd->deleteMulticastMembership(membership);
}

void UDP::setMulticastSourceFilter(SockDesc *sd, InterfaceEntry *ie, L3Address multicastAddress, UDPSourceFilterMode filterMode, const std::vector<L3Address>& sourceList)
{
    ASSERT(ie && ie->isMulticast());
    ASSERT(multicastAddress.isMulticast());

    MulticastMembership *membership = sd->findMulticastMembership(multicastAddress, ie->getInterfaceId());
    if (!membership) {
        membership = new MulticastMembership();
        membership->interfaceId = ie->getInterfaceId();
        membership->multicastAddress = multicastAddress;
        membership->filterMode = UDP_INCLUDE_MCAST_SOURCES;
        sd->addMulticastMembership(membership);
    }

    bool changed = membership->filterMode != filterMode ||
        membership->sourceList.size() != sourceList.size() ||
        !equal(sourceList.begin(), sourceList.end(), membership->sourceList.begin());
    if (changed) {
        std::vector<L3Address> oldSources(membership->sourceList);
        McastSourceFilterMode oldFilterMode = membership->filterMode == UDP_INCLUDE_MCAST_SOURCES ?
            MCAST_INCLUDE_SOURCES : MCAST_EXCLUDE_SOURCES;
        McastSourceFilterMode newFilterMode = filterMode == UDP_INCLUDE_MCAST_SOURCES ?
            MCAST_INCLUDE_SOURCES : MCAST_EXCLUDE_SOURCES;

        membership->filterMode = filterMode;
        membership->sourceList = sourceList;

        ie->changeMulticastGroupMembership(multicastAddress, oldFilterMode, oldSources, newFilterMode, sourceList);
    }
}

bool UDP::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();

    if (dynamic_cast<NodeStartOperation *>(operation)) {
        if ((NodeStartOperation::Stage)stage == NodeStartOperation::STAGE_TRANSPORT_LAYER) {
            icmp = nullptr;
            icmpv6 = nullptr;
            isOperational = true;
        }
    }
    else if (dynamic_cast<NodeShutdownOperation *>(operation)) {
        if ((NodeShutdownOperation::Stage)stage == NodeShutdownOperation::STAGE_TRANSPORT_LAYER) {
            clearAllSockets();
            icmp = nullptr;
            icmpv6 = nullptr;
            isOperational = false;
        }
    }
    else if (dynamic_cast<NodeCrashOperation *>(operation)) {
        if ((NodeCrashOperation::Stage)stage == NodeCrashOperation::STAGE_CRASH) {
            clearAllSockets();
            icmp = nullptr;
            icmpv6 = nullptr;
            isOperational = false;
        }
    }
    else {
        throw cRuntimeError("Unsupported operation '%s'", operation->getClassName());
    }

    return true;
}

/*
 * Multicast memberships are sorted first by multicastAddress, then by interfaceId.
 * The interfaceId -1 comes after any other interfaceId.
 */
static bool lessMembership(const UDP::MulticastMembership *first, const UDP::MulticastMembership *second)
{
    if (first->multicastAddress != second->multicastAddress)
        return first->multicastAddress < second->multicastAddress;

    if (first->interfaceId == -1 || first->interfaceId >= second->interfaceId)
        return false;

    return true;
}

UDP::MulticastMembershipTable::iterator UDP::SockDesc::findFirstMulticastMembership(const L3Address& multicastAddress)
{
    MulticastMembership membership;
    membership.multicastAddress = multicastAddress;
    membership.interfaceId = 0;    // less than any other interfaceId

    auto it = lower_bound(multicastMembershipTable.begin(), multicastMembershipTable.end(), &membership, lessMembership);
    if (it != multicastMembershipTable.end() && (*it)->multicastAddress == multicastAddress)
        return it;
    else
        return multicastMembershipTable.end();
}

UDP::MulticastMembership *UDP::SockDesc::findMulticastMembership(const L3Address& multicastAddress, int interfaceId)
{
    MulticastMembership membership;
    membership.multicastAddress = multicastAddress;
    membership.interfaceId = interfaceId;

    auto it = lower_bound(multicastMembershipTable.begin(), multicastMembershipTable.end(), &membership, lessMembership);
    if (it != multicastMembershipTable.end() && (*it)->multicastAddress == multicastAddress && (*it)->interfaceId == interfaceId)
        return *it;
    else
        return nullptr;
}

void UDP::SockDesc::addMulticastMembership(MulticastMembership *membership)
{
    auto it = lower_bound(multicastMembershipTable.begin(), multicastMembershipTable.end(), membership, lessMembership);
    multicastMembershipTable.insert(it, membership);
}

void UDP::SockDesc::deleteMulticastMembership(MulticastMembership *membership)
{
    multicastMembershipTable.erase(std::remove(multicastMembershipTable.begin(), multicastMembershipTable.end(), membership),
            multicastMembershipTable.end());
    delete membership;
}

} // namespace inet


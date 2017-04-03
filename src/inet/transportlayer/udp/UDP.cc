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

#include "inet/applications/common/SocketTag_m.h"
#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/chunk/cPacketChunk.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/common/serializer/TCPIPchecksum.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/HopLimitTag_m.h"
#include "inet/networklayer/common/InterfaceEntry.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/common/DscpTag_m.h"
#include "inet/networklayer/common/MulticastTag_m.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/contract/IL3AddressType.h"
#include "inet/transportlayer/common/L4PortTag_m.h"
#include "inet/transportlayer/udp/UdpHeader.h"

#ifdef WITH_IPv4
#include "inet/networklayer/ipv4/ICMP.h"
#include "inet/networklayer/ipv4/ICMPHeader.h"
#include "inet/networklayer/ipv4/IPv4Header.h"
#include "inet/networklayer/ipv4/IPv4InterfaceData.h"
#endif // ifdef WITH_IPv4

#ifdef WITH_IPv6
#include "inet/networklayer/icmpv6/ICMPv6.h"
#include "inet/networklayer/icmpv6/ICMPv6Header_m.h"
#include "inet/networklayer/ipv6/IPv6Header.h"
#include "inet/networklayer/ipv6/IPv6ExtensionHeaders.h"
#include "inet/networklayer/ipv6/IPv6InterfaceData.h"
#endif // ifdef WITH_IPv6


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
    for (const auto & elem : list)
        os << "sockId=" << (elem)->sockId << " ";
    return os;
}

//--------

UDP::SockDesc::SockDesc(int sockId_)
{
    sockId = sockId_;
}

UDP::SockDesc::~SockDesc()
{
    for(auto & elem : multicastMembershipTable)
        delete (elem);
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

        const char *crcModeString = par("crcMode");
        if (!strcmp(crcModeString, "disabled"))
            crcMode = CRC_DISABLED;
        else if (!strcmp(crcModeString, "declared"))
            crcMode = CRC_DECLARED_CORRECT;
        else if (!strcmp(crcModeString, "computed")) {
            crcMode = CRC_COMPUTED;
            crcInsertion.udp = this;
#ifdef WITH_IPv4
            auto ipv4 = dynamic_cast<INetfilter *>(getModuleByPath("^.ipv4.ip"));
            if (ipv4 != nullptr)
                ipv4->registerHook(0, &crcInsertion);
#endif
#ifdef WITH_IPv6
            auto ipv6 = dynamic_cast<INetfilter *>(getModuleByPath("^.ipv6.ip"));
            if (ipv6 != nullptr)
                ipv6->registerHook(0, &crcInsertion);
#endif
        }
        else
            throw cRuntimeError("Unknown CRC mode: '%s'", crcModeString);

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
        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
        isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;
        registerProtocol(Protocol::udp, gate("ipOut"));
        registerProtocol(Protocol::udp, gate("appOut"));
    }
}

void UDP::handleMessage(cMessage *msg)
{
    if (!isOperational)
        throw cRuntimeError("Message '%s' received when UDP is OFF", msg->getName());

    // received from IP layer
    if (msg->arrivedOn("ipIn")) {
        Packet *packet = check_and_cast<Packet *>(msg);
        auto protocol = msg->getMandatoryTag<PacketProtocolTag>()->getProtocol();
        if (protocol == &Protocol::udp) {
            processUDPPacket(packet);
        }
        else if (protocol == &Protocol::icmpv4) {
            processICMPv4Error(packet); // assume it's an ICMP error
        }
        else if (protocol == &Protocol::icmpv6) {
            processICMPv6Error(packet); // assume it's an ICMP error
        }
        else
            throw cRuntimeError("Unknown protocol: %s(%d)", protocol->getName(), protocol->getId());
    }
    else {    // received from application layer
        processCommandFromApp(msg);
    }
}

void UDP::refreshDisplay() const
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
    int socketId = msg->getMandatoryTag<SocketReq>()->getSocketId();
    switch (msg->getKind()) {
        case UDP_C_BIND: {
            UDPBindCommand *ctrl = check_and_cast<UDPBindCommand *>(msg->getControlInfo());
            bind(socketId, msg->getArrivalGate()->getIndex(), ctrl->getLocalAddr(), ctrl->getLocalPort());
            break;
        }

        case UDP_C_CONNECT: {
            UDPConnectCommand *ctrl = check_and_cast<UDPConnectCommand *>(msg->getControlInfo());
            connect(socketId, msg->getArrivalGate()->getIndex(), ctrl->getRemoteAddr(), ctrl->getRemotePort());
            break;
        }

        case UDP_C_CLOSE: {
            close(socketId);
            break;
        }

        case UDP_C_DATA:
            processPacketFromApp(check_and_cast<Packet *>(msg));
            return;     // prevent delete of msg

        case UDP_C_SETOPTION: {
            UDPSetOptionCommand *ctrl = check_and_cast<UDPSetOptionCommand *>(msg->getControlInfo());
            SockDesc *sd = getOrCreateSocket(socketId);

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

void UDP::processPacketFromApp(Packet *packet)
{
    L3Address srcAddr, destAddr;
    int srcPort = -1, destPort = -1;

    int socketId = packet->getMandatoryTag<SocketReq>()->getSocketId();
    SockDesc *sd = getOrCreateSocket(socketId);

    auto addressReq = packet->ensureTag<L3AddressReq>();
    srcAddr = addressReq->getSrcAddress();
    destAddr = addressReq->getDestAddress();

    if (srcAddr.isUnspecified())
        addressReq->setSrcAddress(srcAddr = sd->localAddr);
    if (destAddr.isUnspecified())
        addressReq->setDestAddress(destAddr = sd->remoteAddr);
    if (auto portsReq = packet->removeTag<L4PortReq>()) {
        srcPort = portsReq->getSrcPort();
        destPort = portsReq->getDestPort();
        delete portsReq;
    }
    if (srcPort == -1)
        srcPort = sd->localPort;
    if (destPort == -1)
        destPort = sd->remotePort;

    auto interfaceReq = packet->getTag<InterfaceReq>();
    ASSERT(interfaceReq == nullptr || interfaceReq->getInterfaceId() != -1);

    if (interfaceReq == nullptr && destAddr.isMulticast()) {
        auto membership = sd->findFirstMulticastMembership(destAddr);
        int interfaceId = (membership != sd->multicastMembershipTable.end() && (*membership)->interfaceId != -1) ? (*membership)->interfaceId : sd->multicastOutputInterfaceId;
        if (interfaceId != -1)
            packet->ensureTag<InterfaceReq>()->setInterfaceId(interfaceId);
    }

    if (addressReq->getDestAddress().isUnspecified())
        throw cRuntimeError("send: unspecified destination address");
    if (destPort <= 0 || destPort > 65535)
        throw cRuntimeError("send invalid remote port number %d", destPort);


    if (packet->getTag<MulticastReq>() == nullptr)
        packet->ensureTag<MulticastReq>()->setMulticastLoop(sd->multicastLoop);
    if (sd->ttl != -1 && packet->getTag<HopLimitReq>() == nullptr)
        packet->ensureTag<HopLimitReq>()->setHopLimit(sd->ttl);
    if (packet->getTag<DscpReq>() == nullptr)
        packet->ensureTag<DscpReq>()->setDifferentiatedServicesCodePoint(sd->typeOfService);

    const Protocol *l3Protocol = nullptr;
    if (destAddr.getType() == L3Address::IPv4) {
        // send to IPv4
        l3Protocol = &Protocol::ipv4;
    }
    else if (destAddr.getType() == L3Address::IPv6) {
        // send to IPv6
        l3Protocol = &Protocol::ipv6;
    }
    else {
        // send to generic
        l3Protocol = &Protocol::gnp;
    }

    auto udpHeader = std::make_shared<UdpHeader>();
    // set source and destination port
    udpHeader->setSourcePort(srcPort);
    udpHeader->setDestinationPort(destPort);
    udpHeader->setTotalLengthField(byte(udpHeader->getChunkLength() + packet->getTotalLength()).get());
    if (crcMode != CRC_COMPUTED) // CRC_COMPUTED is done in an INetfilter hook
        insertCrc(l3Protocol, L3Address(), L3Address(), udpHeader, packet);
    udpHeader->markImmutable();
    packet->pushHeader(udpHeader);
    packet->ensureTag<PacketProtocolTag>()->setProtocol(&Protocol::udp);
    packet->ensureTag<TransportProtocolTag>()->setProtocol(&Protocol::udp);

    packet->ensureTag<DispatchProtocolReq>()->setProtocol(l3Protocol);

    EV_INFO << "Sending app packet " << packet->getName() << " over " << l3Protocol->getName() << ".\n";
    emit(sentPkSignal, packet);
    send(packet, "ipOut");
    numSent++;
}

void UDP::processUDPPacket(Packet *udpPacket)
{
    ASSERT(udpPacket->getControlInfo() == nullptr);
    emit(rcvdPkSignal, udpPacket);

    bit udpHeaderPopPosition = udpPacket->getHeaderPopOffset();
    const auto& udpHeader = udpPacket->popHeader<UdpHeader>(bit(-1), Chunk::PF_ALLOW_INCORRECT);

    // simulate checksum: discard packet if it has bit error
    EV_INFO << "Packet " << udpPacket->getName() << " received from network, dest port " << udpHeader->getDestinationPort() << "\n";

    auto srcPort = udpHeader->getSourcePort();
    auto destPort = udpHeader->getDestinationPort();
    auto l3AddressInd = udpPacket->getMandatoryTag<L3AddressInd>();
    auto srcAddr = l3AddressInd->getSrcAddress();
    auto destAddr = l3AddressInd->getDestAddress();
    auto totalLength = byte(udpHeader->getTotalLengthField());
    auto hasIncorrectLength = totalLength > udpHeader->getChunkLength() + udpPacket->getDataLength();
    auto networkProtocol = udpPacket->getMandatoryTag<NetworkProtocolInd>()->getProtocol();

    if (hasIncorrectLength || !verifyCrc(networkProtocol, udpHeader, udpPacket)) {
        EV_WARN << "Packet has bit error, discarding\n";
        emit(droppedPkBadChecksumSignal, udpPacket);
        numDroppedBadChecksum++;
        delete udpPacket;
        return;
    }

    bool isMulticast = destAddr.isMulticast();
    bool isBroadcast = destAddr.isBroadcast();
    if (!isMulticast && !isBroadcast) {
        // unicast packet, there must be only one socket listening
        SockDesc *sd = findSocketForUnicastPacket(destAddr, destPort, srcAddr, srcPort);
        if (!sd) {
            EV_WARN << "No socket registered on port " << destPort << "\n";
            udpPacket->setHeaderPopOffset(udpHeaderPopPosition);
            processUndeliverablePacket(udpPacket);
            return;
        }
        else {
            sendUp(udpPacket, sd, srcPort, destPort);
        }
    }
    else {
        // multicast packet: find all matching sockets, and send up a copy to each
        std::vector<SockDesc *> sds = findSocketsForMcastBcastPacket(destAddr, destPort, srcAddr, srcPort, isMulticast, isBroadcast);
        if (sds.empty()) {
            EV_WARN << "No socket registered on port " << destPort << "\n";
            udpPacket->setHeaderPopOffset(udpHeaderPopPosition);
            processUndeliverablePacket(udpPacket);
            return;
        }
        else {
            unsigned int i;
            for (i = 0; i < sds.size() - 1; i++) // sds.size() >= 1
                sendUp(udpPacket->dup(), sds[i], srcPort, destPort); // dup() to all but the last one
            sendUp(udpPacket, sds[i], srcPort, destPort);    // send original to last socket
        }
    }
}

void UDP::processICMPv4Error(Packet *packet)
{
#ifdef WITH_IPv4
    // extract details from the error message, then try to notify socket that sent bogus packet

    int type, code;
    L3Address localAddr, remoteAddr;
    int localPort = -1, remotePort = -1;
    bool udpHeaderAvailable = false;

    const auto& icmpHeader = packet->popHeader<ICMPHeader>();
    ASSERT(icmpHeader);
    type = icmpHeader->getType();
    code = icmpHeader->getCode();
    const auto& ipv4Header = packet->popHeader<IPv4Header>();
    if (ipv4Header->getDontFragment() || ipv4Header->getFragmentOffset() == 0) {
        const auto& udpHeader = packet->peekHeader<UdpHeader>(byte(8), Chunk::PF_ALLOW_INCOMPLETE);
        localAddr = ipv4Header->getSrcAddress();
        remoteAddr = ipv4Header->getDestAddress();
        localPort = udpHeader->getSourcePort();
        remotePort = udpHeader->getDestinationPort();
        udpHeaderAvailable = true;
    }
    EV_WARN << "ICMP error received: type=" << type << " code=" << code
            << " about packet " << localAddr << ":" << localPort << " > "
            << remoteAddr << ":" << remotePort << "\n";

    // identify socket and report error to it
    if (udpHeaderAvailable) {
        SockDesc *sd = findSocketForUnicastPacket(localAddr, localPort, remoteAddr, remotePort);
        if (sd) {
            // send UDP_I_ERROR to socket
            EV_DETAIL << "Source socket is sockId=" << sd->sockId << ", notifying.\n";
            sendUpErrorIndication(sd, localAddr, localPort, remoteAddr, remotePort);
        }
        else {
            EV_WARN << "No socket on that local port, ignoring ICMP error\n";
        }
    }
    else
        EV_WARN << "UDP header not available, ignoring ICMP error\n";
#endif // ifdef WITH_IPv4

    delete packet;
}

void UDP::processICMPv6Error(Packet *packet)
{
#ifdef WITH_IPv6
    // extract details from the error message, then try to notify socket that sent bogus packet

    int type, code;
    L3Address localAddr, remoteAddr;
    ushort localPort, remotePort;
    bool udpHeaderAvailable = false;

    const auto& icmpHeader = packet->popHeader<ICMPv6Header>();
    ASSERT(icmpHeader);

    type = icmpHeader->getType();
    code = -1;    // FIXME this is dependent on getType()...
    // Note: we must NOT use decapsulate() because payload in ICMP is conceptually truncated
    const auto& ipv6Header = packet->popHeader<IPv6Header>();
    IPv6FragmentHeader *fh = dynamic_cast<IPv6FragmentHeader *>(ipv6Header->findExtensionHeaderByType(IP_PROT_IPv6EXT_FRAGMENT));
    if (!fh || fh->getFragmentOffset() == 0) {
        const auto& udpHeader = packet->peekHeader<UdpHeader>(byte(8), Chunk::PF_ALLOW_INCOMPLETE);
        localAddr = ipv6Header->getSrcAddress();
        remoteAddr = ipv6Header->getDestAddress();
        localPort = udpHeader->getSourcePort();
        remotePort = udpHeader->getDestinationPort();
        udpHeaderAvailable = true;
    }

    EV_WARN << "ICMPv6 error received: type=" << type << " code=" << code
            << " about packet " << localAddr << ":" << localPort << " > "
            << remoteAddr << ":" << remotePort << "\n";

    // identify socket and report error to it
    if (udpHeaderAvailable) {
        SockDesc *sd = findSocketForUnicastPacket(localAddr, localPort, remoteAddr, remotePort);
        if (sd) {
            // send UDP_I_ERROR to socket
            EV_DETAIL << "Source socket is sockId=" << sd->sockId << ", notifying.\n";
            sendUpErrorIndication(sd, localAddr, localPort, remoteAddr, remotePort);
        }
        else {
            EV_WARN << "No socket on that local port, ignoring ICMPv6 error\n";
        }
    }
    else
        EV_WARN << "UDP header not available, ignoring ICMPv6 error\n";

#endif // ifdef WITH_IPv6

    delete packet;
}

void UDP::processUndeliverablePacket(Packet *udpPacket)
{
    const auto& udpHeader = udpPacket->peekHeader<UdpHeader>();
    emit(droppedPkWrongPortSignal, udpPacket);
    numDroppedWrongPort++;

    // send back ICMP PORT_UNREACHABLE
    char buff[80];
    snprintf(buff, sizeof(buff), "Port %d unreachable", udpHeader->getDestinationPort());
    udpPacket->setName(buff);
    const Protocol *protocol = udpPacket->getMandatoryTag<NetworkProtocolInd>()->getProtocol();

    if (protocol == nullptr) {
        throw cRuntimeError("(%s)%s arrived from lower layer without NetworkProtocolInd",
                udpPacket->getClassName(), udpPacket->getName());
    }

    //push back network protocol header
    udpPacket->removePoppedChunks();
    udpPacket->pushHeader(udpPacket->getMandatoryTag<NetworkProtocolInd>()->getNetworkProtocolHeader());
    auto inIe = udpPacket->getMandatoryTag<InterfaceInd>()->getInterfaceId();

    if (protocol->getId() == Protocol::ipv4.getId()) {
#ifdef WITH_IPv4
        if (!icmp)
            icmp = getModuleFromPar<ICMP>(par("icmpModule"), this);
        icmp->sendErrorMessage(udpPacket, inIe, ICMP_DESTINATION_UNREACHABLE, ICMP_DU_PORT_UNREACHABLE);
#else // ifdef WITH_IPv4
        delete udpPacket;
#endif // ifdef WITH_IPv4
    }
    else if (protocol->getId() == Protocol::ipv6.getId()) {
#ifdef WITH_IPv6
        if (!icmpv6)
            icmpv6 = getModuleFromPar<ICMPv6>(par("icmpv6Module"), this);
        icmpv6->sendErrorMessage(udpPacket, ICMPv6_DESTINATION_UNREACHABLE, PORT_UNREACHABLE);
#else // ifdef WITH_IPv6
        delete udpPacket;
#endif // ifdef WITH_IPv6
    }
    else if (protocol->getId() == Protocol::gnp.getId()) {
        delete udpPacket;
    }
    else {
        throw cRuntimeError("(%s)%s arrived from lower layer with unrecognized NetworkProtocolInd %s",
                udpPacket->getClassName(), udpPacket->getName(), protocol->getName());
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
        sd = createSocket(sockId, localAddr, localPort);
        sd->isBound = true;
    }
}

void UDP::connect(int sockId, int gateIndex, const L3Address& remoteAddr, int remotePort)
{
    if (remoteAddr.isUnspecified())
        throw cRuntimeError("connect: unspecified remote address");
    if (remotePort <= 0 || remotePort > 65535)
        throw cRuntimeError("connect: invalid remote port number %d", remotePort);

    SockDesc *sd = getOrCreateSocket(sockId);
    sd->remoteAddr = remoteAddr;
    sd->remotePort = remotePort;
    sd->onlyLocalPortIsSet = false;

    EV_INFO << "Socket connected: " << *sd << "\n";
}

UDP::SockDesc *UDP::createSocket(int sockId, const L3Address& localAddr, int localPort)
{
    // create and fill in SockDesc
    SockDesc *sd = new SockDesc(sockId);
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

    for (auto & elem : socketsByPortMap) {
        elem.second.clear();
    }
    socketsByPortMap.clear();
    for (auto & elem : socketsByIdMap)
        delete elem.second;
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
    for (auto sd : list) {
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
    for (auto sd : list) {
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

void UDP::sendUp(cPacket *payload, SockDesc *sd, ushort srcPort, ushort destPort)
{
    EV_INFO << "Sending payload up to socket sockId=" << sd->sockId << "\n";

    // send payload with UDPControlInfo up to the application
    payload->setKind(UDP_I_DATA);
    delete payload->removeTag<DispatchProtocolReq>();
    payload->ensureTag<SocketInd>()->setSocketId(sd->sockId);
    payload->ensureTag<TransportProtocolInd>()->setProtocol(&Protocol::udp);
    payload->ensureTag<L4PortInd>()->setSrcPort(srcPort);
    payload->ensureTag<L4PortInd>()->setDestPort(destPort);

    emit(passedUpPkSignal, payload);
    send(payload, "appOut");
    numPassedUp++;
}

void UDP::sendUpErrorIndication(SockDesc *sd, const L3Address& localAddr, ushort localPort, const L3Address& remoteAddr, ushort remotePort)
{
    cMessage *notifyMsg = new cMessage("ERROR", UDP_I_ERROR);
    UDPErrorIndication *udpCtrl = new UDPErrorIndication();
    notifyMsg->setControlInfo(udpCtrl);
    //FIXME notifyMsg->ensureTag<InterfaceInd>()->setInterfaceId(interfaceId);
    notifyMsg->ensureTag<SocketInd>()->setSocketId(sd->sockId);
    notifyMsg->ensureTag<L3AddressInd>()->setSrcAddress(localAddr);
    notifyMsg->ensureTag<L3AddressInd>()->setDestAddress(remoteAddr);
    notifyMsg->ensureTag<L4PortInd>()->setSrcPort(sd->localPort);
    notifyMsg->ensureTag<L4PortInd>()->setDestPort(remotePort);

    send(notifyMsg, "appOut");
}

UdpHeader *UDP::createUDPPacket()
{
    return new UdpHeader();
}

UDP::SockDesc *UDP::getSocketById(int sockId)
{
    auto it = socketsByIdMap.find(sockId);
    if (it == socketsByIdMap.end())
        throw cRuntimeError("socket id=%d doesn't exist (already closed?)", sockId);
    return it->second;
}

UDP::SockDesc *UDP::getOrCreateSocket(int sockId)
{
    // validate sockId
    if (sockId == -1)
        throw cRuntimeError("sockId in UDP command not filled in");

    auto it = socketsByIdMap.find(sockId);
    if (it != socketsByIdMap.end())
        return it->second;

    return createSocket(sockId, L3Address(), -1);
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

    for (auto & multicastAddresse : multicastAddresses) {
        auto it = sd->findFirstMulticastMembership(multicastAddresse);
        while (it != sd->multicastMembershipTable.end()) {
            MulticastMembership *membership = *it;
            if (membership->multicastAddress != multicastAddresse)
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
    for (auto & elem : sourceList) {
        const L3Address& sourceAddress = elem;
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
    for (auto & elem : sourceList) {
        const L3Address& sourceAddress = elem;
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
    for (auto & elem : sourceList) {
        const L3Address& sourceAddress = elem;
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
    for (auto & elem : sourceList) {
        const L3Address& sourceAddress = elem;
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

INetfilter::IHook::Result UDP::CrcInsertion::datagramPostRoutingHook(Packet *packet, const InterfaceEntry *inputInterfaceEntry, const InterfaceEntry *& outputInterfaceEntry, L3Address& nextHopAddress)
{
    auto networkProtocol = packet->getMandatoryTag<PacketProtocolTag>()->getProtocol();
    const auto& networkHeader = peekNetworkHeader(packet);
    if (networkHeader->getTransportProtocol() == IP_PROT_UDP) {
        packet->removeFromBeginning(networkHeader->getNetworkHeaderLength());
        auto udpHeader = packet->removeHeader<UdpHeader>();
        const L3Address& srcAddress = networkHeader->getSourceAddress();
        const L3Address& destAddress = networkHeader->getDestinationAddress();
        udp->insertCrc(networkProtocol, srcAddress, destAddress, udpHeader, packet);
        udpHeader->markImmutable();
        packet->pushHeader(udpHeader);
        packet->pushHeader(CHK(std::dynamic_pointer_cast<Chunk>(networkHeader)));
    }
    return ACCEPT;
}

void UDP::insertCrc(const Protocol *networkProtocol, const L3Address& srcAddress, const L3Address& destAddress, const std::shared_ptr<UdpHeader>& udpHeader, Packet *packet)
{
    udpHeader->setCrcMode(crcMode);
    switch (crcMode) {
        case CRC_DISABLED:
            // if the CRC mode is disabled, then the CRC is 0
            udpHeader->setCrc(0x0000);
            break;
        case CRC_DECLARED_CORRECT:
            // if the CRC mode is declared to be correct, then set the CRC to an easily recognizable value
            udpHeader->setCrc(0xC00D);
            break;
        case CRC_DECLARED_INCORRECT:
            // if the CRC mode is declared to be incorrect, then set the CRC to an easily recognizable value
            udpHeader->setCrc(0xBAAD);
            break;
        case CRC_COMPUTED: {
            // if the CRC mode is computed, then compute the CRC and set it
            // this computation is delayed after the routing decision, see INetfilter hook
            udpHeader->setCrc(0x0000); // make sure that the CRC is 0 in the UDP header before computing the CRC
            ByteOutputStream udpHeaderStream;
            Chunk::serialize(udpHeaderStream, udpHeader);
            auto udpHeaderBytes = udpHeaderStream.getBytes();
            auto udpDataBytes = packet->peekDataBytes()->getBytes();
            auto crc = computeCrc(networkProtocol, srcAddress, destAddress, udpHeaderBytes, udpDataBytes);
            udpHeader->setCrc(crc);
            break;
        }
        default:
            throw cRuntimeError("Unknown CRC mode");
    }
}

bool UDP::verifyCrc(const Protocol *networkProtocol, const std::shared_ptr<UdpHeader>& udpHeader, Packet *packet)
{
    switch (udpHeader->getCrcMode()) {
        case CRC_DISABLED:
            // if the CRC mode is disabled, then the check passes if the CRC is 0
            return udpHeader->getCrc() == 0x0000;
        case CRC_DECLARED_CORRECT: {
            // if the CRC mode is declared to be correct, then the check passes if and only if the chunks are correct
            auto totalLength = udpHeader->getTotalLengthField();
            auto udpDataBytes = packet->peekDataAt(byte(0), byte(totalLength) - udpHeader->getChunkLength(), Chunk::PF_ALLOW_INCORRECT);
            return udpHeader->isCorrect() && udpDataBytes->isCorrect();
        }
        case CRC_DECLARED_INCORRECT:
            // if the CRC mode is declared to be incorrect, then the check fails
            return false;
        case CRC_COMPUTED: {
            if (udpHeader->getCrc() == 0x0000)
                // if the CRC mode is computed and the CRC is 0 (disabled), then the check passes
                return true;
            else {
                // otherwise compute the CRC, the check passes if the result is 0xFFFF (includes the received CRC) and the chunks are correct
                auto l3AddressInd = packet->getMandatoryTag<L3AddressInd>();
                auto srcAddress = l3AddressInd->getSrcAddress();
                auto destAddress = l3AddressInd->getDestAddress();
                auto udpHeaderBytes = udpHeader->Chunk::peek<BytesChunk>(byte(0), udpHeader->getChunkLength())->getBytes();
                auto totalLength = udpHeader->getTotalLengthField();
                auto udpData = packet->peekDataAt<BytesChunk>(byte(0), byte(totalLength) - udpHeader->getChunkLength(), Chunk::PF_ALLOW_INCORRECT);
                auto udpDataBytes = udpData->getBytes();
                auto computedCrc = computeCrc(networkProtocol, srcAddress, destAddress, udpHeaderBytes, udpDataBytes);
                return computedCrc == 0xFFFF && udpHeader->isCorrect() && udpData->isCorrect();
            }
        }
        default:
            throw cRuntimeError("Unknown CRC mode");
    }
}

uint16_t UDP::computeCrc(const Protocol *networkProtocol, const L3Address& srcAddress, const L3Address& destAddress, const std::vector<uint8_t>& udpHeaderBytes, const std::vector<uint8_t>& udpDataBytes)
{
    auto pseudoHeader = std::make_shared<TransportPseudoHeader>();
    pseudoHeader->setSrcAddress(srcAddress);
    pseudoHeader->setDestAddress(destAddress);
    pseudoHeader->setNetworkProtocolId(networkProtocol->getId());
    pseudoHeader->setProtocolId(IP_PROT_UDP);
    pseudoHeader->setPacketLength(udpHeaderBytes.size() + udpDataBytes.size());
    // pseudoHeader length: ipv4: 12 bytes, ipv6: 40 bytes, other: ???
    if (networkProtocol == &Protocol::ipv4)
        pseudoHeader->setChunkLength(byte(12));
    else if (networkProtocol == &Protocol::ipv6)
        pseudoHeader->setChunkLength(byte(40));
    else
        throw cRuntimeError("Unknown network protocol: %s", networkProtocol->getName());
    pseudoHeader->markImmutable();
    auto pseudoHeaderBytes = pseudoHeader->Chunk::peek<BytesChunk>(byte(0), pseudoHeader->getChunkLength())->getBytes();
    // Excerpt from RFC 768:
    // Checksum is the 16-bit one's complement of the one's complement sum of a
    // pseudo header of information from the IP header, the UDP header, and the
    // data,  padded  with zero octets  at the end (if  necessary)  to  make  a
    // multiple of two octets.
    auto pseudoHeaderLength = pseudoHeaderBytes.size();
    auto udpHeaderLength = udpHeaderBytes.size();
    auto udpDataLength =  udpDataBytes.size();
    auto bufferLength = pseudoHeaderLength + udpHeaderLength + udpDataLength;
    auto buffer = new uint8_t[bufferLength];
    // 1. fill in the data
    std::copy(pseudoHeaderBytes.begin(), pseudoHeaderBytes.end(), (uint8_t *)buffer);
    std::copy(udpHeaderBytes.begin(), udpHeaderBytes.end(), (uint8_t *)buffer + pseudoHeaderLength);
    std::copy(udpDataBytes.begin(), udpDataBytes.end(), (uint8_t *)buffer + pseudoHeaderLength + udpHeaderLength);
    // 2. compute the CRC
    uint16_t crc = inet::serializer::TCPIPchecksum::checksum(buffer, bufferLength);
    delete [] buffer;
    // Excerpt from RFC 768:
    // If the computed  checksum  is zero,  it is transmitted  as all ones (the
    // equivalent  in one's complement  arithmetic).   An all zero  transmitted
    // checksum  value means that the transmitter  generated  no checksum  (for
    // debugging or for higher level protocols that don't care).
    return crc == 0x0000 ? 0xFFFF : crc;
}

} // namespace inet


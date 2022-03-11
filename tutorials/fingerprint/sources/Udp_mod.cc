//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004-2011 Andras Varga
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include <algorithm>
#include <string>

#include "inet/applications/common/SocketTag_m.h"
#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/LayeredProtocolBase.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/checksum/TcpIpChecksum.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/DscpTag_m.h"
#include "inet/networklayer/common/HopLimitTag_m.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/common/L3Tools.h"
#include "inet/networklayer/common/MulticastTag_m.h"
#include "inet/networklayer/common/TosTag_m.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/contract/IL3AddressType.h"

#ifdef WITH_IPv4
#include "inet/networklayer/ipv4/Icmp.h"
#include "inet/networklayer/ipv4/IcmpHeader.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#endif // ifdef WITH_IPv4

#ifdef WITH_IPv6
#include "inet/networklayer/icmpv6/Icmpv6.h"
#include "inet/networklayer/icmpv6/Icmpv6Header_m.h"
#include "inet/networklayer/ipv6/Ipv6ExtensionHeaders_m.h"
#include "inet/networklayer/ipv6/Ipv6Header.h"
#include "inet/networklayer/ipv6/Ipv6InterfaceData.h"
#endif // ifdef WITH_IPv6

#include "inet/transportlayer/common/L4PortTag_m.h"
#include "inet/transportlayer/common/L4Tools.h"
#include "inet/transportlayer/udp/Udp.h"
#include "inet/transportlayer/udp/UdpHeader_m.h"

namespace inet {

Define_Module(Udp);

Udp::Udp()
{
}

Udp::~Udp()
{
    clearAllSockets();
}

void Udp::initialize(int stage)
{
    OperationalBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        const char *crcModeString = par("crcMode");
        crcMode = parseCrcMode(crcModeString, true);

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

        WATCH_PTRMAP(socketsByIdMap);
        WATCH_MAP(socketsByPortMap);
    }
    else if (stage == INITSTAGE_TRANSPORT_LAYER) {
        if (crcMode == CRC_COMPUTED) {
            //TODO:
            // Unlike IPv4, when UDP packets are originated by an IPv6 node,
            // the UDP checksum is not optional.  That is, whenever
            // originating a UDP packet, an IPv6 node must compute a UDP
            // checksum over the packet and the pseudo-header, and, if that
            // computation yields a result of zero, it must be changed to hex
            // FFFF for placement in the UDP header.  IPv6 receivers must
            // discard UDP packets containing a zero checksum, and should log
            // the error.
#ifdef WITH_IPv4
            auto ipv4 = dynamic_cast<INetfilter *>(getModuleByPath("^.ipv4.ip"));
            if (ipv4 != nullptr)
                ipv4->registerHook(0, &crcInsertion);
#endif
#ifdef WITH_IPv6
            auto ipv6 = dynamic_cast<INetfilter *>(getModuleByPath("^.ipv6.ipv6"));
            if (ipv6 != nullptr)
                ipv6->registerHook(0, &crcInsertion);
#endif
        }
        registerService(Protocol::udp, gate("appIn"), gate("ipIn"));
        registerProtocol(Protocol::udp, gate("ipOut"), gate("appOut"));
    }
}

void Udp::handleSelfMessage(cMessage *message)
{
    if (message->getKind() == 42) {
        delete message;
    }
}

void Udp::handleLowerPacket(Packet *packet)
{
    scheduleAt(simTime() + 1E-6, new cMessage(nullptr, 42));
    // received from IP layer
    ASSERT(packet->getControlInfo() == nullptr);
    auto protocol = packet->getTag<PacketProtocolTag>()->getProtocol();
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

void Udp::handleUpperCommand(cMessage *msg)
{
    switch (msg->getKind()) {
        case UDP_C_BIND: {
            int socketId = check_and_cast<Request *>(msg)->getTag<SocketReq>()->getSocketId();
            UdpBindCommand *ctrl = check_and_cast<UdpBindCommand *>(msg->getControlInfo());
            bind(socketId, msg->getArrivalGate()->getIndex(), ctrl->getLocalAddr(), ctrl->getLocalPort());
            break;
        }

        case UDP_C_CONNECT: {
            int socketId = check_and_cast<Request *>(msg)->getTag<SocketReq>()->getSocketId();
            UdpConnectCommand *ctrl = check_and_cast<UdpConnectCommand *>(msg->getControlInfo());
            connect(socketId, msg->getArrivalGate()->getIndex(), ctrl->getRemoteAddr(), ctrl->getRemotePort());
            break;
        }

        case UDP_C_SETOPTION: {
            int socketId = check_and_cast<Request *>(msg)->getTag<SocketReq>()->getSocketId();
            UdpSetOptionCommand *ctrl = check_and_cast<UdpSetOptionCommand *>(msg->getControlInfo());
            SockDesc *sd = getOrCreateSocket(socketId);

            switch(ctrl->getOptionCode()) {
                case UDP_C_SETOPTION_TTL: {
                    auto cmd = check_and_cast<UdpSetTimeToLiveCommand *>(ctrl);
                    setTimeToLive(sd, cmd->getTtl());
                    break;
                }
                case UDP_C_SETOPTION_DSCP: {
                    auto cmd = check_and_cast<UdpSetDscpCommand *>(ctrl);
                    setDscp(sd, cmd->getDscp());
                    break;
                }
                case UDP_C_SETOPTION_TOS: {
                    auto cmd = check_and_cast<UdpSetTosCommand *>(ctrl);
                    setTos(sd, cmd->getTos());
                    break;
                }
                case UDP_C_SETOPTION_BROADCAST: {
                    auto cmd = check_and_cast<UdpSetBroadcastCommand *>(ctrl);
                    setBroadcast(sd, cmd->getBroadcast());
                    break;
                }
                case UDP_C_SETOPTION_MCAST_IFACE: {
                    auto cmd = check_and_cast<UdpSetMulticastInterfaceCommand *>(ctrl);
                    setMulticastOutputInterface(sd, cmd->getInterfaceId());
                    break;
                }
                case UDP_C_SETOPTION_MCAST_LOOP: {
                    auto cmd = check_and_cast<UdpSetMulticastLoopCommand *>(ctrl);
                    setMulticastLoop(sd, cmd->getLoop());
                    break;
                }
                case UDP_C_SETOPTION_REUSEADDR: {
                    auto cmd = check_and_cast<UdpSetReuseAddressCommand *>(ctrl);
                    setReuseAddress(sd, cmd->getReuseAddress());
                    break;
                }
                case UDP_C_SETOPTION_JOIN_MCAST_GRP: {
                    auto cmd = check_and_cast<UdpJoinMulticastGroupsCommand *>(ctrl);
                    std::vector<L3Address> addresses;
                    std::vector<int> interfaceIds;
                    for (size_t i = 0; i < cmd->getMulticastAddrArraySize(); i++)
                        addresses.push_back(cmd->getMulticastAddr(i));
                    for (size_t i = 0; i < cmd->getInterfaceIdArraySize(); i++)
                        interfaceIds.push_back(cmd->getInterfaceId(i));
                    joinMulticastGroups(sd, addresses, interfaceIds);
                    break;
                }
                case UDP_C_SETOPTION_LEAVE_MCAST_GRP: {
                    auto cmd = check_and_cast<UdpLeaveMulticastGroupsCommand *>(ctrl);
                    std::vector<L3Address> addresses;
                    for (size_t i = 0; i < cmd->getMulticastAddrArraySize(); i++)
                        addresses.push_back(cmd->getMulticastAddr(i));
                    leaveMulticastGroups(sd, addresses);
                    break;
                }
                case UDP_C_SETOPTION_BLOCK_MCAST_SRC: {
                    auto cmd = check_and_cast<UdpBlockMulticastSourcesCommand *>(ctrl);
                    NetworkInterface *ie = ift->getInterfaceById(cmd->getInterfaceId());
                    std::vector<L3Address> sourceList;
                    for (size_t i = 0; i < cmd->getSourceListArraySize(); i++)
                        sourceList.push_back(cmd->getSourceList(i));
                    blockMulticastSources(sd, ie, cmd->getMulticastAddr(), sourceList);
                    break;
                }
                case UDP_C_SETOPTION_UNBLOCK_MCAST_SRC: {
                    auto cmd = check_and_cast<UdpUnblockMulticastSourcesCommand *>(ctrl);
                    NetworkInterface *ie = ift->getInterfaceById(cmd->getInterfaceId());
                    std::vector<L3Address> sourceList;
                    for (size_t i = 0; i < cmd->getSourceListArraySize(); i++)
                        sourceList.push_back(cmd->getSourceList(i));
                    unblockMulticastSources(sd, ie, cmd->getMulticastAddr(), sourceList);
                    break;
                }
                case UDP_C_SETOPTION_LEAVE_MCAST_SRC: {
                    auto cmd = check_and_cast<UdpLeaveMulticastSourcesCommand *>(ctrl);
                   NetworkInterface *ie = ift->getInterfaceById(cmd->getInterfaceId());
                    std::vector<L3Address> sourceList;
                    for (size_t i = 0; i < cmd->getSourceListArraySize(); i++)
                        sourceList.push_back(cmd->getSourceList(i));
                    leaveMulticastSources(sd, ie, cmd->getMulticastAddr(), sourceList);
                    break;
                }
                case UDP_C_SETOPTION_JOIN_MCAST_SRC: {
                    auto cmd = check_and_cast<UdpJoinMulticastSourcesCommand *>(ctrl);
                    NetworkInterface *ie = ift->getInterfaceById(cmd->getInterfaceId());
                    std::vector<L3Address> sourceList;
                    for (size_t i = 0; i < cmd->getSourceListArraySize(); i++)
                        sourceList.push_back(cmd->getSourceList(i));
                    joinMulticastSources(sd, ie, cmd->getMulticastAddr(), sourceList);
                    break;
                }
                case UDP_C_SETOPTION_SET_MCAST_SRC_FILTER: {
                    auto cmd = check_and_cast<UdpSetMulticastSourceFilterCommand *>(ctrl);
                    NetworkInterface *ie = ift->getInterfaceById(cmd->getInterfaceId());
                    std::vector<L3Address> sourceList;
                    for (unsigned int i = 0; i < cmd->getSourceListArraySize(); i++)
                        sourceList.push_back(cmd->getSourceList(i));
                    setMulticastSourceFilter(sd, ie, cmd->getMulticastAddr(), cmd->getFilterMode(), sourceList);
                    break;
                }
                    throw cRuntimeError("Unknown subclass of UdpSetOptionCommand received from app: code=%d, name=%s", ctrl->getOptionCode(), ctrl->getClassName());
            }
            break;
        }

        case UDP_C_CLOSE: {
            int socketId = check_and_cast<Request *>(msg)->getTag<SocketReq>()->getSocketId();
            close(socketId);
            auto indication = new Indication("closed", UDP_I_SOCKET_CLOSED);
            auto udpCtrl = new UdpSocketClosedIndication();
            indication->setControlInfo(udpCtrl);
            indication->addTag<SocketInd>()->setSocketId(socketId);
            send(indication, "appOut");

            break;
        }

        case UDP_C_DESTROY: {
            int socketId = check_and_cast<Request *>(msg)->getTag<SocketReq>()->getSocketId();
            destroySocket(socketId);
            break;
        }

        default: {
            throw cRuntimeError("Unknown command code (message kind) %d received from app", msg->getKind());
        }
    }

    delete msg;    // also deletes control info in it
}

void Udp::bind(int sockId, int gateIndex, const L3Address& localAddr, int localPort)
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

Udp::SockDesc *Udp::findFirstSocketByLocalAddress(const L3Address& localAddr, ushort localPort)
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

Udp::SockDesc *Udp::createSocket(int sockId, const L3Address& localAddr, int localPort)
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

ushort Udp::getEphemeralPort()
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

void Udp::connect(int sockId, int gateIndex, const L3Address& remoteAddr, int remotePort)
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

Udp::SockDesc *Udp::getOrCreateSocket(int sockId)
{
    // validate sockId
    if (sockId == -1)
        throw cRuntimeError("sockId in Udp command not filled in");

    auto it = socketsByIdMap.find(sockId);
    if (it != socketsByIdMap.end())
        return it->second;

    return createSocket(sockId, L3Address(), -1);
}

// ###############################################################
// ###################### set options start ######################
// ###############################################################

void Udp::setTimeToLive(SockDesc *sd, int ttl)
{
    sd->ttl = ttl;
}

void Udp::setDscp(SockDesc *sd, short dscp)
{
    sd->dscp = dscp;
}

void Udp::setTos(SockDesc *sd, short tos)
{
    sd->tos = tos;
}

void Udp::setBroadcast(SockDesc *sd, bool broadcast)
{
    sd->isBroadcast = broadcast;
}

void Udp::setMulticastOutputInterface(SockDesc *sd, int interfaceId)
{
    sd->multicastOutputInterfaceId = interfaceId;
}

void Udp::setMulticastLoop(SockDesc *sd, bool loop)
{
    sd->multicastLoop = loop;
}

void Udp::setReuseAddress(SockDesc *sd, bool reuseAddr)
{
    sd->reuseAddr = reuseAddr;
}

void Udp::joinMulticastGroups(SockDesc *sd, const std::vector<L3Address>& multicastAddresses, const std::vector<int> interfaceIds)
{
    for (uint32_t k = 0; k < multicastAddresses.size(); k++) {
        const L3Address& multicastAddr = multicastAddresses[k];
        int interfaceId = k < interfaceIds.size() ? interfaceIds[k] : -1;
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
            NetworkInterface *ie = ift->getInterfaceById(interfaceId);
            if (!ie)
                throw cRuntimeError("Interface id=%d does not exist", interfaceId);
            ASSERT(ie->isMulticast());
            addMulticastAddressToInterface(ie, multicastAddr);
        }
        else {
            for (int i = 0; i < ift->getNumInterfaces(); i++) {
                NetworkInterface *ie = ift->getInterface(i);
                if (ie->isMulticast())
                    addMulticastAddressToInterface(ie, multicastAddr);
            }
        }
    }
}

void Udp::addMulticastAddressToInterface(NetworkInterface *ie, const L3Address& multicastAddr)
{
    ASSERT(ie && ie->isMulticast());
    ASSERT(multicastAddr.isMulticast());

    if (multicastAddr.getType() == L3Address::IPv4) {
#ifdef WITH_IPv4
        ie->getProtocolData<Ipv4InterfaceData>()->joinMulticastGroup(multicastAddr.toIpv4());
#endif // ifdef WITH_IPv4
    }
    else if (multicastAddr.getType() == L3Address::IPv6) {
#ifdef WITH_IPv6
        ie->getProtocolData<Ipv6InterfaceData>()->assignAddress(multicastAddr.toIpv6(), false, SimTime::getMaxTime(), SimTime::getMaxTime());
#endif // ifdef WITH_IPv6
    }
    else
        ie->joinMulticastGroup(multicastAddr);
}

void Udp::leaveMulticastGroups(SockDesc *sd, const std::vector<L3Address>& multicastAddresses)
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
                NetworkInterface *ie = ift->getInterfaceById(membership->interfaceId);
                ie->changeMulticastGroupMembership(membership->multicastAddress,
                        oldFilterMode, membership->sourceList, MCAST_INCLUDE_SOURCES, empty);
            }
            else {
                for (int j = 0; j < ift->getNumInterfaces(); ++j) {
                    NetworkInterface *ie = ift->getInterface(j);
                    if (ie->isMulticast())
                        ie->changeMulticastGroupMembership(membership->multicastAddress,
                                oldFilterMode, membership->sourceList, MCAST_INCLUDE_SOURCES, empty);
                }
            }

            delete membership;
        }
    }
}

void Udp::blockMulticastSources(SockDesc *sd, NetworkInterface *ie, L3Address multicastAddress, const std::vector<L3Address>& sourceList)
{
    ASSERT(ie && ie->isMulticast());
    ASSERT(multicastAddress.isMulticast());

    MulticastMembership *membership = sd->findMulticastMembership(multicastAddress, ie->getInterfaceId());
    if (!membership)
        throw cRuntimeError("Udp::blockMulticastSources(): not a member of %s group on interface '%s'",
                multicastAddress.str().c_str(), ie->getFullName());

    if (membership->filterMode != UDP_EXCLUDE_MCAST_SOURCES)
        throw cRuntimeError("Udp::blockMulticastSources(): socket was not joined to all sources of %s group on interface '%s'",
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

void Udp::unblockMulticastSources(SockDesc *sd, NetworkInterface *ie, L3Address multicastAddress, const std::vector<L3Address>& sourceList)
{
    ASSERT(ie && ie->isMulticast());
    ASSERT(multicastAddress.isMulticast());

    MulticastMembership *membership = sd->findMulticastMembership(multicastAddress, ie->getInterfaceId());
    if (!membership)
        throw cRuntimeError("Udp::unblockMulticastSources(): not a member of %s group in interface '%s'",
                multicastAddress.str().c_str(), ie->getFullName());

    if (membership->filterMode != UDP_EXCLUDE_MCAST_SOURCES)
        throw cRuntimeError("Udp::unblockMulticastSources(): socket was not joined to all sources of %s group on interface '%s'",
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

void Udp::leaveMulticastSources(SockDesc *sd, NetworkInterface *ie, L3Address multicastAddress, const std::vector<L3Address>& sourceList)
{
    ASSERT(ie && ie->isMulticast());
    ASSERT(multicastAddress.isMulticast());

    MulticastMembership *membership = sd->findMulticastMembership(multicastAddress, ie->getInterfaceId());
    if (!membership)
        throw cRuntimeError("Udp::leaveMulticastSources(): not a member of %s group in interface '%s'",
                multicastAddress.str().c_str(), ie->getFullName());

    if (membership->filterMode == UDP_EXCLUDE_MCAST_SOURCES)
        throw cRuntimeError("Udp::leaveMulticastSources(): socket was joined to all sources of %s group on interface '%s'",
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

void Udp::joinMulticastSources(SockDesc *sd, NetworkInterface *ie, L3Address multicastAddress, const std::vector<L3Address>& sourceList)
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
        throw cRuntimeError("Udp::joinMulticastSources(): socket was joined to all sources of %s group on interface '%s'",
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

void Udp::setMulticastSourceFilter(SockDesc *sd, NetworkInterface *ie, L3Address multicastAddress, UdpSourceFilterMode filterMode, const std::vector<L3Address>& sourceList)
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

// ###############################################################
// ####################### set options end #######################
// ###############################################################

void Udp::handleUpperPacket(Packet *packet)
{
    if (packet->getKind() != UDP_C_DATA)
        throw cRuntimeError("Unknown packet command code (message kind) %d received from app", packet->getKind());

    emit(packetReceivedFromUpperSignal, packet);
    L3Address srcAddr, destAddr;
    int srcPort = -1, destPort = -1;

    auto socketReq = packet->removeTag<SocketReq>();
    int socketId = socketReq->getSocketId();
    delete socketReq;

    SockDesc *sd = getOrCreateSocket(socketId);

    auto addressReq = packet->addTagIfAbsent<L3AddressReq>();
    srcAddr = addressReq->getSrcAddress();
    destAddr = addressReq->getDestAddress();

    if (srcAddr.isUnspecified())
        addressReq->setSrcAddress(srcAddr = sd->localAddr);

    if (destAddr.isUnspecified())
        addressReq->setDestAddress(destAddr = sd->remoteAddr);

    if (auto portsReq = packet->removeTagIfPresent<L4PortReq>()) {
        srcPort = portsReq->getSrcPort();
        destPort = portsReq->getDestPort();
        delete portsReq;
    }

    if (srcPort == -1)
        srcPort = sd->localPort;

    if (destPort == -1)
        destPort = sd->remotePort;

    auto interfaceReq = packet->findTag<InterfaceReq>();
    ASSERT(interfaceReq == nullptr || interfaceReq->getInterfaceId() != -1);

    if (interfaceReq == nullptr && destAddr.isMulticast()) {
        auto membership = sd->findFirstMulticastMembership(destAddr);
        int interfaceId = (membership != sd->multicastMembershipTable.end() && (*membership)->interfaceId != -1) ? (*membership)->interfaceId : sd->multicastOutputInterfaceId;
        if (interfaceId != -1)
            packet->addTagIfAbsent<InterfaceReq>()->setInterfaceId(interfaceId);
    }

    if (addressReq->getDestAddress().isUnspecified())
        throw cRuntimeError("send: unspecified destination address");

    if (destPort <= 0 || destPort > 65535)
        throw cRuntimeError("send: invalid remote port number %d", destPort);

    if (packet->findTag<MulticastReq>() == nullptr)
        packet->addTag<MulticastReq>()->setMulticastLoop(sd->multicastLoop);

    if (sd->ttl != -1 && packet->findTag<HopLimitReq>() == nullptr)
        packet->addTag<HopLimitReq>()->setHopLimit(sd->ttl);

    if (sd->dscp != -1 && packet->findTag<DscpReq>() == nullptr)
        packet->addTag<DscpReq>()->setDifferentiatedServicesCodePoint(sd->dscp);

    if (sd->tos != -1 && packet->findTag<TosReq>() == nullptr) {
        packet->addTag<TosReq>()->setTos(sd->tos);
        if (packet->findTag<DscpReq>())
            throw cRuntimeError("setting error: TOS and DSCP found together");
    }

    const Protocol *l3Protocol = nullptr;
    // TODO: apps use ModuleIdAddress if the network interface doesn't have an IP address configured, and UDP uses NextHopForwarding which results in a weird error in MessageDispatcher
    if (destAddr.getType() == L3Address::IPv4)
        l3Protocol = &Protocol::ipv4;
    else if (destAddr.getType() == L3Address::IPv6)
        l3Protocol = &Protocol::ipv6;
    else
        l3Protocol = &Protocol::nextHopForwarding;

    auto udpHeader = makeShared<UdpHeader>();
    // set source and destination port
    udpHeader->setSourcePort(srcPort);
    udpHeader->setDestinationPort(destPort);

    B totalLength = udpHeader->getChunkLength() + packet->getTotalLength();
    if(totalLength.get() > UDP_MAX_MESSAGE_SIZE)
        throw cRuntimeError("send: total UDP message size exceeds %u", UDP_MAX_MESSAGE_SIZE);

    udpHeader->setTotalLengthField(totalLength);
    if (crcMode == CRC_COMPUTED) {
        udpHeader->setCrcMode(CRC_COMPUTED);
        udpHeader->setCrc(0x0000);    // crcMode == CRC_COMPUTED is done in an INetfilter hook
    }
    else {
        udpHeader->setCrcMode(crcMode);
        insertCrc(l3Protocol, srcAddr, destAddr, udpHeader, packet);
    }

    insertTransportProtocolHeader(packet, Protocol::udp, udpHeader);
    packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(l3Protocol);
    packet->setKind(0);

    EV_INFO << "Sending app packet " << packet->getName() << " over " << l3Protocol->getName() << ".\n";
    emit(packetSentSignal, packet);
    emit(packetSentToLowerSignal, packet);
    send(packet, "ipOut");
    numSent++;
}

void Udp::insertCrc(const Protocol *networkProtocol, const L3Address& srcAddress, const L3Address& destAddress, const Ptr<UdpHeader>& udpHeader, Packet *packet)
{
    CrcMode crcMode = udpHeader->getCrcMode();
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
            udpHeader->setCrc(0x0000); // make sure that the CRC is 0 in the Udp header before computing the CRC
            udpHeader->setCrcMode(CRC_DISABLED);    // for serializer/deserializer checks only: deserializer sets the crcMode to disabled when crc is 0
            auto udpData = packet->peekData(Chunk::PF_ALLOW_EMPTY);
            auto crc = computeCrc(networkProtocol, srcAddress, destAddress, udpHeader, udpData);
            udpHeader->setCrc(crc);
            udpHeader->setCrcMode(CRC_COMPUTED);
            break;
        }
        default:
            throw cRuntimeError("Unknown CRC mode: %d", (int)crcMode);
    }
}

uint16_t Udp::computeCrc(const Protocol *networkProtocol, const L3Address& srcAddress, const L3Address& destAddress, const Ptr<const UdpHeader>& udpHeader, const Ptr<const Chunk>& udpData)
{
    auto pseudoHeader = makeShared<TransportPseudoHeader>();
    pseudoHeader->setSrcAddress(srcAddress);
    pseudoHeader->setDestAddress(destAddress);
    pseudoHeader->setNetworkProtocolId(networkProtocol->getId());
    pseudoHeader->setProtocolId(IP_PROT_UDP);
    pseudoHeader->setPacketLength(udpHeader->getChunkLength() + udpData->getChunkLength());
    // pseudoHeader length: ipv4: 12 bytes, ipv6: 40 bytes, other: ???
    if (networkProtocol == &Protocol::ipv4)
        pseudoHeader->setChunkLength(B(12));
    else if (networkProtocol == &Protocol::ipv6)
        pseudoHeader->setChunkLength(B(40));
    else
        throw cRuntimeError("Unknown network protocol: %s", networkProtocol->getName());

    MemoryOutputStream stream;
    Chunk::serialize(stream, pseudoHeader);
    Chunk::serialize(stream, udpHeader);
    Chunk::serialize(stream, udpData);
    uint16_t crc = TcpIpChecksum::checksum(stream.getData());

    // Excerpt from RFC 768:
    // If the computed  checksum  is zero,  it is transmitted  as all ones (the
    // equivalent  in one's complement  arithmetic).   An all zero  transmitted
    // checksum  value means that the transmitter  generated  no checksum  (for
    // debugging or for higher level protocols that don't care).
    return crc == 0x0000 ? 0xFFFF : crc;
}

void Udp::close(int sockId)
{
    // remove from socketsByIdMap
    auto it = socketsByIdMap.find(sockId);
    if (it == socketsByIdMap.end()) {
        EV_ERROR << "socket id=" << sockId << " doesn't exist (already closed?)\n";
        return;
    }

    EV_INFO << "Closing socket: " << *(it->second) << "\n";

    destroySocket(it);
}

void Udp::destroySocket(SocketsByIdMap::iterator it)
{
    SockDesc *sd = it->second;
    socketsByIdMap.erase(it);

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

void Udp::destroySocket(int sockId)
{
    // remove from socketsByIdMap
    auto it = socketsByIdMap.find(sockId);
    if (it == socketsByIdMap.end()) {
        EV_WARN << "socket id=" << sockId << " doesn't exist\n";
        return;
    }
    EV_INFO << "Destroy socket: " << *(it->second) << "\n";

    destroySocket(it);
}

void Udp::processUDPPacket(Packet *udpPacket)
{
    ASSERT(udpPacket->getControlInfo() == nullptr);
    emit(packetReceivedFromLowerSignal, udpPacket);
    emit(packetReceivedSignal, udpPacket);

    delete udpPacket->removeTag<PacketProtocolTag>();
    b udpHeaderPopPosition = udpPacket->getFrontOffset();
    auto udpHeader = udpPacket->popAtFront<UdpHeader>(b(-1), Chunk::PF_ALLOW_INCORRECT);

    // simulate checksum: discard packet if it has bit error
    EV_INFO << "Packet " << udpPacket->getName() << " received from network, dest port " << udpHeader->getDestinationPort() << "\n";

    auto srcPort = udpHeader->getSourcePort();
    auto destPort = udpHeader->getDestinationPort();
    auto l3AddressInd = udpPacket->getTag<L3AddressInd>();
    auto srcAddr = l3AddressInd->getSrcAddress();
    auto destAddr = l3AddressInd->getDestAddress();
    auto totalLength = B(udpHeader->getTotalLengthField());
    auto hasIncorrectLength = totalLength < udpHeader->getChunkLength() || totalLength > udpHeader->getChunkLength() + udpPacket->getDataLength();
    auto networkProtocol = udpPacket->getTag<NetworkProtocolInd>()->getProtocol();

    if (hasIncorrectLength || !verifyCrc(networkProtocol, udpHeader, udpPacket)) {
        EV_WARN << "Packet has bit error, discarding\n";
        PacketDropDetails details;
        details.setReason(INCORRECTLY_RECEIVED);
        emit(packetDroppedSignal, udpPacket, &details);
        numDroppedBadChecksum++;
        delete udpPacket;
        return;
    }

    // remove lower layer paddings:
    if (totalLength < udpPacket->getDataLength()) {
        udpPacket->setBackOffset(udpHeaderPopPosition + totalLength);
    }

    bool isMulticast = destAddr.isMulticast();
    bool isBroadcast = destAddr.isBroadcast();
    if (!isMulticast && !isBroadcast) {
        // unicast packet, there must be only one socket listening
        SockDesc *sd = findSocketForUnicastPacket(destAddr, destPort, srcAddr, srcPort);
        if (!sd) {
            EV_WARN << "No socket registered on port " << destPort << "\n";
            udpPacket->setFrontOffset(udpHeaderPopPosition);
            processUndeliverablePacket(udpPacket);
            return;
        }
        else {
            sendUp(udpHeader, udpPacket, sd, srcPort, destPort);
        }
    }
    else {
        // multicast packet: find all matching sockets, and send up a copy to each
        std::vector<SockDesc *> sds = findSocketsForMcastBcastPacket(destAddr, destPort, srcAddr, srcPort, isMulticast, isBroadcast);
        if (sds.empty()) {
            EV_WARN << "No socket registered on port " << destPort << "\n";
            udpPacket->setFrontOffset(udpHeaderPopPosition);
            processUndeliverablePacket(udpPacket);
            return;
        }
        else {
            unsigned int i;
            for (i = 0; i < sds.size() - 1; i++) // sds.size() >= 1
                sendUp(udpHeader, udpPacket->dup(), sds[i], srcPort, destPort); // dup() to all but the last one
            sendUp(udpHeader, udpPacket, sds[i], srcPort, destPort);    // send original to last socket
        }
    }
}

bool Udp::verifyCrc(const Protocol *networkProtocol, const Ptr<const UdpHeader>& udpHeader, Packet *packet)
{
    switch (udpHeader->getCrcMode()) {
        case CRC_DISABLED:
            // if the CRC mode is disabled, then the check passes if the CRC is 0
            return udpHeader->getCrc() == 0x0000;
        case CRC_DECLARED_CORRECT: {
            // if the CRC mode is declared to be correct, then the check passes if and only if the chunks are correct
            auto totalLength = udpHeader->getTotalLengthField();
            auto udpDataBytes = packet->peekDataAt(B(0), totalLength - udpHeader->getChunkLength(), Chunk::PF_ALLOW_INCORRECT);
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
                auto l3AddressInd = packet->getTag<L3AddressInd>();
                auto srcAddress = l3AddressInd->getSrcAddress();
                auto destAddress = l3AddressInd->getDestAddress();
                auto totalLength = udpHeader->getTotalLengthField();
                auto udpData = packet->peekDataAt<BytesChunk>(B(0), totalLength - udpHeader->getChunkLength(), Chunk::PF_ALLOW_INCORRECT);
                auto computedCrc = computeCrc(networkProtocol, srcAddress, destAddress, udpHeader, udpData);
                // TODO: delete these isCorrect calls, rely on CRC only
                return computedCrc == 0xFFFF && udpHeader->isCorrect() && udpData->isCorrect();
            }
        }
        default:
            throw cRuntimeError("Unknown CRC mode");
    }
}

Udp::SockDesc *Udp::findSocketForUnicastPacket(const L3Address& localAddr, ushort localPort, const L3Address& remoteAddr, ushort remotePort)
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

std::vector<Udp::SockDesc *> Udp::findSocketsForMcastBcastPacket(const L3Address& localAddr, ushort localPort, const L3Address& remoteAddr, ushort remotePort, bool isMulticast, bool isBroadcast)
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

void Udp::processUndeliverablePacket(Packet *udpPacket)
{
    const auto& udpHeader = udpPacket->peekAtFront<UdpHeader>();
    PacketDropDetails details;
    details.setReason(NO_PORT_FOUND);
    emit(packetDroppedSignal, udpPacket, &details);
    numDroppedWrongPort++;

    // send back ICMP PORT_UNREACHABLE
    char buff[80];
    snprintf(buff, sizeof(buff), "Port %d unreachable", udpHeader->getDestinationPort());
    udpPacket->setName(buff);
    const Protocol *protocol = udpPacket->getTag<NetworkProtocolInd>()->getProtocol();

    if (protocol == nullptr) {
        throw cRuntimeError("(%s)%s arrived from lower layer without NetworkProtocolInd",
                udpPacket->getClassName(), udpPacket->getName());
    }

    // push back network protocol header
    udpPacket->trim();
    udpPacket->insertAtFront(udpPacket->getTag<NetworkProtocolInd>()->getNetworkProtocolHeader());
    auto inIe = udpPacket->getTag<InterfaceInd>()->getInterfaceId();

    if (protocol->getId() == Protocol::ipv4.getId()) {
#ifdef WITH_IPv4
        if (!icmp)
            // TODO: move to initialize?
            icmp = getModuleFromPar<Icmp>(par("icmpModule"), this);
        icmp->sendErrorMessage(udpPacket, inIe, ICMP_DESTINATION_UNREACHABLE, ICMP_DU_PORT_UNREACHABLE);
#else // ifdef WITH_IPv4
        delete udpPacket;
#endif // ifdef WITH_IPv4
    }
    else if (protocol->getId() == Protocol::ipv6.getId()) {
#ifdef WITH_IPv6
        if (!icmpv6)
            // TODO: move to initialize?
            icmpv6 = getModuleFromPar<Icmpv6>(par("icmpv6Module"), this);
        icmpv6->sendErrorMessage(udpPacket, ICMPv6_DESTINATION_UNREACHABLE, PORT_UNREACHABLE);
#else // ifdef WITH_IPv6
        delete udpPacket;
#endif // ifdef WITH_IPv6
    }
    else if (protocol->getId() == Protocol::nextHopForwarding.getId()) {
        delete udpPacket;
    }
    else {
        throw cRuntimeError("(%s)%s arrived from lower layer with unrecognized NetworkProtocolInd %s",
                udpPacket->getClassName(), udpPacket->getName(), protocol->getName());
    }
}

void Udp::sendUp(Ptr<const UdpHeader>& header, Packet *payload, SockDesc *sd, ushort srcPort, ushort destPort)
{
    EV_INFO << "Sending payload up to socket sockId=" << sd->sockId << "\n";

    // send payload with UdpControlInfo up to the application
    payload->setKind(UDP_I_DATA);
    delete payload->removeTagIfPresent<PacketProtocolTag>();
    delete payload->removeTagIfPresent<DispatchProtocolReq>();
    payload->addTagIfAbsent<SocketInd>()->setSocketId(sd->sockId);
    payload->addTagIfAbsent<TransportProtocolInd>()->setProtocol(&Protocol::udp);
    payload->addTagIfAbsent<TransportProtocolInd>()->setTransportProtocolHeader(header);
    payload->addTagIfAbsent<L4PortInd>()->setSrcPort(srcPort);
    payload->addTagIfAbsent<L4PortInd>()->setDestPort(destPort);

    emit(packetSentToUpperSignal, payload);
    send(payload, "appOut");
    numPassedUp++;
}

void Udp::processICMPv4Error(Packet *packet)
{
#ifdef WITH_IPv4
    // extract details from the error message, then try to notify socket that sent bogus packet

    if (!icmp)
        // TODO: move to initialize?
        icmp = getModuleFromPar<Icmp>(par("icmpModule"), this);
    if (!icmp->verifyCrc(packet)) {
        EV_WARN << "incoming ICMP packet has wrong CRC, dropped\n";
        PacketDropDetails details;
        details.setReason(INCORRECTLY_RECEIVED);
        emit(packetDroppedSignal, packet, &details);
        delete packet;
        return;
    }
    int type, code;
    L3Address localAddr, remoteAddr;
    int localPort = -1, remotePort = -1;
    bool udpHeaderAvailable = false;

    const auto& icmpHeader = packet->popAtFront<IcmpHeader>();
    ASSERT(icmpHeader);
    type = icmpHeader->getType();
    code = icmpHeader->getCode();
    const auto& ipv4Header = packet->popAtFront<Ipv4Header>();
    if (ipv4Header->getDontFragment() || ipv4Header->getFragmentOffset() == 0) {
        const auto& udpHeader = packet->peekAtFront<UdpHeader>(B(8), Chunk::PF_ALLOW_INCOMPLETE);
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
        EV_WARN << "Udp header not available, ignoring ICMP error\n";
#endif // ifdef WITH_IPv4

    delete packet;
}

void Udp::processICMPv6Error(Packet *packet)
{
#ifdef WITH_IPv6
    if (!icmpv6)
        // TODO: move to initialize?
        icmpv6 = getModuleFromPar<Icmpv6>(par("icmpv6Module"), this);
    if (!icmpv6->verifyCrc(packet)) {
        EV_WARN << "incoming ICMPv6 packet has wrong CRC, dropped\n";
        PacketDropDetails details;
        details.setReason(INCORRECTLY_RECEIVED);
        emit(packetDroppedSignal, packet, &details);
        delete packet;
        return;
    }

    // extract details from the error message, then try to notify socket that sent bogus packet
    int type, code;
    L3Address localAddr, remoteAddr;
    ushort localPort, remotePort;
    bool udpHeaderAvailable = false;

    const auto& icmpHeader = packet->popAtFront<Icmpv6Header>();
    ASSERT(icmpHeader);

    type = icmpHeader->getType();
    code = -1;    // FIXME this is dependent on getType()...
    // Note: we must NOT use decapsulate() because payload in ICMP is conceptually truncated
    const auto& ipv6Header = packet->popAtFront<Ipv6Header>();
    const Ipv6FragmentHeader *fh = dynamic_cast<const Ipv6FragmentHeader *>(ipv6Header->findExtensionHeaderByType(IP_PROT_IPv6EXT_FRAGMENT));
    if (!fh || fh->getFragmentOffset() == 0) {
        const auto& udpHeader = packet->peekAtFront<UdpHeader>(B(8), Chunk::PF_ALLOW_INCOMPLETE);
        localAddr = ipv6Header->getSrcAddress();
        remoteAddr = ipv6Header->getDestAddress();
        localPort = udpHeader->getSourcePort();
        remotePort = udpHeader->getDestinationPort();
        udpHeaderAvailable = true;
    }

    // identify socket and report error to it
    if (udpHeaderAvailable) {
        EV_WARN << "ICMP error received: type=" << type << " code=" << code
                << " about packet " << localAddr << ":" << localPort << " > "
                << remoteAddr << ":" << remotePort << "\n";

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
        EV_WARN << "Udp header not available, ignoring ICMPv6 error\n";

#endif // ifdef WITH_IPv6

    delete packet;
}

void Udp::sendUpErrorIndication(SockDesc *sd, const L3Address& localAddr, ushort localPort, const L3Address& remoteAddr, ushort remotePort)
{
    auto indication = new Indication("ERROR", UDP_I_ERROR);
    UdpErrorIndication *udpCtrl = new UdpErrorIndication();
    indication->setControlInfo(udpCtrl);
    //FIXME notifyMsg->addTagIfAbsent<InterfaceInd>()->setInterfaceId(interfaceId);
    indication->addTag<SocketInd>()->setSocketId(sd->sockId);
    auto addresses = indication->addTag<L3AddressInd>();
    addresses->setSrcAddress(localAddr);
    addresses->setDestAddress(remoteAddr);
    auto ports = indication->addTag<L4PortInd>();
    ports->setSrcPort(sd->localPort);
    ports->setDestPort(remotePort);

    send(indication, "appOut");
}

// #############################
// life cycle
// #############################

void Udp::handleStartOperation(LifecycleOperation *operation)
{
    icmp = nullptr;
    icmpv6 = nullptr;
}

void Udp::handleStopOperation(LifecycleOperation *operation)
{
    clearAllSockets();
    icmp = nullptr;
    icmpv6 = nullptr;
}

void Udp::handleCrashOperation(LifecycleOperation *operation)
{
    clearAllSockets();
    icmp = nullptr;
    icmpv6 = nullptr;
}

void Udp::clearAllSockets()
{
    EV_INFO << "Clear all sockets\n";

    for (auto & elem : socketsByIdMap)
        delete elem.second;

    socketsByIdMap.clear();
    socketsByPortMap.clear();
}

// #############################
// other UDP methods
// #############################

void Udp::refreshDisplay() const
{
    OperationalBase::refreshDisplay();

    char buf[80];
    sprintf(buf, "passed up: %d pks\nsent: %d pks", numPassedUp, numSent);
    if (numDroppedWrongPort > 0) {
        sprintf(buf + strlen(buf), "\ndropped (no app): %d pks", numDroppedWrongPort);
        getDisplayString().setTagArg("i", 1, "red");
    }
    getDisplayString().setTagArg("t", 0, buf);
}

// used in UdpProtocolDissector
bool Udp::isCorrectPacket(Packet *packet, const Ptr<const UdpHeader>& udpHeader)
{
    auto trailerPopOffset = packet->getBackOffset();
    auto udpHeaderOffset = packet->getFrontOffset() - udpHeader->getChunkLength();
    if (udpHeader->getTotalLengthField() < UDP_HEADER_LENGTH)
        return false;
    else if (B(udpHeader->getTotalLengthField()) > trailerPopOffset - udpHeaderOffset)
        return false;
    else {
        auto l3AddressInd = packet->findTag<L3AddressInd>();
        auto networkProtocolInd = packet->findTag<NetworkProtocolInd>();
        if (l3AddressInd != nullptr && networkProtocolInd != nullptr)
            return verifyCrc(networkProtocolInd->getProtocol(), udpHeader, packet);
        else
            return udpHeader->getCrcMode() != CrcMode::CRC_DECLARED_INCORRECT;
    }
}

// #######################
// Udp::SockDesc
// #######################

Udp::SockDesc::SockDesc(int sockId_)
{
    sockId = sockId_;
}

Udp::SockDesc::~SockDesc()
{
    for(auto & elem : multicastMembershipTable)
        delete (elem);
}

/*
 * Multicast memberships are sorted first by multicastAddress, then by interfaceId.
 * The interfaceId -1 comes after any other interfaceId.
 */
static bool lessMembership(const Udp::MulticastMembership *first, const Udp::MulticastMembership *second)
{
    if (first->multicastAddress != second->multicastAddress)
        return first->multicastAddress < second->multicastAddress;

    if (first->interfaceId == -1 || first->interfaceId >= second->interfaceId)
        return false;

    return true;
}

Udp::MulticastMembershipTable::iterator Udp::SockDesc::findFirstMulticastMembership(const L3Address& multicastAddress)
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

Udp::MulticastMembership *Udp::SockDesc::findMulticastMembership(const L3Address& multicastAddress, int interfaceId)
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

void Udp::SockDesc::addMulticastMembership(MulticastMembership *membership)
{
    auto it = lower_bound(multicastMembershipTable.begin(), multicastMembershipTable.end(), membership, lessMembership);
    multicastMembershipTable.insert(it, membership);
}

void Udp::SockDesc::deleteMulticastMembership(MulticastMembership *membership)
{
    multicastMembershipTable.erase(std::remove(multicastMembershipTable.begin(), multicastMembershipTable.end(), membership),
            multicastMembershipTable.end());
    delete membership;
}

std::ostream& operator<<(std::ostream& os, const Udp::SockDesc& sd)
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

std::ostream& operator<<(std::ostream& os, const Udp::SockDescList& list)
{
    for (const auto & elem : list)
        os << "sockId=" << (elem)->sockId << " ";
    return os;
}

// ######################
// other methods
// ######################

INetfilter::IHook::Result Udp::CrcInsertion::datagramPostRoutingHook(Packet *packet)
{
    if (packet->findTag<InterfaceInd>())
        return ACCEPT;  // FORWARD

    auto networkProtocol = packet->getTag<PacketProtocolTag>()->getProtocol();
    const auto& networkHeader = getNetworkProtocolHeader(packet);
    if (networkHeader->getProtocol() == &Protocol::udp) {
        ASSERT(!networkHeader->isFragment());
        packet->eraseAtFront(networkHeader->getChunkLength());
        auto udpHeader = packet->removeAtFront<UdpHeader>();
        ASSERT(udpHeader->getCrcMode() == CRC_COMPUTED);
        const L3Address& srcAddress = networkHeader->getSourceAddress();
        const L3Address& destAddress = networkHeader->getDestinationAddress();
        Udp::insertCrc(networkProtocol, srcAddress, destAddress, udpHeader, packet);
        packet->insertAtFront(udpHeader);
        packet->insertAtFront(networkHeader);
    }

    return ACCEPT;
}

bool Udp::MulticastMembership::isSourceAllowed(L3Address sourceAddr)
{
    auto it = std::find(sourceList.begin(), sourceList.end(), sourceAddr);
    return (filterMode == UDP_INCLUDE_MCAST_SOURCES && it != sourceList.end()) ||
           (filterMode == UDP_EXCLUDE_MCAST_SOURCES && it == sourceList.end());
}

// unused functions!

UdpHeader *Udp::createUDPPacket()
{
    return new UdpHeader();
}

Udp::SockDesc *Udp::getSocketById(int sockId)
{
    auto it = socketsByIdMap.find(sockId);
    if (it == socketsByIdMap.end())
        throw cRuntimeError("socket id=%d doesn't exist (already closed?)", sockId);
    return it->second;
}

} // namespace inet

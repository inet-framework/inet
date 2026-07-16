//
// Copyright (C) 2005,2011 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/Message.h"
#include "inet/common/socket/SocketTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/DscpTag_m.h"
#include "inet/networklayer/common/HopLimitTag_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/common/TosTag_m.h"

#ifdef INET_WITH_IPv4
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#endif // ifdef INET_WITH_IPv4

#include "inet/transportlayer/common/L4PortTag_m.h"
#include "inet/transportlayer/contract/udp/UdpCommand_m.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"
#include "inet/common/SimulationContinuation.h"

namespace inet {

UdpSocket::UdpSocket()
{
    // don't allow user-specified socketIds because they may conflict with
    // automatically assigned ones.
    socketId = getActiveSimulationOrEnvir()->getUniqueNumber();
}

void UdpSocket::bind(int localPort)
{
    bind(L3Address(), localPort);
}

void UdpSocket::bind(L3Address localAddr, int localPort)
{
    if (localPort < -1 || localPort > 65535) // -1: ephemeral port
        throw cRuntimeError("UdpSocket::bind(): invalid port number %d", localPort);
    EV_INFO << "Binding socket" << EV_FIELD(socketId) << EV_FIELD(localAddr) << EV_FIELD(localPort) << EV_ENDL;
    udp->bind(socketId, localAddr, localPort);
    udp->setCallback(socketId, this);
    sockState = CONNECTED;
}

void UdpSocket::connect(L3Address addr, int port)
{
    if (addr.isUnspecified())
        throw cRuntimeError("UdpSocket::connect(): unspecified remote address");
    if (port <= 0 || port > 65535)
        throw cRuntimeError("UdpSocket::connect(): invalid remote port number %d", port);
    EV_INFO << "Connecting socket" << EV_FIELD(socketId) << EV_FIELD(addr) << EV_FIELD(port) << EV_ENDL;
    udp->connect(socketId, addr, port);
    udp->setCallback(socketId, this);
    sockState = CONNECTED;
}

void UdpSocket::sendTo(Packet *pk, L3Address destAddr, int destPort)
{
    pk->setKind(UDP_C_DATA);
    auto addressReq = pk->addTagIfAbsent<L3AddressReq>();
    addressReq->setDestAddress(destAddr);
    if (destPort != -1)
        pk->addTagIfAbsent<L4PortReq>()->setDestPort(destPort);
    pk->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&Protocol::udp);
    pk->addTagIfAbsent<SocketReq>()->setSocketId(socketId);
    yieldBeforePush();
    sink.pushPacket(pk);
    sockState = CONNECTED;
}

void UdpSocket::send(Packet *pk)
{
    pk->setKind(UDP_C_DATA);
    pk->addTagIfAbsent<SocketReq>()->setSocketId(socketId);
    pk->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&Protocol::udp);
    EV_INFO << "Sending packet on socket" << EV_FIELD(socketId) << EV_FIELD(pk) << EV_ENDL;
    yieldBeforePush();
    sink.pushPacket(pk);
    sockState = CONNECTED;
}

void UdpSocket::close()
{
    if (sockState == CLOSED)
        return;
    // local state first: the module call completes the close synchronously,
    // and the application may even delete this socket from the callback
    sockState = CLOSED;
    udp->close(socketId);
}

void UdpSocket::destroy()
{
    if (this->gateToUdp == nullptr)
        return;
    udp->destroy(socketId);
}

// ########################
// UDP Socket Options Start
// ########################

void UdpSocket::setTimeToLive(int ttl)
{
    udp->setTimeToLive(socketId, ttl);
}

void UdpSocket::setDscp(short dscp)
{
    udp->setDscp(socketId, dscp);
}

void UdpSocket::setTos(short tos)
{
    udp->setTos(socketId, tos);
}

void UdpSocket::setBroadcast(bool broadcast)
{
    udp->setBroadcast(socketId, broadcast);
}

void UdpSocket::setMulticastOutputInterface(int interfaceId)
{
    udp->setMulticastOutputInterface(socketId, interfaceId);
}

void UdpSocket::setMulticastLoop(bool value)
{
    EV_INFO << "Setting multicast loop" << EV_FIELD(socketId) << EV_FIELD(value) << EV_ENDL;
    udp->setMulticastLoop(socketId, value);
}

void UdpSocket::setReuseAddress(bool value)
{
    udp->setReuseAddress(socketId, value);
}

void UdpSocket::joinMulticastGroup(const L3Address& multicastAddr, int interfaceId)
{
    udp->joinMulticastGroups(socketId, {multicastAddr}, {interfaceId});
}

void UdpSocket::leaveMulticastGroup(const L3Address& multicastAddr)
{
    udp->leaveMulticastGroups(socketId, {multicastAddr});
}

void UdpSocket::blockMulticastSources(int interfaceId, const L3Address& multicastAddr, const std::vector<L3Address>& sourceList)
{
    udp->blockMulticastSources(socketId, interfaceId, multicastAddr, sourceList);
}

void UdpSocket::unblockMulticastSources(int interfaceId, const L3Address& multicastAddr, const std::vector<L3Address>& sourceList)
{
    udp->unblockMulticastSources(socketId, interfaceId, multicastAddr, sourceList);
}

void UdpSocket::leaveMulticastSources(int interfaceId, const L3Address& multicastAddr, const std::vector<L3Address>& sourceList)
{
    udp->leaveMulticastSources(socketId, interfaceId, multicastAddr, sourceList);
}

void UdpSocket::joinMulticastSources(int interfaceId, const L3Address& multicastAddr, const std::vector<L3Address>& sourceList)
{
    udp->joinMulticastSources(socketId, interfaceId, multicastAddr, sourceList);
}

void UdpSocket::setMulticastSourceFilter(int interfaceId, const L3Address& multicastAddr,
        UdpSourceFilterMode filterMode, const std::vector<L3Address>& sourceList)
{
    udp->setMulticastSourceFilter(socketId, interfaceId, multicastAddr, filterMode, sourceList);
}

// Join to "local" multicast group
void UdpSocket::joinLocalMulticastGroups(MulticastGroupList mgl)
{
    if (mgl.size() > 0) {
        std::vector<L3Address> multicastAddresses;
        std::vector<int> interfaceIds;
        for (auto el : mgl) {
            multicastAddresses.push_back(el.multicastAddr);
            interfaceIds.push_back(el.interfaceId);
        }
        udp->joinMulticastGroups(socketId, multicastAddresses, interfaceIds);
    }
}

// Leave from "local" multicast group
void UdpSocket::leaveLocalMulticastGroups(MulticastGroupList mgl)
{
    if (mgl.size() > 0) {
        std::vector<L3Address> multicastAddresses;
        for (auto el : mgl)
            multicastAddresses.push_back(el.multicastAddr);
        udp->leaveMulticastGroups(socketId, multicastAddresses);
    }
}

// ######################
// UDP Socket Options end
// ######################

void UdpSocket::sendToUDP(cMessage *msg)
{
    if (!gateToUdp)
        throw cRuntimeError("UdpSocket: setOutputGate() must be invoked before socket can be used");
    EV_DEBUG << "Sending to UDP protocol" << EV_FIELD(msg) << EV_ENDL;
    auto& tags = check_and_cast<ITaggedObject *>(msg)->getTags();
    tags.addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&Protocol::udp);
    tags.addTagIfAbsent<SocketReq>()->setSocketId(socketId);
    check_and_cast<cSimpleModule *>(gateToUdp->getOwnerModule())->send(msg, gateToUdp);
}

std::string UdpSocket::getReceivedPacketInfo(Packet *pk)
{
    auto l3Addresses = pk->getTag<L3AddressInd>();
    auto ports = pk->getTag<L4PortInd>();
    L3Address srcAddr = l3Addresses->getSrcAddress();
    L3Address destAddr = l3Addresses->getDestAddress();
    int srcPort = ports->getSrcPort();
    int destPort = ports->getDestPort();
    int interfaceID = pk->getTag<InterfaceInd>()->getInterfaceId();
    int ttl = pk->getTag<HopLimitInd>()->getHopLimit();

    std::stringstream os;
    os << pk << " (" << pk->getByteLength() << " bytes) ";
    os << srcAddr << ":" << srcPort << " --> " << destAddr << ":" << destPort;
    os << " TTL=" << ttl;
    if (auto tosTag = pk->findTag<TosInd>())
        os << " TOS=" << tosTag->getTos();
    if (auto dscpTag = pk->findTag<DscpInd>())
        os << " DSCP=" << dscpTag->getDifferentiatedServicesCodePoint();
    os << " on ifID=" << interfaceID;
    return os.str();
}

void UdpSocket::setCallback(ICallback *callback)
{
    cb = callback;
}

void UdpSocket::processMessage(cMessage *msg)
{
    ASSERT(belongsToSocket(msg));

    switch (msg->getKind()) {
        case UDP_I_DATA:
            if (cb) {
                auto packet = check_and_cast<Packet *>(msg);
                EV_INFO << "Received packet on socket" << EV_FIELD(socketId) << EV_FIELD(packet) << EV_ENDL;
                cb->socketDataArrived(this, packet);
            }
            else
                delete msg;
            break;
        case UDP_I_ERROR:
            if (cb)
                cb->socketErrorArrived(this, check_and_cast<Indication *>(msg));
            else
                delete msg;
            break;
        case UDP_I_SOCKET_CLOSED:
            check_and_cast<Indication *>(msg);
            sockState = CLOSED;
            if (cb)
                cb->socketClosed(this);
            delete msg;
            break;
        default:
            throw cRuntimeError("UdpSocket: invalid msg kind %d, one of the UDP_I_xxx constants expected", msg->getKind());
            break;
    }
}

bool UdpSocket::belongsToSocket(cMessage *msg) const
{
    auto& tags = check_and_cast<ITaggedObject *>(msg)->getTags();
    const auto& socketInd = tags.findTag<SocketInd>();
    return socketInd != nullptr && socketInd->getSocketId() == socketId;
}

} // namespace inet


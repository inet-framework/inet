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
#include "inet/transportlayer/contract/udp/UdpControlInfo.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"

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

    UdpBindCommand *ctrl = new UdpBindCommand();
    ctrl->setLocalAddr(localAddr);
    ctrl->setLocalPort(localPort);
    auto request = new Request("bind", UDP_C_BIND);
    request->setControlInfo(ctrl);
    sendToUDP(request);
    sockState = CONNECTED;
}

void UdpSocket::connect(L3Address addr, int port)
{
    if (addr.isUnspecified())
        throw cRuntimeError("UdpSocket::connect(): unspecified remote address");
    if (port <= 0 || port > 65535)
        throw cRuntimeError("UdpSocket::connect(): invalid remote port number %d", port);

    UdpConnectCommand *ctrl = new UdpConnectCommand();
    ctrl->setRemoteAddr(addr);
    ctrl->setRemotePort(port);
    auto request = new Request("connect", UDP_C_CONNECT);
    request->setControlInfo(ctrl);
    sendToUDP(request);
    sockState = CONNECTED;
}

void UdpSocket::sendTo(Packet *pk, L3Address destAddr, int destPort)
{
    pk->setKind(UDP_C_DATA);
    auto addressReq = pk->addTagIfAbsent<L3AddressReq>();
    addressReq->setDestAddress(destAddr);
    if (destPort != -1)
        pk->addTagIfAbsent<L4PortReq>()->setDestPort(destPort);
    sendToUDP(pk);
    sockState = CONNECTED;
}

void UdpSocket::send(Packet *pk)
{
    pk->setKind(UDP_C_DATA);
    sendToUDP(pk);
    sockState = CONNECTED;
}

void UdpSocket::close()
{
    if (sockState == CLOSED)
        return;
    auto request = new Request("close", UDP_C_CLOSE);
    UdpCloseCommand *ctrl = new UdpCloseCommand();
    request->setControlInfo(ctrl);
    sendToUDP(request);
    sockState = CLOSED;
}

void UdpSocket::destroy()
{
    if (this->gateToUdp == nullptr)
        return;
    auto request = new Request("destroy", UDP_C_DESTROY);
    auto ctrl = new UdpDestroyCommand();
    request->setControlInfo(ctrl);
    sendToUDP(request);
}

// ########################
// UDP Socket Options Start
// ########################

void UdpSocket::setTimeToLive(int ttl)
{
    auto request = new Request("setTTL", UDP_C_SETOPTION);
    UdpSetTimeToLiveCommand *ctrl = new UdpSetTimeToLiveCommand();
    ctrl->setTtl(ttl);
    request->setControlInfo(ctrl);
    sendToUDP(request);
}

void UdpSocket::setDscp(short dscp)
{
    auto request = new Request("setDscp", UDP_C_SETOPTION);
    auto *ctrl = new UdpSetDscpCommand();
    ctrl->setDscp(dscp);
    request->setControlInfo(ctrl);
    sendToUDP(request);
}

void UdpSocket::setTos(short tos)
{
    auto request = new Request("setTos", UDP_C_SETOPTION);
    auto *ctrl = new UdpSetTosCommand();
    ctrl->setTos(tos);
    request->setControlInfo(ctrl);
    sendToUDP(request);
}

void UdpSocket::setBroadcast(bool broadcast)
{
    auto request = new Request("setBroadcast", UDP_C_SETOPTION);
    UdpSetBroadcastCommand *ctrl = new UdpSetBroadcastCommand();
    ctrl->setBroadcast(broadcast);
    request->setControlInfo(ctrl);
    sendToUDP(request);
}

void UdpSocket::setMulticastOutputInterface(int interfaceId)
{
    auto request = new Request("setMulticastOutputIf", UDP_C_SETOPTION);
    UdpSetMulticastInterfaceCommand *ctrl = new UdpSetMulticastInterfaceCommand();
    ctrl->setInterfaceId(interfaceId);
    request->setControlInfo(ctrl);
    sendToUDP(request);
}

void UdpSocket::setMulticastLoop(bool value)
{
    auto request = new Request("setMulticastLoop", UDP_C_SETOPTION);
    UdpSetMulticastLoopCommand *ctrl = new UdpSetMulticastLoopCommand();
    ctrl->setLoop(value);
    request->setControlInfo(ctrl);
    sendToUDP(request);
}

void UdpSocket::setReuseAddress(bool value)
{
    auto request = new Request("setReuseAddress", UDP_C_SETOPTION);
    UdpSetReuseAddressCommand *ctrl = new UdpSetReuseAddressCommand();
    ctrl->setReuseAddress(value);
    request->setControlInfo(ctrl);
    sendToUDP(request);
}

void UdpSocket::joinMulticastGroup(const L3Address& multicastAddr, int interfaceId)
{
    auto request = new Request("joinMulticastGroups", UDP_C_SETOPTION);
    UdpJoinMulticastGroupsCommand *ctrl = new UdpJoinMulticastGroupsCommand();
    ctrl->setMulticastAddrArraySize(1);
    ctrl->setMulticastAddr(0, multicastAddr);
    ctrl->setInterfaceIdArraySize(1);
    ctrl->setInterfaceId(0, interfaceId);
    request->setControlInfo(ctrl);
    sendToUDP(request);
}

void UdpSocket::leaveMulticastGroup(const L3Address& multicastAddr)
{
    auto request = new Request("leaveMulticastGroups", UDP_C_SETOPTION);
    UdpLeaveMulticastGroupsCommand *ctrl = new UdpLeaveMulticastGroupsCommand();
    ctrl->setMulticastAddrArraySize(1);
    ctrl->setMulticastAddr(0, multicastAddr);
    request->setControlInfo(ctrl);
    sendToUDP(request);
}

void UdpSocket::blockMulticastSources(int interfaceId, const L3Address& multicastAddr, const std::vector<L3Address>& sourceList)
{
    auto request = new Request("blockMulticastSources", UDP_C_SETOPTION);
    UdpBlockMulticastSourcesCommand *ctrl = new UdpBlockMulticastSourcesCommand();
    ctrl->setInterfaceId(interfaceId);
    ctrl->setMulticastAddr(multicastAddr);
    ctrl->setSourceListArraySize(sourceList.size());
    for (size_t i = 0; i < sourceList.size(); ++i)
        ctrl->setSourceList(i, sourceList[i]);
    request->setControlInfo(ctrl);
    sendToUDP(request);
}

void UdpSocket::unblockMulticastSources(int interfaceId, const L3Address& multicastAddr, const std::vector<L3Address>& sourceList)
{
    auto request = new Request("unblockMulticastSources", UDP_C_SETOPTION);
    UdpUnblockMulticastSourcesCommand *ctrl = new UdpUnblockMulticastSourcesCommand();
    ctrl->setInterfaceId(interfaceId);
    ctrl->setMulticastAddr(multicastAddr);
    ctrl->setSourceListArraySize(sourceList.size());
    for (size_t i = 0; i < sourceList.size(); ++i)
        ctrl->setSourceList(i, sourceList[i]);
    request->setControlInfo(ctrl);
    sendToUDP(request);
}

void UdpSocket::leaveMulticastSources(int interfaceId, const L3Address& multicastAddr, const std::vector<L3Address>& sourceList)
{
    auto request = new Request("leaveMulticastSources", UDP_C_SETOPTION);
    UdpLeaveMulticastSourcesCommand *ctrl = new UdpLeaveMulticastSourcesCommand();
    ctrl->setInterfaceId(interfaceId);
    ctrl->setMulticastAddr(multicastAddr);
    ctrl->setSourceListArraySize(sourceList.size());
    for (size_t i = 0; i < sourceList.size(); ++i)
        ctrl->setSourceList(i, sourceList[i]);
    request->setControlInfo(ctrl);
    sendToUDP(request);
}

void UdpSocket::joinMulticastSources(int interfaceId, const L3Address& multicastAddr, const std::vector<L3Address>& sourceList)
{
    auto request = new Request("joinMulticastSources", UDP_C_SETOPTION);
    UdpJoinMulticastSourcesCommand *ctrl = new UdpJoinMulticastSourcesCommand();
    ctrl->setInterfaceId(interfaceId);
    ctrl->setMulticastAddr(multicastAddr);
    ctrl->setSourceListArraySize(sourceList.size());
    for (size_t i = 0; i < sourceList.size(); ++i)
        ctrl->setSourceList(i, sourceList[i]);
    request->setControlInfo(ctrl);
    sendToUDP(request);
}

void UdpSocket::setMulticastSourceFilter(int interfaceId, const L3Address& multicastAddr,
        UdpSourceFilterMode filterMode, const std::vector<L3Address>& sourceList)
{
    auto request = new Request("setMulticastSourceFilter", UDP_C_SETOPTION);
    UdpSetMulticastSourceFilterCommand *ctrl = new UdpSetMulticastSourceFilterCommand();
    ctrl->setInterfaceId(interfaceId);
    ctrl->setMulticastAddr(multicastAddr);
    ctrl->setFilterMode(filterMode);
    ctrl->setSourceListArraySize(sourceList.size());
    for (size_t i = 0; i < sourceList.size(); ++i)
        ctrl->setSourceList(i, sourceList[i]);
    request->setControlInfo(ctrl);
    sendToUDP(request);
}

// Join to "local" multicast group
void UdpSocket::joinLocalMulticastGroups(MulticastGroupList mgl)
{
    if (mgl.size() > 0) {
        UdpJoinMulticastGroupsCommand *ctrl = new UdpJoinMulticastGroupsCommand();
        ctrl->setMulticastAddrArraySize(mgl.size());
        ctrl->setInterfaceIdArraySize(mgl.size());

        for (unsigned int j = 0; j < mgl.size(); ++j) {
            ctrl->setMulticastAddr(j, mgl[j].multicastAddr);
            ctrl->setInterfaceId(j, mgl[j].interfaceId);
        }

        auto request = new Request("joinMulticastGroups", UDP_C_SETOPTION);
        request->setControlInfo(ctrl);
        sendToUDP(request);
    }
}

// Leave from "local" multicast group
void UdpSocket::leaveLocalMulticastGroups(MulticastGroupList mgl)
{
    if (mgl.size() > 0) {
        UdpLeaveMulticastGroupsCommand *ctrl = new UdpLeaveMulticastGroupsCommand();
        ctrl->setMulticastAddrArraySize(mgl.size());

        for (unsigned int j = 0; j < mgl.size(); ++j) {
            ctrl->setMulticastAddr(j, mgl[j].multicastAddr);
        }

        auto request = new Request("leaveMulticastGroups", UDP_C_SETOPTION);
        request->setControlInfo(ctrl);
        sendToUDP(request);
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
            if (cb)
                cb->socketDataArrived(this, check_and_cast<Packet *>(msg));
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


//
// Copyright (C) 2005,2011 Andras Varga
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

#include "inet/applications/common/SocketTag_m.h"
#include "inet/common/packet/Message.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/DscpTag_m.h"
#include "inet/networklayer/common/HopLimitTag_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/transportlayer/common/L4PortTag_m.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"

#ifdef WITH_IPv4
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#endif // ifdef WITH_IPv4

namespace inet {

UdpSocket::UdpSocket()
{
    // don't allow user-specified socketIds because they may conflict with
    // automatically assigned ones.
    socketId = generateSocketId();
    gateToUdp = nullptr;
}

int UdpSocket::generateSocketId()
{
    return getEnvir()->getUniqueNumber();
}

void UdpSocket::sendToUDP(cMessage *msg)
{
    if (!gateToUdp)
        throw cRuntimeError("UdpSocket: setOutputGate() must be invoked before socket can be used");

    cObject *ctrl = msg->getControlInfo();
    EV_TRACE << "UdpSocket: Send (" << msg->getClassName() << ")" << msg->getFullName();
    if (ctrl)
        EV_TRACE << "  control info: (" << ctrl->getClassName() << ")" << ctrl->getFullName();
    EV_TRACE << endl;

    auto& tags = getTags(msg);
    tags.addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&Protocol::udp);
    tags.addTagIfAbsent<SocketReq>()->setSocketId(socketId);
    check_and_cast<cSimpleModule *>(gateToUdp->getOwnerModule())->send(msg, gateToUdp);
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
    auto request = new Request("BIND", UDP_C_BIND);
    request->setControlInfo(ctrl);
    sendToUDP(request);
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
    auto request = new Request("CONNECT", UDP_C_CONNECT);
    request->setControlInfo(ctrl);
    sendToUDP(request);
}

void UdpSocket::sendTo(Packet *pk, L3Address destAddr, int destPort)
{
    pk->setKind(UDP_C_DATA);
    auto addressReq = pk->addTagIfAbsent<L3AddressReq>();
    addressReq->setDestAddress(destAddr);
    if (destPort != -1)
        pk->addTagIfAbsent<L4PortReq>()->setDestPort(destPort);
    sendToUDP(pk);
}

void UdpSocket::send(Packet *pk)
{
    pk->setKind(UDP_C_DATA);
    sendToUDP(pk);
}

void UdpSocket::close()
{
    auto request = new Request("CLOSE", UDP_C_CLOSE);
    UdpCloseCommand *ctrl = new UdpCloseCommand();
    request->setControlInfo(ctrl);
    sendToUDP(request);
}

void UdpSocket::setBroadcast(bool broadcast)
{
    auto request = new Request("SetBroadcast", UDP_C_SETOPTION);
    UdpSetBroadcastCommand *ctrl = new UdpSetBroadcastCommand();
    ctrl->setBroadcast(broadcast);
    request->setControlInfo(ctrl);
    sendToUDP(request);
}

void UdpSocket::setTimeToLive(int ttl)
{
    auto request = new Request("SetTTL", UDP_C_SETOPTION);
    UdpSetTimeToLiveCommand *ctrl = new UdpSetTimeToLiveCommand();
    ctrl->setTtl(ttl);
    request->setControlInfo(ctrl);
    sendToUDP(request);
}

void UdpSocket::setTypeOfService(unsigned char tos)
{
    auto request = new Request("SetTOS", UDP_C_SETOPTION);
    UdpSetTypeOfServiceCommand *ctrl = new UdpSetTypeOfServiceCommand();
    ctrl->setTos(tos);
    request->setControlInfo(ctrl);
    sendToUDP(request);
}

void UdpSocket::setMulticastOutputInterface(int interfaceId)
{
    auto request = new Request("SetMulticastOutputIf", UDP_C_SETOPTION);
    UdpSetMulticastInterfaceCommand *ctrl = new UdpSetMulticastInterfaceCommand();
    ctrl->setInterfaceId(interfaceId);
    request->setControlInfo(ctrl);
    sendToUDP(request);
}

void UdpSocket::setMulticastLoop(bool value)
{
    auto request = new Request("SetMulticastLoop", UDP_C_SETOPTION);
    UdpSetMulticastLoopCommand *ctrl = new UdpSetMulticastLoopCommand();
    ctrl->setLoop(value);
    request->setControlInfo(ctrl);
    sendToUDP(request);
}

void UdpSocket::setReuseAddress(bool value)
{
    auto request = new Request("SetReuseAddress", UDP_C_SETOPTION);
    UdpSetReuseAddressCommand *ctrl = new UdpSetReuseAddressCommand();
    ctrl->setReuseAddress(value);
    request->setControlInfo(ctrl);
    sendToUDP(request);
}

void UdpSocket::joinMulticastGroup(const L3Address& multicastAddr, int interfaceId)
{
    auto request = new Request("JoinMulticastGroups", UDP_C_SETOPTION);
    UdpJoinMulticastGroupsCommand *ctrl = new UdpJoinMulticastGroupsCommand();
    ctrl->setMulticastAddrArraySize(1);
    ctrl->setMulticastAddr(0, multicastAddr);
    ctrl->setInterfaceIdArraySize(1);
    ctrl->setInterfaceId(0, interfaceId);
    request->setControlInfo(ctrl);
    sendToUDP(request);
}

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

        auto request = new Request("JoinMulticastGroups", UDP_C_SETOPTION);
        request->setControlInfo(ctrl);
        sendToUDP(request);
    }
}

void UdpSocket::leaveMulticastGroup(const L3Address& multicastAddr)
{
    auto request = new Request("LeaveMulticastGroups", UDP_C_SETOPTION);
    UdpLeaveMulticastGroupsCommand *ctrl = new UdpLeaveMulticastGroupsCommand();
    ctrl->setMulticastAddrArraySize(1);
    ctrl->setMulticastAddr(0, multicastAddr);
    request->setControlInfo(ctrl);
    sendToUDP(request);
}

void UdpSocket::leaveLocalMulticastGroups(MulticastGroupList mgl)
{
    if (mgl.size() > 0) {
        UdpLeaveMulticastGroupsCommand *ctrl = new UdpLeaveMulticastGroupsCommand();
        ctrl->setMulticastAddrArraySize(mgl.size());

        for (unsigned int j = 0; j < mgl.size(); ++j) {
            ctrl->setMulticastAddr(j, mgl[j].multicastAddr);
        }

        auto request = new Request("LeaveMulticastGroups", UDP_C_SETOPTION);
        request->setControlInfo(ctrl);
        sendToUDP(request);
    }
}

void UdpSocket::blockMulticastSources(int interfaceId, const L3Address& multicastAddr, const std::vector<L3Address>& sourceList)
{
    auto request = new Request("BlockMulticastSources", UDP_C_SETOPTION);
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
    auto request = new Request("UnblockMulticastSources", UDP_C_SETOPTION);
    UdpUnblockMulticastSourcesCommand *ctrl = new UdpUnblockMulticastSourcesCommand();
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
    auto request = new Request("JoinMulticastSources", UDP_C_SETOPTION);
    UdpJoinMulticastSourcesCommand *ctrl = new UdpJoinMulticastSourcesCommand();
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
    auto request = new Request("LeaveMulticastSources", UDP_C_SETOPTION);
    UdpLeaveMulticastSourcesCommand *ctrl = new UdpLeaveMulticastSourcesCommand();
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
    auto request = new Request("SetMulticastSourceFilter", UDP_C_SETOPTION);
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

bool UdpSocket::belongsToSocket(cMessage *msg) const
{
    auto& tags = getTags(msg);
    int socketId = tags.getTag<SocketInd>()->getSocketId();
    return socketId == this->socketId;
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
        default:
            throw cRuntimeError("UdpSocket: invalid msg kind %d, one of the UDP_I_xxx constants expected", msg->getKind());
            break;
    }
}

} // namespace inet


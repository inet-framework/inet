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
    // don't allow user-specified sockIds because they may conflict with
    // automatically assigned ones.
    sockId = generateSocketId();
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

    msg->_addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&Protocol::udp);
    msg->_addTagIfAbsent<SocketReq>()->setSocketId(sockId);
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
    cMessage *msg = new cMessage("BIND", UDP_C_BIND);
    msg->setControlInfo(ctrl);
    sendToUDP(msg);
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
    cMessage *msg = new cMessage("CONNECT", UDP_C_CONNECT);
    msg->setControlInfo(ctrl);
    sendToUDP(msg);
}

void UdpSocket::sendTo(Packet *pk, L3Address destAddr, int destPort)
{
    pk->setKind(UDP_C_DATA);
    auto addressReq = pk->_addTagIfAbsent<L3AddressReq>();
    addressReq->setDestAddress(destAddr);
    if (destPort != -1)
        pk->_addTagIfAbsent<L4PortReq>()->setDestPort(destPort);
    sendToUDP(pk);
}

void UdpSocket::send(Packet *pk)
{
    pk->setKind(UDP_C_DATA);
    sendToUDP(pk);
}

void UdpSocket::close()
{
    cMessage *msg = new cMessage("CLOSE", UDP_C_CLOSE);
    UdpCloseCommand *ctrl = new UdpCloseCommand();
    msg->setControlInfo(ctrl);
    sendToUDP(msg);
}

void UdpSocket::setBroadcast(bool broadcast)
{
    cMessage *msg = new cMessage("SetBroadcast", UDP_C_SETOPTION);
    UdpSetBroadcastCommand *ctrl = new UdpSetBroadcastCommand();
    ctrl->setBroadcast(broadcast);
    msg->setControlInfo(ctrl);
    sendToUDP(msg);
}

void UdpSocket::setTimeToLive(int ttl)
{
    cMessage *msg = new cMessage("SetTTL", UDP_C_SETOPTION);
    UdpSetTimeToLiveCommand *ctrl = new UdpSetTimeToLiveCommand();
    ctrl->setTtl(ttl);
    msg->setControlInfo(ctrl);
    sendToUDP(msg);
}

void UdpSocket::setTypeOfService(unsigned char tos)
{
    cMessage *msg = new cMessage("SetTOS", UDP_C_SETOPTION);
    UdpSetTypeOfServiceCommand *ctrl = new UdpSetTypeOfServiceCommand();
    ctrl->setTos(tos);
    msg->setControlInfo(ctrl);
    sendToUDP(msg);
}

void UdpSocket::setMulticastOutputInterface(int interfaceId)
{
    cMessage *msg = new cMessage("SetMulticastOutputIf", UDP_C_SETOPTION);
    UdpSetMulticastInterfaceCommand *ctrl = new UdpSetMulticastInterfaceCommand();
    ctrl->setInterfaceId(interfaceId);
    msg->setControlInfo(ctrl);
    sendToUDP(msg);
}

void UdpSocket::setMulticastLoop(bool value)
{
    cMessage *msg = new cMessage("SetMulticastLoop", UDP_C_SETOPTION);
    UdpSetMulticastLoopCommand *ctrl = new UdpSetMulticastLoopCommand();
    ctrl->setLoop(value);
    msg->setControlInfo(ctrl);
    sendToUDP(msg);
}

void UdpSocket::setReuseAddress(bool value)
{
    cMessage *msg = new cMessage("SetReuseAddress", UDP_C_SETOPTION);
    UdpSetReuseAddressCommand *ctrl = new UdpSetReuseAddressCommand();
    ctrl->setReuseAddress(value);
    msg->setControlInfo(ctrl);
    sendToUDP(msg);
}

void UdpSocket::joinMulticastGroup(const L3Address& multicastAddr, int interfaceId)
{
    cMessage *msg = new cMessage("JoinMulticastGroups", UDP_C_SETOPTION);
    UdpJoinMulticastGroupsCommand *ctrl = new UdpJoinMulticastGroupsCommand();
    ctrl->setMulticastAddrArraySize(1);
    ctrl->setMulticastAddr(0, multicastAddr);
    ctrl->setInterfaceIdArraySize(1);
    ctrl->setInterfaceId(0, interfaceId);
    msg->setControlInfo(ctrl);
    sendToUDP(msg);
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

        cMessage *msg = new cMessage("JoinMulticastGroups", UDP_C_SETOPTION);
        msg->setControlInfo(ctrl);
        sendToUDP(msg);
    }
}

void UdpSocket::leaveMulticastGroup(const L3Address& multicastAddr)
{
    cMessage *msg = new cMessage("LeaveMulticastGroups", UDP_C_SETOPTION);
    UdpLeaveMulticastGroupsCommand *ctrl = new UdpLeaveMulticastGroupsCommand();
    ctrl->setMulticastAddrArraySize(1);
    ctrl->setMulticastAddr(0, multicastAddr);
    msg->setControlInfo(ctrl);
    sendToUDP(msg);
}

void UdpSocket::leaveLocalMulticastGroups(MulticastGroupList mgl)
{
    if (mgl.size() > 0) {
        UdpLeaveMulticastGroupsCommand *ctrl = new UdpLeaveMulticastGroupsCommand();
        ctrl->setMulticastAddrArraySize(mgl.size());

        for (unsigned int j = 0; j < mgl.size(); ++j) {
            ctrl->setMulticastAddr(j, mgl[j].multicastAddr);
        }

        cMessage *msg = new cMessage("LeaveMulticastGroups", UDP_C_SETOPTION);
        msg->setControlInfo(ctrl);
        sendToUDP(msg);
    }
}

void UdpSocket::blockMulticastSources(int interfaceId, const L3Address& multicastAddr, const std::vector<L3Address>& sourceList)
{
    cMessage *msg = new cMessage("BlockMulticastSources", UDP_C_SETOPTION);
    UdpBlockMulticastSourcesCommand *ctrl = new UdpBlockMulticastSourcesCommand();
    ctrl->setInterfaceId(interfaceId);
    ctrl->setMulticastAddr(multicastAddr);
    ctrl->setSourceListArraySize(sourceList.size());
    for (int i = 0; i < (int)sourceList.size(); ++i)
        ctrl->setSourceList(i, sourceList[i]);
    msg->setControlInfo(ctrl);
    sendToUDP(msg);
}

void UdpSocket::unblockMulticastSources(int interfaceId, const L3Address& multicastAddr, const std::vector<L3Address>& sourceList)
{
    cMessage *msg = new cMessage("UnblockMulticastSources", UDP_C_SETOPTION);
    UdpUnblockMulticastSourcesCommand *ctrl = new UdpUnblockMulticastSourcesCommand();
    ctrl->setInterfaceId(interfaceId);
    ctrl->setMulticastAddr(multicastAddr);
    ctrl->setSourceListArraySize(sourceList.size());
    for (int i = 0; i < (int)sourceList.size(); ++i)
        ctrl->setSourceList(i, sourceList[i]);
    msg->setControlInfo(ctrl);
    sendToUDP(msg);
}

void UdpSocket::joinMulticastSources(int interfaceId, const L3Address& multicastAddr, const std::vector<L3Address>& sourceList)
{
    cMessage *msg = new cMessage("JoinMulticastSources", UDP_C_SETOPTION);
    UdpJoinMulticastSourcesCommand *ctrl = new UdpJoinMulticastSourcesCommand();
    ctrl->setInterfaceId(interfaceId);
    ctrl->setMulticastAddr(multicastAddr);
    ctrl->setSourceListArraySize(sourceList.size());
    for (int i = 0; i < (int)sourceList.size(); ++i)
        ctrl->setSourceList(i, sourceList[i]);
    msg->setControlInfo(ctrl);
    sendToUDP(msg);
}

void UdpSocket::leaveMulticastSources(int interfaceId, const L3Address& multicastAddr, const std::vector<L3Address>& sourceList)
{
    cMessage *msg = new cMessage("LeaveMulticastSources", UDP_C_SETOPTION);
    UdpLeaveMulticastSourcesCommand *ctrl = new UdpLeaveMulticastSourcesCommand();
    ctrl->setInterfaceId(interfaceId);
    ctrl->setMulticastAddr(multicastAddr);
    ctrl->setSourceListArraySize(sourceList.size());
    for (int i = 0; i < (int)sourceList.size(); ++i)
        ctrl->setSourceList(i, sourceList[i]);
    msg->setControlInfo(ctrl);
    sendToUDP(msg);
}

void UdpSocket::setMulticastSourceFilter(int interfaceId, const L3Address& multicastAddr,
        UdpSourceFilterMode filterMode, const std::vector<L3Address>& sourceList)
{
    cMessage *msg = new cMessage("SetMulticastSourceFilter", UDP_C_SETOPTION);
    UdpSetMulticastSourceFilterCommand *ctrl = new UdpSetMulticastSourceFilterCommand();
    ctrl->setInterfaceId(interfaceId);
    ctrl->setMulticastAddr(multicastAddr);
    ctrl->setFilterMode(filterMode);
    ctrl->setSourceListArraySize(sourceList.size());
    for (int i = 0; i < (int)sourceList.size(); ++i)
        ctrl->setSourceList(i, sourceList[i]);
    msg->setControlInfo(ctrl);
    sendToUDP(msg);
}

bool UdpSocket::belongsToSocket(cMessage *msg)
{
    int socketId = msg->_getTag<SocketReq>()->getSocketId();
    return dynamic_cast<UdpControlInfo *>(msg->getControlInfo()) && socketId == sockId;
}

bool UdpSocket::belongsToAnyUDPSocket(cMessage *msg)
{
    return dynamic_cast<UdpControlInfo *>(msg->getControlInfo());
}

std::string UdpSocket::getReceivedPacketInfo(Packet *pk)
{
    auto l3Addresses = pk->_getTag<L3AddressInd>();
    auto ports = pk->_getTag<L4PortInd>();
    L3Address srcAddr = l3Addresses->getSrcAddress();
    L3Address destAddr = l3Addresses->getDestAddress();
    int srcPort = ports->getSrcPort();
    int destPort = ports->getDestPort();
    int interfaceID = pk->_getTag<InterfaceInd>()->getInterfaceId();
    int ttl = pk->_getTag<HopLimitInd>()->getHopLimit();

    std::stringstream os;
    os << pk << " (" << pk->getByteLength() << " bytes) ";
    os << srcAddr << ":" << srcPort << " --> " << destAddr << ":" << destPort;
    os << " TTL=" << ttl;
    if (auto dscpTag = pk->_findTag<DscpInd>())
        os << " DSCP=" << dscpTag->getDifferentiatedServicesCodePoint();
    os << " on ifID=" << interfaceID;
    return os.str();
}

} // namespace inet


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

#include "inet/transportlayer/contract/udp/UDPSocket.h"
#include "inet/transportlayer/contract/udp/UDPControlInfo.h"
#ifdef WITH_IPv4
#include "inet/networklayer/ipv4/IPv4InterfaceData.h"
#endif // ifdef WITH_IPv4

namespace inet {

UDPSocket::UDPSocket()
{
    // don't allow user-specified sockIds because they may conflict with
    // automatically assigned ones.
    sockId = generateSocketId();
    gateToUdp = NULL;
}

int UDPSocket::generateSocketId()
{
    return ev.getUniqueNumber();
}

void UDPSocket::sendToUDP(cMessage *msg)
{
    if (!gateToUdp)
        throw cRuntimeError("UDPSocket: setOutputGate() must be invoked before socket can be used");

    check_and_cast<cSimpleModule *>(gateToUdp->getOwnerModule())->send(msg, gateToUdp);
}

void UDPSocket::bind(int localPort)
{
    bind(L3Address(), localPort);
}

void UDPSocket::bind(L3Address localAddr, int localPort)
{
    if (localPort < -1 || localPort > 65535) // -1: ephemeral port
        throw cRuntimeError("UDPSocket::bind(): invalid port number %d", localPort);

    UDPBindCommand *ctrl = new UDPBindCommand();
    ctrl->setSockId(sockId);
    ctrl->setLocalAddr(localAddr);
    ctrl->setLocalPort(localPort);
    cMessage *msg = new cMessage("BIND", UDP_C_BIND);
    msg->setControlInfo(ctrl);
    sendToUDP(msg);
}

void UDPSocket::connect(L3Address addr, int port)
{
    if (addr.isUnspecified())
        throw cRuntimeError("UDPSocket::connect(): unspecified remote address");
    if (port <= 0 || port > 65535)
        throw cRuntimeError("UDPSocket::connect(): invalid remote port number %d", port);

    UDPConnectCommand *ctrl = new UDPConnectCommand();
    ctrl->setSockId(sockId);
    ctrl->setRemoteAddr(addr);
    ctrl->setRemotePort(port);
    cMessage *msg = new cMessage("CONNECT", UDP_C_CONNECT);
    msg->setControlInfo(ctrl);
    sendToUDP(msg);
}

void UDPSocket::sendTo(cPacket *pk, L3Address destAddr, int destPort, const SendOptions *options)
{
    pk->setKind(UDP_C_DATA);
    UDPSendCommand *ctrl = new UDPSendCommand();
    ctrl->setSockId(sockId);
    ctrl->setDestAddr(destAddr);
    ctrl->setDestPort(destPort);
    if (options) {
        ctrl->setSrcAddr(options->srcAddr);
        ctrl->setInterfaceId(options->outInterfaceId);
    }
    pk->setControlInfo(ctrl);
    sendToUDP(pk);
}

void UDPSocket::send(cPacket *pk)
{
    pk->setKind(UDP_C_DATA);
    UDPSendCommand *ctrl = new UDPSendCommand();
    ctrl->setSockId(sockId);
    pk->setControlInfo(ctrl);
    sendToUDP(pk);
}

void UDPSocket::close()
{
    cMessage *msg = new cMessage("CLOSE", UDP_C_CLOSE);
    UDPCloseCommand *ctrl = new UDPCloseCommand();
    ctrl->setSockId(sockId);
    msg->setControlInfo(ctrl);
    sendToUDP(msg);
}

void UDPSocket::setBroadcast(bool broadcast)
{
    cMessage *msg = new cMessage("SetBroadcast", UDP_C_SETOPTION);
    UDPSetBroadcastCommand *ctrl = new UDPSetBroadcastCommand();
    ctrl->setSockId(sockId);
    ctrl->setBroadcast(broadcast);
    msg->setControlInfo(ctrl);
    sendToUDP(msg);
}

void UDPSocket::setTimeToLive(int ttl)
{
    cMessage *msg = new cMessage("SetTTL", UDP_C_SETOPTION);
    UDPSetTimeToLiveCommand *ctrl = new UDPSetTimeToLiveCommand();
    ctrl->setSockId(sockId);
    ctrl->setTtl(ttl);
    msg->setControlInfo(ctrl);
    sendToUDP(msg);
}

void UDPSocket::setTypeOfService(unsigned char tos)
{
    cMessage *msg = new cMessage("SetTOS", UDP_C_SETOPTION);
    UDPSetTypeOfServiceCommand *ctrl = new UDPSetTypeOfServiceCommand();
    ctrl->setSockId(sockId);
    ctrl->setTos(tos);
    msg->setControlInfo(ctrl);
    sendToUDP(msg);
}

void UDPSocket::setMulticastOutputInterface(int interfaceId)
{
    cMessage *msg = new cMessage("SetMulticastOutputIf", UDP_C_SETOPTION);
    UDPSetMulticastInterfaceCommand *ctrl = new UDPSetMulticastInterfaceCommand();
    ctrl->setSockId(sockId);
    ctrl->setInterfaceId(interfaceId);
    msg->setControlInfo(ctrl);
    sendToUDP(msg);
}

void UDPSocket::setMulticastLoop(bool value)
{
    cMessage *msg = new cMessage("SetMulticastLoop", UDP_C_SETOPTION);
    UDPSetMulticastLoopCommand *ctrl = new UDPSetMulticastLoopCommand();
    ctrl->setSockId(sockId);
    ctrl->setLoop(value);
    msg->setControlInfo(ctrl);
    sendToUDP(msg);
}

void UDPSocket::setReuseAddress(bool value)
{
    cMessage *msg = new cMessage("SetReuseAddress", UDP_C_SETOPTION);
    UDPSetReuseAddressCommand *ctrl = new UDPSetReuseAddressCommand();
    ctrl->setSockId(sockId);
    ctrl->setReuseAddress(value);
    msg->setControlInfo(ctrl);
    sendToUDP(msg);
}

void UDPSocket::joinMulticastGroup(const L3Address& multicastAddr, int interfaceId)
{
    cMessage *msg = new cMessage("JoinMulticastGroups", UDP_C_SETOPTION);
    UDPJoinMulticastGroupsCommand *ctrl = new UDPJoinMulticastGroupsCommand();
    ctrl->setSockId(sockId);
    ctrl->setMulticastAddrArraySize(1);
    ctrl->setMulticastAddr(0, multicastAddr);
    ctrl->setInterfaceIdArraySize(1);
    ctrl->setInterfaceId(0, interfaceId);
    msg->setControlInfo(ctrl);
    sendToUDP(msg);
}

void UDPSocket::joinLocalMulticastGroups(MulticastGroupList mgl)
{
    if (mgl.size() > 0) {
        UDPJoinMulticastGroupsCommand *ctrl = new UDPJoinMulticastGroupsCommand();
        ctrl->setSockId(sockId);
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

void UDPSocket::leaveMulticastGroup(const L3Address& multicastAddr)
{
    cMessage *msg = new cMessage("LeaveMulticastGroups", UDP_C_SETOPTION);
    UDPLeaveMulticastGroupsCommand *ctrl = new UDPLeaveMulticastGroupsCommand();
    ctrl->setSockId(sockId);
    ctrl->setMulticastAddrArraySize(1);
    ctrl->setMulticastAddr(0, multicastAddr);
    msg->setControlInfo(ctrl);
    sendToUDP(msg);
}

void UDPSocket::leaveLocalMulticastGroups(MulticastGroupList mgl)
{
    if (mgl.size() > 0) {
        UDPLeaveMulticastGroupsCommand *ctrl = new UDPLeaveMulticastGroupsCommand();
        ctrl->setSockId(sockId);
        ctrl->setMulticastAddrArraySize(mgl.size());

        for (unsigned int j = 0; j < mgl.size(); ++j) {
            ctrl->setMulticastAddr(j, mgl[j].multicastAddr);
        }

        cMessage *msg = new cMessage("LeaveMulticastGroups", UDP_C_SETOPTION);
        msg->setControlInfo(ctrl);
        sendToUDP(msg);
    }
}

void UDPSocket::blockMulticastSources(int interfaceId, const L3Address& multicastAddr, const std::vector<L3Address>& sourceList)
{
    cMessage *msg = new cMessage("BlockMulticastSources", UDP_C_SETOPTION);
    UDPBlockMulticastSourcesCommand *ctrl = new UDPBlockMulticastSourcesCommand();
    ctrl->setSockId(sockId);
    ctrl->setInterfaceId(interfaceId);
    ctrl->setMulticastAddr(multicastAddr);
    ctrl->setSourceListArraySize(sourceList.size());
    for (int i = 0; i < (int)sourceList.size(); ++i)
        ctrl->setSourceList(i, sourceList[i]);
    msg->setControlInfo(ctrl);
    sendToUDP(msg);
}

void UDPSocket::unblockMulticastSources(int interfaceId, const L3Address& multicastAddr, const std::vector<L3Address>& sourceList)
{
    cMessage *msg = new cMessage("UnblockMulticastSources", UDP_C_SETOPTION);
    UDPUnblockMulticastSourcesCommand *ctrl = new UDPUnblockMulticastSourcesCommand();
    ctrl->setSockId(sockId);
    ctrl->setInterfaceId(interfaceId);
    ctrl->setMulticastAddr(multicastAddr);
    ctrl->setSourceListArraySize(sourceList.size());
    for (int i = 0; i < (int)sourceList.size(); ++i)
        ctrl->setSourceList(i, sourceList[i]);
    msg->setControlInfo(ctrl);
    sendToUDP(msg);
}

void UDPSocket::joinMulticastSources(int interfaceId, const L3Address& multicastAddr, const std::vector<L3Address>& sourceList)
{
    cMessage *msg = new cMessage("JoinMulticastSources", UDP_C_SETOPTION);
    UDPJoinMulticastSourcesCommand *ctrl = new UDPJoinMulticastSourcesCommand();
    ctrl->setSockId(sockId);
    ctrl->setInterfaceId(interfaceId);
    ctrl->setMulticastAddr(multicastAddr);
    ctrl->setSourceListArraySize(sourceList.size());
    for (int i = 0; i < (int)sourceList.size(); ++i)
        ctrl->setSourceList(i, sourceList[i]);
    msg->setControlInfo(ctrl);
    sendToUDP(msg);
}

void UDPSocket::leaveMulticastSources(int interfaceId, const L3Address& multicastAddr, const std::vector<L3Address>& sourceList)
{
    cMessage *msg = new cMessage("LeaveMulticastSources", UDP_C_SETOPTION);
    UDPLeaveMulticastSourcesCommand *ctrl = new UDPLeaveMulticastSourcesCommand();
    ctrl->setSockId(sockId);
    ctrl->setInterfaceId(interfaceId);
    ctrl->setMulticastAddr(multicastAddr);
    ctrl->setSourceListArraySize(sourceList.size());
    for (int i = 0; i < (int)sourceList.size(); ++i)
        ctrl->setSourceList(i, sourceList[i]);
    msg->setControlInfo(ctrl);
    sendToUDP(msg);
}

void UDPSocket::setMulticastSourceFilter(int interfaceId, const L3Address& multicastAddr,
        UDPSourceFilterMode filterMode, const std::vector<L3Address>& sourceList)
{
    cMessage *msg = new cMessage("SetMulticastSourceFilter", UDP_C_SETOPTION);
    UDPSetMulticastSourceFilterCommand *ctrl = new UDPSetMulticastSourceFilterCommand();
    ctrl->setSockId(sockId);
    ctrl->setInterfaceId(interfaceId);
    ctrl->setMulticastAddr(multicastAddr);
    ctrl->setFilterMode(filterMode);
    ctrl->setSourceListArraySize(sourceList.size());
    for (int i = 0; i < (int)sourceList.size(); ++i)
        ctrl->setSourceList(i, sourceList[i]);
    msg->setControlInfo(ctrl);
    sendToUDP(msg);
}

bool UDPSocket::belongsToSocket(cMessage *msg)
{
    return dynamic_cast<UDPControlInfo *>(msg->getControlInfo()) &&
           ((UDPControlInfo *)(msg->getControlInfo()))->getSockId() == sockId;
}

bool UDPSocket::belongsToAnyUDPSocket(cMessage *msg)
{
    return dynamic_cast<UDPControlInfo *>(msg->getControlInfo());
}

std::string UDPSocket::getReceivedPacketInfo(cPacket *pk)
{
    UDPDataIndication *ctrl = check_and_cast<UDPDataIndication *>(pk->getControlInfo());

    L3Address srcAddr = ctrl->getSrcAddr();
    L3Address destAddr = ctrl->getDestAddr();
    int srcPort = ctrl->getSrcPort();
    int destPort = ctrl->getDestPort();
    int interfaceID = ctrl->getInterfaceId();
    int ttl = ctrl->getTtl();
    int tos = ctrl->getTypeOfService();

    std::stringstream os;
    os << pk << " (" << pk->getByteLength() << " bytes) ";
    os << srcAddr << ":" << srcPort << " --> " << destAddr << ":" << destPort;
    os << " TTL=" << ttl << " ToS=" << tos << " on ifID=" << interfaceID;
    return os.str();
}

} // namespace inet


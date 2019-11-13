//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include <omnetpp/platdep/sockets.h>

#include "inet/applications/common/SocketTag_m.h"
#include "inet/common/NetworkNamespaceContext.h"
#include "inet/common/Simsignals.h"
#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/common/packet/Message.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/emulation/transportlayer/udp/ExtLowerUdp.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/transportlayer/common/L4PortTag_m.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo.h"

namespace inet {

Define_Module(ExtLowerUdp);

ExtLowerUdp::~ExtLowerUdp()
{
    for (auto& it : socketIdToSocketMap) {
        close(it.second->socketId);
    }
}

void ExtLowerUdp::initialize(int stage)
{
    cSimpleModule::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        packetNameFormat = par("packetNameFormat");
        if (auto scheduler = dynamic_cast<RealTimeScheduler *>(getSimulation()->getScheduler())) {
            rtScheduler = scheduler;
        }
    }
    else if (stage == INITSTAGE_TRANSPORT_LAYER) {
        registerService(Protocol::udp, gate("appIn"), nullptr);
        registerProtocol(Protocol::udp, nullptr, gate("appOut"));
    }
}

void ExtLowerUdp::handleMessage(cMessage *message)
{
    switch (message->getKind()) {
        case UDP_C_BIND: {
            int socketId = check_and_cast<Request *>(message)->getTag<SocketReq>()->getSocketId();
            UdpBindCommand *ctrl = check_and_cast<UdpBindCommand *>(message->getControlInfo());
            bind(socketId, ctrl->getLocalAddr(), ctrl->getLocalPort());
            break;
        }

        case UDP_C_CONNECT: {
            int socketId = check_and_cast<Request *>(message)->getTag<SocketReq>()->getSocketId();
            UdpConnectCommand *ctrl = check_and_cast<UdpConnectCommand *>(message->getControlInfo());
            connect(socketId, ctrl->getRemoteAddr(), ctrl->getRemotePort());
            break;
        }

        case UDP_C_CLOSE: {
            int socketId = check_and_cast<Request *>(message)->getTag<SocketReq>()->getSocketId();
            close(socketId);
            break;
        }

        case UDP_C_DATA: {
            processPacketFromUpper(check_and_cast<Packet *>(message));
            break;
        }

        case UDP_C_SETOPTION: {
            throw cRuntimeError("Not implemented");
//            int socketId = check_and_cast<Request *>(message)->getTag<SocketReq>()->getSocketId();
//            UdpSetOptionCommand *ctrl = check_and_cast<UdpSetOptionCommand *>(message->getControlInfo());
//            SockDesc *sd = getOrCreateSocket(socketId);
//
//            if (auto cmd = dynamic_cast<UdpSetTimeToLiveCommand *>(ctrl))
//                setTimeToLive(sd, cmd->getTtl());
//            else if (auto cmd = dynamic_cast<UdpSetTypeOfServiceCommand *>(ctrl))
//                setTypeOfService(sd, cmd->getTos());
//            else if (auto cmd = dynamic_cast<UdpSetBroadcastCommand *>(ctrl))
//                setBroadcast(sd, cmd->getBroadcast());
//            else if (auto cmd = dynamic_cast<UdpSetMulticastInterfaceCommand *>(ctrl))
//                setMulticastOutputInterface(sd, cmd->getInterfaceId());
//            else if (auto cmd = dynamic_cast<UdpSetMulticastLoopCommand *>(ctrl))
//                setMulticastLoop(sd, cmd->getLoop());
//            else if (auto cmd = dynamic_cast<UdpSetReuseAddressCommand *>(ctrl))
//                setReuseAddress(sd, cmd->getReuseAddress());
//            else if (auto cmd = dynamic_cast<UdpJoinMulticastGroupsCommand *>(ctrl)) {
//                std::vector<L3Address> addresses;
//                std::vector<int> interfaceIds;
//                for (size_t i = 0; i < cmd->getMulticastAddrArraySize(); i++)
//                    addresses.push_back(cmd->getMulticastAddr(i));
//                for (size_t i = 0; i < cmd->getInterfaceIdArraySize(); i++)
//                    interfaceIds.push_back(cmd->getInterfaceId(i));
//                joinMulticastGroups(sd, addresses, interfaceIds);
//            }
//            else if (auto cmd = dynamic_cast<UdpLeaveMulticastGroupsCommand *>(ctrl)) {
//                std::vector<L3Address> addresses;
//                for (size_t i = 0; i < cmd->getMulticastAddrArraySize(); i++)
//                    addresses.push_back(cmd->getMulticastAddr(i));
//                leaveMulticastGroups(sd, addresses);
//            }
//            else if (auto cmd = dynamic_cast<UdpBlockMulticastSourcesCommand *>(ctrl)) {
//                InterfaceEntry *ie = ift->getInterfaceById(cmd->getInterfaceId());
//                std::vector<L3Address> sourceList;
//                for (size_t i = 0; i < cmd->getSourceListArraySize(); i++)
//                    sourceList.push_back(cmd->getSourceList(i));
//                blockMulticastSources(sd, ie, cmd->getMulticastAddr(), sourceList);
//            }
//            else if (auto cmd = dynamic_cast<UdpUnblockMulticastSourcesCommand *>(ctrl)) {
//                InterfaceEntry *ie = ift->getInterfaceById(cmd->getInterfaceId());
//                std::vector<L3Address> sourceList;
//                for (size_t i = 0; i < cmd->getSourceListArraySize(); i++)
//                    sourceList.push_back(cmd->getSourceList(i));
//                leaveMulticastSources(sd, ie, cmd->getMulticastAddr(), sourceList);
//            }
//            else if (auto cmd = dynamic_cast<UdpJoinMulticastSourcesCommand *>(ctrl)) {
//                InterfaceEntry *ie = ift->getInterfaceById(cmd->getInterfaceId());
//                std::vector<L3Address> sourceList;
//                for (size_t i = 0; i < cmd->getSourceListArraySize(); i++)
//                    sourceList.push_back(cmd->getSourceList(i));
//                joinMulticastSources(sd, ie, cmd->getMulticastAddr(), sourceList);
//            }
//            else if (auto cmd = dynamic_cast<UdpLeaveMulticastSourcesCommand *>(ctrl)) {
//               InterfaceEntry *ie = ift->getInterfaceById(cmd->getInterfaceId());
//                std::vector<L3Address> sourceList;
//                for (size_t i = 0; i < cmd->getSourceListArraySize(); i++)
//                    sourceList.push_back(cmd->getSourceList(i));
//                leaveMulticastSources(sd, ie, cmd->getMulticastAddr(), sourceList);
//            }
//            else if (auto cmd = dynamic_cast<UdpSetMulticastSourceFilterCommand *>(ctrl)) {
//                InterfaceEntry *ie = ift->getInterfaceById(cmd->getInterfaceId());
//                std::vector<L3Address> sourceList;
//                for (unsigned int i = 0; i < cmd->getSourceListArraySize(); i++)
//                    sourceList.push_back(cmd->getSourceList(i));
//                setMulticastSourceFilter(sd, ie, cmd->getMulticastAddr(), cmd->getFilterMode(), sourceList);
//            }
//            else
//                throw cRuntimeError("Unknown subclass of UdpSetOptionCommand received from app: %s", ctrl->getClassName());
//            break;
        }

        default:
            throw cRuntimeError("Unknown command code (message kind) %d received from app", message->getKind());
    }
    delete message;
}

bool ExtLowerUdp::handleOperationStage(LifecycleOperation *operation, IDoneCallback *doneCallback)
{
    // TODO:
    return true;
}

bool ExtLowerUdp::notify(int fd)
{
    auto it = fdToSocketMap.find(fd);
    if (it == fdToSocketMap.end())
        return false;
    else {
        auto socket = it->second;
        processPacketFromLower(socket->fd);
        return true;
    }
}

ExtLowerUdp::Socket *ExtLowerUdp::open(int socketId)
{
    NetworkNamespaceContext context(par("namespace"));
    auto socket = new Socket(socketId);
    int fd = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
        throw cRuntimeError("Cannot create socket: %d", fd);

    // Setting this option makes it possible to kill the simulations
    // and restart them right away using the same port numbers.
    int enable = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&enable, sizeof(int)) < 0)
        throw cRuntimeError("ExtLowerUdp: cannot set socket option");

    socket->fd = fd;
    socketIdToSocketMap[socketId] = socket;
    fdToSocketMap[fd] = socket;
    rtScheduler->addCallback(fd, this);
    return socket;
}

void ExtLowerUdp::bind(int socketId, const L3Address& localAddress, int localPort)
{
    Socket *socket = nullptr;
    auto it = socketIdToSocketMap.find(socketId);
    if (it == socketIdToSocketMap.end())
        socket = open(socketId);
    else
        socket = it->second;
    struct sockaddr_in sockaddr;
    sockaddr.sin_family = PF_INET;
    sockaddr.sin_port = htons(localPort);
    sockaddr.sin_addr.s_addr = htonl(localAddress.toIpv4().getInt());
#if !defined(linux) && !defined(__linux) && !defined(_WIN32)
        sockaddr.sin_len = sizeof(struct sockaddr_in);
#endif
    int n = ::bind(socket->fd, (struct sockaddr *)&sockaddr, sizeof(sockaddr));
    if (n < 0)
        throw cRuntimeError("Cannot bind socket: %d", n);
}

void ExtLowerUdp::connect(int socketId, const L3Address& remoteAddress, int remotePort)
{
    Socket *socket = nullptr;
    auto it = socketIdToSocketMap.find(socketId);
    if (it == socketIdToSocketMap.end())
        socket = open(socketId);
    else
        socket = it->second;
    struct sockaddr_in sockaddr;
    sockaddr.sin_family = PF_INET;
    sockaddr.sin_port = htons(remotePort);
    sockaddr.sin_addr.s_addr = htonl(remoteAddress.toIpv4().getInt());
#if !defined(linux) && !defined(__linux) && !defined(_WIN32)
    sockaddr.sin_len = sizeof(struct sockaddr_in);
#endif
    int n = ::connect(socket->fd, (struct sockaddr *)&sockaddr, sizeof(sockaddr));
    if (n < 0)
        throw cRuntimeError("Cannot connect socket: %d", n);
}

void ExtLowerUdp::close(int socketId)
{
    auto it = socketIdToSocketMap.find(socketId);
    if (it == socketIdToSocketMap.end())
        throw cRuntimeError("Unknown socket");
    else {
        auto socket = it->second;
        socketIdToSocketMap.erase(it);
        fdToSocketMap.erase(socket->fd);
        rtScheduler->removeCallback(socket->fd, this);
        delete socket;
    }
}

void ExtLowerUdp::processPacketFromUpper(Packet *packet)
{
    emit(packetReceivedFromUpperSignal, packet);
    auto socketId = packet->getTag<SocketReq>()->getSocketId();
    auto it = socketIdToSocketMap.find(socketId);
    if (it == socketIdToSocketMap.end())
        throw cRuntimeError("Unknown socket");
    else {
        auto socket = it->second;
        auto bytesChunk = packet->peekAllAsBytes();
        uint8_t buffer[packet->getByteLength()];
        size_t packetLength = bytesChunk->copyToBuffer(buffer, packet->getByteLength());
        ASSERT(packetLength == (size_t)packet->getByteLength());
        if (auto addressReq = packet->findTag<L3AddressReq>()) {
            struct sockaddr_in sockaddr;
            sockaddr.sin_family = PF_INET;
            sockaddr.sin_port = htons(packet->getTag<L4PortReq>()->getDestPort());
            sockaddr.sin_addr.s_addr = htonl(addressReq->getDestAddress().toIpv4().getInt());
#if !defined(linux) && !defined(__linux) && !defined(_WIN32)
            sockaddr.sin_len = sizeof(struct sockaddr_in);
#endif
            // type of buffer in sendto(): win: char *, linux: void *
            int n = ::sendto(socket->fd, (char *)buffer, packetLength, 0, (struct sockaddr *)&sockaddr, sizeof(sockaddr));
            if (n < 0)
                throw cRuntimeError("Calling sendto failed: %d", n);
        }
        else {
            // type of buffer in send(): win: char *, linux: void *
            int n = ::send(socket->fd, (char *)buffer, packetLength, 0);
            if (n < 0)
                throw cRuntimeError("Calling send failed: %d", n);
        }
        emit(packetSentSignal, packet);
    }
}

void ExtLowerUdp::processPacketFromLower(int fd)
{
    Enter_Method_Silent();
    auto it = fdToSocketMap.find(fd);
    if (it == fdToSocketMap.end())
        throw cRuntimeError("Unknown socket");
    else {
        auto socket = it->second;
        uint8_t buffer[1 << 16];
        struct sockaddr_in sockaddr;
        socklen_t socklen = sizeof(sockaddr);
        // type of buffer in recvfrom(): win: char *, linux: void *
        int n = ::recvfrom(fd, (char *)buffer, sizeof(buffer), 0, (struct sockaddr*)&sockaddr, &socklen);
        if (n < 0)
            throw cRuntimeError("Calling recv failed: %d", n);
        auto data = makeShared<BytesChunk>(static_cast<const uint8_t *>(buffer), n);
        auto packet = new Packet(nullptr, data);
        packet->addTag<SocketInd>()->setSocketId(socket->socketId);
        packet->addTag<L3AddressInd>()->setSrcAddress(Ipv4Address(ntohl(sockaddr.sin_addr.s_addr)));
        packet->addTag<L4PortInd>()->setSrcPort(ntohs(sockaddr.sin_port));
        packet->setName(packetPrinter.printPacketToString(packet, packetNameFormat).c_str());
        emit(packetReceivedSignal, packet);
        send(packet, "appOut");
        emit(packetSentToUpperSignal, packet);
    }
}

} // namespace inet


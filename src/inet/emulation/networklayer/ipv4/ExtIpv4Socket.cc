//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include <omnetpp/platdep/sockets.h>

#ifndef  __linux__
#error The 'Network Emulation Support' feature currently works on Linux systems only
#else

#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <sys/ioctl.h>

#include "inet/common/ModuleAccess.h"
#include "inet/common/NetworkNamespaceContext.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/Packet.h"
#include "inet/emulation/networklayer/ipv4/ExtIpv4Socket.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/NetworkInterface.h"

namespace inet {

Define_Module(ExtIpv4Socket);

ExtIpv4Socket::~ExtIpv4Socket()
{
    closeSocket();
}

void ExtIpv4Socket::initialize(int stage)
{
    cSimpleModule::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        packetNameFormat = par("packetName");
        rtScheduler = check_and_cast<RealTimeScheduler *>(getSimulation()->getScheduler());
        openSocket();
        numSent = numReceived = 0;
        WATCH(numSent);
        WATCH(numReceived);
    }
}

void ExtIpv4Socket::handleMessage(cMessage *msg)
{
    Packet *packet = check_and_cast<Packet *>(msg);
    if (packet->getTag<PacketProtocolTag>()->getProtocol() != &Protocol::ipv4)
        throw cRuntimeError("Invalid protocol");

    struct sockaddr_in ip_addr;
    ip_addr.sin_family = AF_INET;
#if !defined(linux) && !defined(__linux) && !defined(_WIN32)
    ip_addr.sin_len = sizeof(struct sockaddr_in);
#endif // if !defined(linux) && !defined(__linux) && !defined(_WIN32)
    ip_addr.sin_port = htons(0);

    auto bytesChunk = packet->peekAllAsBytes();
    uint8_t buffer[1 << 16];
    size_t packetLength = bytesChunk->copyToBuffer(buffer, sizeof(buffer));
    ASSERT(packetLength == (size_t)packet->getByteLength());

//    int sent = ::send(fd, buffer, packetLength, 0);
    int sent = sendto(fd, buffer, packetLength, 0, (struct sockaddr *)&ip_addr, sizeof(ip_addr));
    if ((size_t)sent == packetLength)
        EV << "Sent " << sent << " bytes packet.\n";
    else
        EV << "Sending packet FAILED! (sendto returned " << sent << " (" << strerror(errno) << ") instead of " << packetLength << ").\n";
    numSent++;
    delete packet;
}

void ExtIpv4Socket::refreshDisplay() const
{
    char buf[80];
    sprintf(buf, "snt:%d rcv:%d", numSent, numReceived);
    getDisplayString().setTagArg("t", 0, buf);
}

void ExtIpv4Socket::finish()
{
    std::cout << getFullPath() << ": " << numSent << " packets sent, " << numReceived << " packets received\n";
    closeSocket();
}

void ExtIpv4Socket::openSocket()
{
    NetworkNamespaceContext context(par("namespace"));
    fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (fd == INVALID_SOCKET)
        throw cRuntimeError("Cannot open socket");
    int hdrincl = 1;
    if (setsockopt(fd, IPPROTO_IP, IP_HDRINCL, &hdrincl, sizeof(hdrincl)) == -1)
        throw cRuntimeError("IP_HDRINCL");
    if (gate("upperLayerOut")->isConnected())
        rtScheduler->addCallback(fd, this);
}

void ExtIpv4Socket::closeSocket()
{
    if (fd != INVALID_SOCKET) {
        if (gate("upperLayerOut")->isConnected())
            rtScheduler->removeCallback(fd, this);
        close(fd);
        fd = INVALID_SOCKET;
    }
}

bool ExtIpv4Socket::notify(int fd)
{
    Enter_Method("notify");
    ASSERT(this->fd == fd);
    uint8_t buffer[1 << 16];
    memset(&buffer, 0, sizeof(buffer));
    // type of buffer in recvfrom(): win: char *, linux: void *
    int n = ::recv(fd, (char *)buffer, sizeof(buffer), 0);
    if (n < 0)
        throw cRuntimeError("Calling recvfrom failed: %d", n);
    auto data = makeShared<BytesChunk>(static_cast<const uint8_t *>(buffer), n);
    auto packet = new Packet(nullptr, data);
    packet->addTag<PacketProtocolTag>()->setProtocol(&Protocol::ipv4);
    packet->addTag<DispatchProtocolReq>()->setProtocol(&Protocol::ipv4);
    packet->setName(packetPrinter.printPacketToString(packet, packetNameFormat).c_str());
    emit(packetReceivedSignal, packet);
    send(packet, "upperLayerOut");
    emit(packetSentToUpperSignal, packet);
    return true;
}

} // namespace inet

#endif // __linux__


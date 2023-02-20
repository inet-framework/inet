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
#include "inet/common/checksum/EthernetCRC.h"
#include "inet/common/packet/Packet.h"
#include "inet/emulation/linklayer/ethernet/ExtEthernetSocket.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/NetworkInterface.h"

namespace inet {

Define_Module(ExtEthernetSocket);

ExtEthernetSocket::~ExtEthernetSocket()
{
    closeSocket();
}

void ExtEthernetSocket::initialize(int stage)
{
    cSimpleModule::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        device = par("device");
        packetNameFormat = par("packetNameFormat");
        rtScheduler = check_and_cast<RealTimeScheduler *>(getSimulation()->getScheduler());
        openSocket();
        numSent = numReceived = 0;
        WATCH(numSent);
        WATCH(numReceived);
    }
}

void ExtEthernetSocket::handleMessage(cMessage *message)
{
    Packet *packet = check_and_cast<Packet *>(message);
    emit(packetReceivedFromUpperSignal, packet);
    if (packet->getTag<PacketProtocolTag>()->getProtocol() != &Protocol::ethernetMac)
        throw cRuntimeError("Unaccepted packet protocol specified on upper layer incoming packet");

    struct sockaddr_ll socket_address;
    socket_address.sll_ifindex = ifindex;
    socket_address.sll_halen = ETH_ALEN;
    socket_address.sll_addr[0] = macAddress.getAddressByte(0);
    socket_address.sll_addr[1] = macAddress.getAddressByte(1);
    socket_address.sll_addr[2] = macAddress.getAddressByte(2);
    socket_address.sll_addr[3] = macAddress.getAddressByte(3);
    socket_address.sll_addr[4] = macAddress.getAddressByte(4);
    socket_address.sll_addr[5] = macAddress.getAddressByte(5);

    uint8_t buffer[packet->getByteLength()];
    auto bytesChunk = packet->peekAllAsBytes();
    size_t packetLength = bytesChunk->copyToBuffer(buffer, sizeof(buffer));
    ASSERT(packetLength == (size_t)packet->getByteLength());

    int sent = sendto(fd, buffer, packetLength, 0, (struct sockaddr *)&socket_address, sizeof(socket_address));
    if ((size_t)sent == packetLength)
        EV_INFO << "Sent " << packetLength << " packet to '" << device << "' device.\n";
    else
        EV_WARN << "Sending packet FAILED! (sendto returned " << sent << " B (" << strerror(errno) << ") instead of " << packetLength << ").\n";
    emit(packetSentSignal, packet);

    numSent++;
    delete packet;
}

void ExtEthernetSocket::refreshDisplay() const
{
    char buf[80];
    sprintf(buf, "device: %s\nsnt:%d rcv:%d", device, numSent, numReceived);
    getDisplayString().setTagArg("t", 0, buf);
}

void ExtEthernetSocket::finish()
{
    std::cout << numSent << " packets sent, " << numReceived << " packets received\n";
    closeSocket();
}

void ExtEthernetSocket::openSocket()
{
    NetworkNamespaceContext context(par("namespace"));
    // open socket
    struct ifreq if_mac;
    struct ifreq if_idx;
    fd = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW);
    if (fd == INVALID_SOCKET)
        throw cRuntimeError("Cannot open socket: %s", strerror(errno));
    // get the index of the interface to send on
    memset(&if_idx, 0, sizeof(struct ifreq));
    strncpy(if_idx.ifr_name, device, IFNAMSIZ - 1);
    if (ioctl(fd, SIOCGIFINDEX, &if_idx) < 0)
        throw cRuntimeError("Cannot get SIOCGIFINDEX: %s", strerror(errno));
    ifindex = if_idx.ifr_ifindex;
    // get the MAC address of the interface to send on
    memset(&if_mac, 0, sizeof(struct ifreq));
    strncpy(if_mac.ifr_name, device, IFNAMSIZ - 1);
    if (ioctl(fd, SIOCGIFHWADDR, &if_mac) < 0)
        throw cRuntimeError("Cannot get SIOCGIFHWADDR: %s", strerror(errno));
    macAddress.setAddressBytes(if_mac.ifr_hwaddr.sa_data);
    // bind to interface
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", device);
    if (setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr, sizeof(ifr)) < 0)
        throw cRuntimeError("Cannot bind raw socket to '%s' interface: %s", device, strerror(errno));
    // bind to all ethernet frames
    struct sockaddr_ll socket_address;
    memset(&socket_address, 0, sizeof(socket_address));
    socket_address.sll_family = PF_PACKET;
    socket_address.sll_ifindex = ifindex;
    socket_address.sll_protocol = htons(ETH_P_ALL);
    if (bind(fd, (struct sockaddr *)&socket_address, sizeof(socket_address)) < 0)
        throw cRuntimeError("Cannot bind socket: %s", strerror(errno));
    if (gate("upperLayerOut")->isConnected())
        rtScheduler->addCallback(fd, this);
}

void ExtEthernetSocket::closeSocket()
{
    if (fd != INVALID_SOCKET) {
        if (gate("upperLayerOut")->isConnected())
            rtScheduler->removeCallback(fd, this);
        close(fd);
        fd = INVALID_SOCKET;
    }
}

bool ExtEthernetSocket::notify(int fd)
{
    Enter_Method("notify");
    ASSERT(this->fd == fd);
    uint8_t buffer[1 << 16];
    memset(&buffer, 0, sizeof(buffer));
    // type of buffer in recvfrom(): win: char *, linux: void *
    int n = ::recv(fd, (char *)buffer, sizeof(buffer), 0);
    if (n < 0)
        throw cRuntimeError("Calling recvfrom failed: %d", n);
    n = std::max(n, ETHER_MIN_LEN - 4);
    uint32_t checksum = htonl(ethernetCRC(buffer, n));
    memcpy(&buffer[n], &checksum, sizeof(checksum));
    auto data = makeShared<BytesChunk>(static_cast<const uint8_t *>(buffer), n + 4);
    auto packet = new Packet(nullptr, data);
    auto networkInterface = check_and_cast<NetworkInterface *>(getContainingNicModule(this));
    packet->addTag<InterfaceInd>()->setInterfaceId(networkInterface->getInterfaceId());
    packet->addTag<PacketProtocolTag>()->setProtocol(&Protocol::ethernetMac);
    packet->addTag<DispatchProtocolReq>()->setProtocol(&Protocol::ethernetMac);
    packet->setName(packetPrinter.printPacketToString(packet, packetNameFormat).c_str());
    emit(packetReceivedSignal, packet);
    numReceived++;
    EV_INFO << "Received " << packet->getTotalLength() << " packet from '" << device << "' device.\n";
    send(packet, "upperLayerOut");
    emit(packetSentToUpperSignal, packet);
    return true;
}

} // namespace inet

#endif // __linux__


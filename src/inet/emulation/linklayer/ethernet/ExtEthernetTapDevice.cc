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
#include <linux/if_tun.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

#include "inet/common/ModuleAccess.h"
#include "inet/common/NetworkNamespaceContext.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/Simsignals.h"
#include "inet/common/packet/Packet.h"
#include "inet/emulation/linklayer/ethernet/ExtEthernetTapDevice.h"
#include "inet/linklayer/ethernet/common/Ethernet.h"
#include "inet/linklayer/ethernet/common/EthernetMacHeader_m.h"

namespace inet {

Define_Module(ExtEthernetTapDevice);

ExtEthernetTapDevice::~ExtEthernetTapDevice()
{
    closeTap();
}

void ExtEthernetTapDevice::initialize(int stage)
{
    cSimpleModule::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        device = par("device").stdstringValue();
        packetNameFormat = par("packetNameFormat");
        rtScheduler = check_and_cast<RealTimeScheduler *>(getSimulation()->getScheduler());
        openTap(device);
        numSent = numReceived = 0;
        WATCH(numSent);
        WATCH(numReceived);
    }
}

void ExtEthernetTapDevice::handleMessage(cMessage *msg)
{
    auto packet = check_and_cast<Packet *>(msg);
    emit(packetReceivedFromLowerSignal, packet);
    auto protocol = packet->getTag<PacketProtocolTag>()->getProtocol();
    if (protocol != &Protocol::ethernetMac)
        throw cRuntimeError("Accepts ethernet packets only");
    const auto& ethHeader = packet->peekAtFront<EthernetMacHeader>();
    packet->popAtBack<EthernetFcs>(ETHER_FCS_BYTES);
    auto bytesChunk = packet->peekDataAsBytes();
    uint8_t buffer[packet->getByteLength() + 4];
    buffer[0] = 0;
    buffer[1] = 0;
    buffer[2] = 0x86; // Ethernet
    buffer[3] = 0xdd;
    size_t packetLength = bytesChunk->copyToBuffer(buffer + 4, packet->getByteLength());
    ASSERT(packetLength == (size_t)packet->getByteLength());
    packetLength += 4;
    ssize_t nwrite = write(fd, buffer, packetLength);
    if ((size_t)nwrite == packetLength) {
        emit(packetSentSignal, packet);
        EV_INFO << "Sent a " << packet->getTotalLength() << " packet from " << ethHeader->getSrc() << " to " << ethHeader->getDest() << " to TAP device '" << device << "'.\n";
        numSent++;
    }
    else
        EV_ERROR << "Sending Ethernet packet FAILED! (sendto returned " << nwrite << " (" << strerror(errno) << ") instead of " << packetLength << ").\n";
    delete packet;
}

void ExtEthernetTapDevice::refreshDisplay() const
{
    char buf[180];
    sprintf(buf, "TAP device: %s\nrcv:%d snt:%d", device.c_str(), numReceived, numSent);
    getDisplayString().setTagArg("t", 0, buf);
}

void ExtEthernetTapDevice::finish()
{
    EV_INFO << numSent << " packets sent, " << numReceived << " packets received.\n";
    closeTap();
}

void ExtEthernetTapDevice::openTap(std::string dev)
{
    NetworkNamespaceContext context(par("namespace"));
    if ((fd = open("/dev/net/tun", O_RDWR)) < 0)
        throw cRuntimeError("Cannot open TAP device: %s", strerror(errno));

    // preparation of the struct ifr, of type "struct ifreq"
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = IFF_TAP; /* IFF_TUN or IFF_TAP, plus maybe IFF_NO_PI */
    if (!dev.empty())
        /* if a device name was specified, put it in the structure; otherwise,
         * the kernel will try to allocate the "next" device of the
         * specified type */
        strncpy(ifr.ifr_name, dev.c_str(), IFNAMSIZ);
    if (ioctl(fd, (TUNSETIFF), (void *)&ifr) < 0) {
        close(fd);
        throw cRuntimeError("Cannot create TAP device: %s", strerror(errno));
    }

    /* if the operation was successful, write back the name of the
     * interface to the variable "dev", so the caller can know
     * it. Note that the caller MUST reserve space in *dev (see calling
     * code below) */
    dev = ifr.ifr_name;
    rtScheduler->addCallback(fd, this);
}

void ExtEthernetTapDevice::closeTap()
{
    if (fd != INVALID_SOCKET) {
        rtScheduler->removeCallback(fd, this);
        close(fd);
        fd = -1;
    }
}

bool ExtEthernetTapDevice::notify(int fd)
{
    Enter_Method("notify");
    ASSERT(fd == this->fd);
    uint8_t buffer[1 << 16];
    ssize_t nread = read(fd, buffer, sizeof(buffer));
    if (nread < 0) {
        close(fd);
        throw cRuntimeError("Cannot read '%s' device: %s", device.c_str(), strerror(errno));
    }
    else if (nread > 0) {
        ASSERT(nread > 4);
        // buffer[0..1]: flags, buffer[2..3]: ethertype
        Packet *packet = new Packet(nullptr, makeShared<BytesChunk>(buffer + 4, nread - 4));
        auto ethernetFcs = makeShared<EthernetFcs>();
        ethernetFcs->setFcsMode(FCS_COMPUTED); // TODO get fcsMode from NED parameter
        packet->insertAtBack(ethernetFcs);
        packet->addTag<DispatchProtocolReq>()->setProtocol(&Protocol::ethernetMac);
        packet->addTag<PacketProtocolTag>()->setProtocol(&Protocol::ethernetMac);
        packet->setName(packetPrinter.printPacketToString(packet, packetNameFormat).c_str());
        emit(packetReceivedSignal, packet);
        const auto& macHeader = packet->peekAtFront<EthernetMacHeader>();
        EV_INFO << "Received a " << packet->getTotalLength() << " packet from " << macHeader->getSrc() << " to " << macHeader->getDest() << ".\n";
        send(packet, "lowerLayerOut");
        emit(packetSentToLowerSignal, packet);
        numReceived++;
        return true;
    }
    else
        return false;
}

} // namespace inet

#endif // __linux__


//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#define WANT_WINSOCK2

#include <arpa/inet.h>
#include <fcntl.h>
#include <linux/if_tun.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <omnetpp/platdep/sockets.h>

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/NetworkNamespaceContext.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/emulation/networklayer/ipv4/ExtIpv4TunDevice.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"

namespace inet {

Define_Module(ExtIpv4TunDevice);

ExtIpv4TunDevice::~ExtIpv4TunDevice()
{
    closeTun();
}

void ExtIpv4TunDevice::initialize(int stage)
{
    cSimpleModule::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        device = par("device").stdstringValue();
        packetNameFormat = par("packetNameFormat");
        rtScheduler = check_and_cast<RealTimeScheduler *>(getSimulation()->getScheduler());
        registerService(Protocol::ipv4, nullptr, gate("lowerLayerIn"));
        registerProtocol(Protocol::ipv4, gate("lowerLayerOut"), nullptr);
        openTun(device);
        numSent = numReceived = 0;
        WATCH(numSent);
        WATCH(numReceived);
    }
}

void ExtIpv4TunDevice::handleMessage(cMessage *msg)
{
    auto packet = check_and_cast<Packet *>(msg);
    emit(packetReceivedFromLowerSignal, packet);
    auto protocol = packet->getTag<PacketProtocolTag>()->getProtocol();
    if (protocol != &Protocol::ipv4)
        throw cRuntimeError("ExtInterface accepts IPv4 packets only");
    const auto& ipv4Header = packet->peekAtFront<Ipv4Header>();
    auto bytesChunk = packet->peekDataAsBytes();
    uint8_t buffer[packet->getByteLength()];
    size_t packetLength = bytesChunk->copyToBuffer(buffer, packet->getByteLength());
    ASSERT(packetLength == (size_t)packet->getByteLength());
    ssize_t nwrite = write(fd, buffer, packetLength);
    if ((size_t)nwrite == packetLength) {
        emit(packetSentSignal, packet);
        EV_INFO << "Sent a " << packet->getTotalLength() << " packet from " << ipv4Header->getSrcAddress() << " to " << ipv4Header->getDestAddress() << " to TUN device '" << device << "'.\n";
        numSent++;
    }
    else
        EV_ERROR << "Sending IPv4 packet FAILED! (sendto returned " << nwrite << " (" << strerror(errno) << ") instead of " << packetLength << ").\n";
    delete packet;
}

void ExtIpv4TunDevice::refreshDisplay() const
{
    char buf[180];
    sprintf(buf, "TUN device: %s\nrcv:%d snt:%d", device.c_str(), numReceived, numSent);
    getDisplayString().setTagArg("t", 0, buf);
}

void ExtIpv4TunDevice::finish()
{
    EV_INFO << numSent << " packets sent, " << numReceived << " packets received.\n";
    closeTun();
}

void ExtIpv4TunDevice::openTun(std::string dev)
{
    NetworkNamespaceContext context(par("namespace"));
    if ((fd = open("/dev/net/tun", O_RDWR)) < 0)
        throw cRuntimeError("Cannot open TUN device: %s", strerror(errno));

    // preparation of the struct ifr, of type "struct ifreq"
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = IFF_TUN + IFF_NO_PI; /* IFF_TUN or IFF_TAP, plus maybe IFF_NO_PI */
    if (!dev.empty())
        /* if a device name was specified, put it in the structure; otherwise,
         * the kernel will try to allocate the "next" device of the
         * specified type */
        strncpy(ifr.ifr_name, dev.c_str(), IFNAMSIZ);
    if (ioctl(fd, (TUNSETIFF), (void *) &ifr) < 0) {
        close(fd);
        throw cRuntimeError("Cannot create TUN device: %s", strerror(errno));
    }

    /* if the operation was successful, write back the name of the
     * interface to the variable "dev", so the caller can know
     * it. Note that the caller MUST reserve space in *dev (see calling
     * code below) */
    dev = ifr.ifr_name;
    rtScheduler->addCallback(fd, this);
}

void ExtIpv4TunDevice::closeTun()
{
    if (fd != INVALID_SOCKET) {
        rtScheduler->removeCallback(fd, this);
        close(fd);
        fd = -1;
    }
}

bool ExtIpv4TunDevice::notify(int fd)
{
    Enter_Method_Silent();
    ASSERT(fd == this->fd);
    uint8_t buffer[1 << 16];
    ssize_t nread = read(fd, buffer, sizeof(buffer));
    if (nread < 0) {
        close(fd);
        throw cRuntimeError("Cannot read '%s' device: %s", device.c_str(), strerror(errno));
    }
    else if (nread > 0) {
        Packet *packet = new Packet(nullptr, makeShared<BytesChunk>(buffer, nread));
        // KLUDGE:
        packet->addTag<InterfaceReq>()->setInterfaceId(101);
        packet->addTag<PacketProtocolTag>()->setProtocol(&Protocol::ipv4);
        packet->setName(packetPrinter.printPacketToString(packet, packetNameFormat).c_str());
        emit(packetReceivedSignal, packet);
        const auto& ipv4Header = packet->peekAtFront<Ipv4Header>();
        EV_INFO << "Received a " << packet->getTotalLength() << " packet from " << ipv4Header->getSrcAddress() << " to " << ipv4Header->getDestAddress() << ".\n";
        send(packet, "lowerLayerOut");
        emit(packetSentToLowerSignal, packet);
        numReceived++;
        return true;
    }
    else
        return false;
}

} // namespace inet


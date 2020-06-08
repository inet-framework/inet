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
#include "inet/common/packet/Packet.h"
#include "inet/emulation/linklayer/ethernet/ExtEthernetTapDevice.h"
#include "inet/linklayer/ethernet/EtherEncap.h"
#include "inet/linklayer/ethernet/EtherFrame_m.h"

#include "inet/common/LinuxUtils.h"

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

    size_t packetLength = bytesChunk->getByteArraySize();
    ssize_t nwrite = write(fd, bytesChunk->getBytes().data(), packetLength);
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

/*


ip tuntap add mode tap dev tap0

 ip link set up dev tap0
 ip link set up dev tap0 address 2e:6f:c3:7f:6e:cf

 ip addr add 192.168.10.2 dev tap0
 ip addr add 192.168.10.2/24 dev tap0
 X ip addr del fe80::440d:27ff:fe14:d7ca/64 dev tap0

 ? ip route add 192.168.10.0/24 dev tap0

 ip tuntap del dev tap0 mode tap

 ip netns del srv
 */

void ExtEthernetTapDevice::openTap(std::string dev)
{
    NetworkNamespaceContext context(par("namespace"));

    if (!checkTapDeviceExists(dev))
        run_command({"ip", "tuntap", "add", "mode", "tap", "dev", dev.c_str()}, true, true);


    if ((fd = open("/dev/net/tun", O_RDWR)) < 0)
        throw cRuntimeError("Cannot open TAP device: %s", strerror(errno));

    // preparation of the struct ifr, of type "struct ifreq"
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = IFF_TAP | IFF_NO_PI; /* IFF_TUN or IFF_TAP, plus maybe IFF_NO_PI */
    if (!dev.empty())
        /* if a device name was specified, put it in the structure; otherwise,
         * the kernel will try to allocate the "next" device of the
         * specified type */
        strncpy(ifr.ifr_name, dev.c_str(), IFNAMSIZ);
    if (ioctl(fd, (TUNSETIFF), (void *) &ifr) < 0) {
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
        packet->insertAtBack(makeShared<EthernetFcs>(FCS_COMPUTED));    //TODO get fcsMode from NED parameter
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


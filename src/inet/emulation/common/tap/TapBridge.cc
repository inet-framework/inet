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

#include <stdio.h>
#include <string.h>

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/if_tun.h>
#include <unistd.h>

#include "inet/common/INETDefs.h"

#include <omnetpp/platdep/sockets.h>

#include "inet/common/INETUtils.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/chunk/BytesChunk.h"

#include "inet/linklayer/common/EtherType_m.h"
#include "inet/linklayer/common/Ieee802Ctrl_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/ethernet/EtherEncap.h"
#include "inet/linklayer/ethernet/EtherFrame_m.h"
#include "inet/linklayer/ethernet/Ethernet.h"
#include "inet/linklayer/ethernet/EtherPhyFrame_m.h"
#include "inet/emulation/common/tap/TapBridge.h"

#include "inet/networklayer/common/InterfaceEntry.h"
#include "inet/networklayer/common/IpProtocolId_m.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"

namespace inet {

Define_Module(TapBridge);

int openTap(std::string dev) {
    struct ifreq ifr;
    int fd, err;
    const char *clonedev = "/dev/net/tun";

    /* Arguments taken by the function:
     *
     * char *dev: the name of an interface (or '\0'). MUST have enough
     *   space to hold the interface name if '\0' is passed
     * int flags: interface flags (eg, IFF_TUN etc.)
     */

    /* open the clone device */
    if ((fd = open(clonedev, O_RDWR)) < 0) {
        return fd;
    }

    /* preparation of the struct ifr, of type "struct ifreq" */
    memset(&ifr, 0, sizeof(ifr));

    ifr.ifr_flags = IFF_TAP; /* IFF_TUN or IFF_TAP, plus maybe IFF_NO_PI */

    if (!dev.empty()) {
        /* if a device name was specified, put it in the structure; otherwise,
         * the kernel will try to allocate the "next" device of the
         * specified type */
        strncpy(ifr.ifr_name, dev.c_str(), IFNAMSIZ);
    }

    /* try to create the device */
    if ((err = ioctl(fd, (TUNSETIFF), (void *) &ifr)) < 0) {
        close(fd);
        return err;
    }

    /* if the operation was successful, write back the name of the
     * interface to the variable "dev", so the caller can know
     * it. Note that the caller MUST reserve space in *dev (see calling
     * code below) */
    dev = ifr.ifr_name;

    /* this is the special file descriptor that the caller will use to talk
     * with the virtual interface */
    return fd;
}

bool TapBridge::notify(int fd)
{
    ASSERT(fd == this->tapFd);
    ssize_t nread = read(fd, buffer, bufferLength);
    if(nread < 0) {
        perror("Reading from interface");
        close(fd);
        throw cRuntimeError("Tap::notify(): An error occured on interface '%s': %s", device.c_str(), strerror(errno));
    }
    else if (nread > 0) {
        ASSERT (nread > 4);
        std::string pkName = device + "Captured" + std::to_string(pkId);
        pkId++;
        // buffer[0..1]: flags, buffer[2..3]: ethertype
        Packet *packet = new Packet(pkName.c_str(), makeShared<BytesChunk>(buffer + 4, nread - 4));
        EtherEncap::addPaddingAndFcs(packet, FCS_COMPUTED);
        packet->addTag<DispatchProtocolReq>()->setProtocol(&Protocol::ethernetMac);
        packet->addTag<PacketProtocolTag>()->setProtocol(&Protocol::ethernetMac);
        EV << "Captured packet, length is " << packet->getTotalLength() << endl;

        rtScheduler->scheduleMessage(this, packet);
        return true;
    }
    else
        return false;
}

TapBridge::~TapBridge()
{
    rtScheduler->removeCallback(tapFd, this);
    close(tapFd);
}

void TapBridge::initialize(int stage)
{
    // subscribe at scheduler for external messages
    if (stage == INITSTAGE_LOCAL) {
        numSent = numRcvd = numDropped = pkId = 0;

        if (auto scheduler = dynamic_cast<IFdScheduler *>(getSimulation()->getScheduler())) {
            rtScheduler = scheduler;
            device = par("device").stdstringValue();

            // Enabling sending makes no sense when we can't receive...
            tapFd = openTap(device);
            if (tapFd == INVALID_SOCKET)
                throw cRuntimeError("Tap interface: open: error occured: %s", strerror(errno));
            rtScheduler->addCallback(tapFd, this);
            connected = true;
        }
        else {
            // this simulation run works without external interface
            connected = false;
        }

        WATCH(numSent);
        WATCH(numRcvd);
        WATCH(numDropped);
    }
}

void TapBridge::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        Packet *packet = check_and_cast<Packet *>(msg);
        // incoming real packet from real host (captured by tap interface)
        const auto& nwHeader = packet->peekAtFront<EthernetMacHeader>();
        EV << "Delivering a packet from "
           << nwHeader->getSrc()
           << " to "
           << nwHeader->getDest()
           << " and length of"
           << packet->getByteLength()
           << " bytes to networklayer.\n";
        send(packet, "fromTap");
        numRcvd++;
    }
    else {
        // incoming packet from lower layer, sending it to tap interface
        auto packet = check_and_cast<Packet *>(msg);
        auto protocol = packet->getTag<PacketProtocolTag>()->getProtocol();
        if (protocol != &Protocol::ethernetMac)
            throw cRuntimeError("ExtInterface accepts ethernet packets only");
        const auto& ethHeader = packet->peekAtFront<EthernetMacHeader>();
        if (connected) {
            if (tapFd == INVALID_SOCKET)
                throw cRuntimeError("Tap: doesn't have socket for tap interface.");
            auto bytesChunk = packet->peekDataAsBytes();
            buffer[0] = 0;
            buffer[1] = 0;
            buffer[2] = 0x86;   // ethernet
            buffer[3] = 0xdd;
            size_t packetLength = bytesChunk->copyToBuffer(buffer+4, bufferLength-4);
            ASSERT(packetLength == (size_t)packet->getByteLength());
            packetLength += 4;
            ssize_t nwrite = write(tapFd, buffer, packetLength);
            if ((size_t)nwrite == packetLength) {
                EV << "Sending a packet from "
                   << ethHeader->getSrc()
                   << " to "
                   << ethHeader->getDest()
                   << " and length of "
                   << packet->getByteLength()
                   << " bytes to tap device '" << device << "'.\n";
                numSent++;
            }
            else
                EV_ERROR << "Sending of an ethernet packet FAILED! (sendto returned " << nwrite << " (" << strerror(errno) << ") instead of " << packetLength << ").\n";
            delete packet;
        }
        else {
            EV_WARN << "Interface is not connected, dropping packet " << msg << endl;
            numDropped++;
            delete packet;
        }
    }
}

void TapBridge::refreshDisplay() const
{
    if (connected) {
        char buf[180];
        sprintf(buf, "tap device: %s\nrcv:%d snt:%d", device.c_str(), numRcvd, numSent);
        getDisplayString().setTagArg("t", 0, buf);
    }
    else {
        getDisplayString().setTagArg("t", 0, "not connected");
    }
}

void TapBridge::finish()
{
    rtScheduler->removeCallback(tapFd, this);
    EV << getFullPath() << ": " << numSent << " packets sent, "
              << numRcvd << " packets received, " << numDropped << " packets dropped.\n";
    //close tap socket:
    close(tapFd);
    tapFd = INVALID_SOCKET;
}

} // namespace inet


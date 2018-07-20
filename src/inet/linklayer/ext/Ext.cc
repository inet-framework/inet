//
// Copyright (C) 2004 Andras Varga
// Copyright (C) 2005 Christian Dankbar, Irene Ruengeler, Michael Tuexen
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

// This file is based on the Ppp.cc of INET written by Andras Varga.

#define WANT_WINSOCK2

#include <stdio.h>
#include <string.h>

#include "inet/common/INETDefs.h"

#include <omnetpp/platdep/sockets.h>

#include <net/if.h>

#include "inet/common/INETUtils.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/chunk/BytesChunk.h"

#include "inet/linklayer/common/EtherType_m.h"
#include "inet/linklayer/common/Ieee802Ctrl_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/ethernet/Ethernet.h"
#include "inet/linklayer/ext/Ext.h"

#include "inet/networklayer/common/InterfaceEntry.h"
#include "inet/networklayer/common/InterfaceTable.h"
#include "inet/networklayer/common/IpProtocolId_m.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"

#include <arpa/inet.h>

namespace inet {

Define_Module(Ext);

void Ext::ext_packet_handler(u_char *usermod, const struct pcap_pkthdr *hdr, const u_char *bytes)
{
    EV_STATICCONTEXT;
    Ext *module = (Ext*)usermod;

    //FIXME Why? Could we use the pcap for filtering incoming IPv4 packet?
    //FIXME Why filtering IPv4 only on eth interface? why not filtering on PPP or other interfaces?
    // skip ethernet frames not encapsulating an IP packet.
    // TODO: how about ipv6 and other protocols?
    if (module->datalink == DLT_EN10MB && B(hdr->caplen) > ETHER_MAC_HEADER_BYTES) {
        //TODO for decapsulate, using code from EtherEncap
        uint16_t etherType = (uint16_t)(bytes[B(ETHER_ADDR_LEN).get() * 2]) << 8 | bytes[B(ETHER_ADDR_LEN).get() * 2 + 1];
        //TODO get ethertype from snap header when packet has snap header
        if (etherType != ETHERTYPE_IPv4) // ipv4
            return;
    }
    //TODO for other DLT_ : decapsulate
    //TODO or move decapsulation to Ext interface

    // put the IP packet from wire into Packet
    uint32_t pklen = hdr->caplen - module->headerLength;
    Packet *packet = new Packet("rtEvent");
    const auto& bytesChunk = makeShared<BytesChunk>(bytes + module->headerLength, pklen);
    packet->insertAtBack(bytesChunk);

    // signalize new incoming packet to the interface via cMessage
    EV << "Captured " << pklen << " bytes for an IP packet.\n";
    module->rtScheduler->scheduleMessage(module, packet);
}

bool Ext::notify(int fd)
{
    ASSERT(fd == pcap_socket);
    bool found = false;
    int32 n = pcap_dispatch(pd, 1, ext_packet_handler, (u_char *)this);
    if (n < 0)
        throw cRuntimeError("Ext::notify(): An error occured: %s", pcap_geterr(pd));
    if (n > 0)
        found = true;
    return found;
}

Ext::~Ext()
{
    //close raw socket:
    close(fd);
    if (pd)
        pcap_close(pd);
    rtScheduler->removeCallback(pcap_socket, this);
}

void Ext::initialize(int stage)
{
    MacBase::initialize(stage);

    // subscribe at scheduler for external messages
    if (stage == INITSTAGE_LOCAL) {
        numSent = numRcvd = numDropped = 0;

        if (auto scheduler = dynamic_cast<RealTimeScheduler *>(getSimulation()->getScheduler())) {
            rtScheduler = scheduler;
            device = par("device");
            const char *filter = par("filterString");
            openPcap(device, filter);
            rtScheduler->addCallback(pcap_socket, this);
            connected = true;

            // Enabling sending makes no sense when we can't receive...
            fd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
            if (fd == INVALID_SOCKET)
                throw cRuntimeError("Ext interface: Root privileges needed");
            const int32 on = 1;
            if (setsockopt(fd, IPPROTO_IP, IP_HDRINCL, (char *)&on, sizeof(on)) < 0)
                throw cRuntimeError("Ext: couldn't set sockopt for raw socket");

            // bind to interface:
            struct ifreq ifr;
            memset(&ifr, 0, sizeof(ifr));
            snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", device);
            if (setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr, sizeof(ifr)) < 0) {
                throw cRuntimeError("Ext: couldn't bind raw socket to '%s' interface", device);
            }
        }
        else {
            // this simulation run works without external interface
            connected = false;
        }

        WATCH(numSent);
        WATCH(numRcvd);
        WATCH(numDropped);
    }
    else if (stage == INITSTAGE_LINK_LAYER) {
        registerInterface();
    }
}

void Ext::openPcap(const char *device, const char *filter)
{
    char errbuf[PCAP_ERRBUF_SIZE];
    struct bpf_program fcode;
    pcap_t *pd;
    int32 datalink;
    int32 headerLength;

    if (!device || !filter)
        throw cRuntimeError("arguments must be non-nullptr");

    /* get pcap handle */
    memset(&errbuf, 0, sizeof(errbuf));
    if ((pd = pcap_create(device, errbuf)) == nullptr)
        throw cRuntimeError("Cannot create pcap device, error = %s", errbuf);
    else if (strlen(errbuf) > 0)
        EV << "pcap_open_live returned warning: " << errbuf << "\n";

    /* apply the immediate mode to pcap */
    if (pcap_set_immediate_mode(pd, 1) != 0)
            throw cRuntimeError("Cannot set immediate mode to pcap device");

    if (pcap_activate(pd) != 0)
        throw cRuntimeError("Cannot activate pcap device");

    /* compile this command into a filter program */
    if (pcap_compile(pd, &fcode, filter, 0, 0) < 0)
        throw cRuntimeError("Cannot compile pcap filter: %s", pcap_geterr(pd));

    /* apply the compiled filter to the packet capture device */
    if (pcap_setfilter(pd, &fcode) < 0)
        throw cRuntimeError("Cannot apply compiled pcap filter: %s", pcap_geterr(pd));

    if ((datalink = pcap_datalink(pd)) < 0)
        throw cRuntimeError("Cannot query pcap link-layer header type: %s", pcap_geterr(pd));

#ifndef __linux__
    if (pcap_setnonblock(pd, 1, errbuf) < 0)
        throw cRuntimeError("Cannot put pcap device into non-blocking mode, error: %s", errbuf);
#endif

    switch (datalink) {
        case DLT_NULL:
            headerLength = 4;
            break;

        case DLT_EN10MB:
            headerLength = 14;
            break;

        case DLT_SLIP:
            headerLength = 24;
            break;

        case DLT_PPP:
            headerLength = 24;
            break;

        default:
            throw cRuntimeError("RealTimeScheduler::setInterfaceModule(): Unsupported datalink: %d", datalink);
    }

    this->pd = pd;
    this->datalink = datalink;
    this->headerLength = headerLength;
#ifdef __linux__
    this->pcap_socket = pcap_get_selectable_fd(pd);
#endif

    EV << "Opened pcap device " << device << " with filter " << filter << " and datalink " << datalink << ".\n";
}

InterfaceEntry *Ext::createInterfaceEntry()
{
    InterfaceEntry *e = getContainingNicModule(this);

    e->setMtu(par("mtu"));      //TODO get mtu from real interface / or set mtu in real interface
    e->setMulticast(true);      //TODO
    e->setPointToPoint(true);   //TODO

    return e;
}

void Ext::handleMessage(cMessage *msg)
{
    if (!isOperational) {
        handleMessageWhenDown(msg);
        return;
    }

    Packet *packet = check_and_cast<Packet *>(msg);

    if (msg->isSelfMessage()) {
        // incoming real packet from wire (captured by pcap)
        const auto& nwHeader = packet->peekAtFront<Ipv4Header>();
        EV << "Delivering a packet from "
           << nwHeader->getSourceAddress()
           << " to "
           << nwHeader->getDestinationAddress()
           << " and length of"
           << packet->getByteLength()
           << " bytes to networklayer.\n";
        packet->addTagIfAbsent<InterfaceInd>()->setInterfaceId(interfaceEntry->getInterfaceId());
        packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&Protocol::ipv4);
        packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ipv4);
        send(packet, "upperLayerOut");
        numRcvd++;
    }
    else {
        auto protocol = packet->getTag<PacketProtocolTag>()->getProtocol();
        if (protocol != &Protocol::ipv4)
            throw cRuntimeError("ExtInterface accepts ipv4 packets only");

        const auto& ipv4Header = packet->peekAtFront<Ipv4Header>();

        if (connected) {
            struct sockaddr_in addr;
            addr.sin_family = AF_INET;
#if !defined(linux) && !defined(__linux) && !defined(_WIN32)
            addr.sin_len = sizeof(struct sockaddr_in);
#endif // if !defined(linux) && !defined(__linux) && !defined(_WIN32)
            addr.sin_port = htons(0);
            addr.sin_addr.s_addr = htonl(ipv4Header->getDestAddress().getInt());
            auto bytesChunk = packet->peekAllAsBytes();
            size_t packetLength = bytesChunk->copyToBuffer(buffer, sizeof(buffer));
            ASSERT(packetLength == (size_t)packet->getByteLength());

            EV << "Delivering an IPv4 packet from "
               << ipv4Header->getSrcAddress()
               << " to "
               << ipv4Header->getDestAddress()
               << " and length of "
               << packet->getByteLength()
               << " bytes to link layer.\n";
            sendBytes(buffer, packetLength, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
            numSent++;
            delete packet;
        }
        else {
            EV << "Interface is not connected, dropping packet " << msg << endl;
            numDropped++;
        }
    }
}

void Ext::sendBytes(uint8 *buf, size_t numBytes, struct sockaddr *to, socklen_t addrlen)
{
    //TODO check: is this an IPv4 packet --OR-- is this packet acceptable by fd socket?
    if (fd == INVALID_SOCKET)
        throw cRuntimeError("RealTimeScheduler::sendBytes(): no raw socket.");

    int sent = sendto(fd, buf, numBytes, 0, to, addrlen);    //note: no ssize_t on MSVC

    if ((size_t)sent == numBytes)
        EV << "Sent an IP packet with length of " << sent << " bytes.\n";
    else
        EV << "Sending of an IP packet FAILED! (sendto returned " << sent << " (" << strerror(errno) << ") instead of " << numBytes << ").\n";
    return;
}


void Ext::displayBusy()
{
    getDisplayString().setTagArg("i", 1, "yellow");
}

void Ext::displayIdle()
{
    getDisplayString().setTagArg("i", 1, "");
}

void Ext::refreshDisplay() const
{
    if (connected) {
        char buf[80];
        sprintf(buf, "pcap device: %s\nrcv:%d snt:%d", device, numRcvd, numSent);
        getDisplayString().setTagArg("t", 0, buf);
    }
    else {
        getDisplayString().setTagArg("i", 1, "#707070");
        getDisplayString().setTagArg("i", 2, "100");
        getDisplayString().setTagArg("t", 0, "not connected");
    }
}

void Ext::finish()
{
    rtScheduler->removeCallback(pcap_socket, this);
    std::cout << getFullPath() << ": " << numSent << " packets sent, "
              << numRcvd << " packets received, " << numDropped << " packets dropped.\n";
    //close raw socket:
    close(fd);
    fd = INVALID_SOCKET;

    // close pcap:
    pcap_stat ps;
    if (pcap_stats(pd, &ps) < 0)
        throw cRuntimeError("Cannot query pcap statistics: %s", pcap_geterr(pd));
    EV << "Received Packets: " << ps.ps_recv << " Dropped Packets: " << ps.ps_drop << ".\n";
    recordScalar("Received Packets", ps.ps_recv);
    recordScalar("Dropped Packets", ps.ps_drop);
    if (pd) {
        pcap_close(pd);
        pd = nullptr;
    }
    pcap_socket = -1;
}

void Ext::flushQueue()
{
    // does not have a queue, do nothing
}

void Ext::clearQueue()
{
    // does not have a queue, do nothing
}

} // namespace inet


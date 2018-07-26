//
// Copyright (C) OpenSim Ltd.
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

#define WANT_WINSOCK2

#include <omnetpp/platdep/sockets.h>
#include <net/if.h>
#include <arpa/inet.h>

#include "inet/common/ModuleAccess.h"
#include "inet/common/packet/Packet.h"
#include "inet/emulation/common/PcapCapturer.h"

namespace inet {

Define_Module(PcapCapturer);

void PcapCapturer::ext_packet_handler(u_char *usermod, const struct pcap_pkthdr *hdr, const u_char *bytes)
{
    EV_STATICCONTEXT;
    PcapCapturer *module = (PcapCapturer*)usermod;
    Packet *packet = new Packet("CapturedPacket", makeShared<BytesChunk>(bytes, hdr->caplen));
    EV << "Captured " << packet->getTotalLength() << " packet" << endl;
    module->realTimeScheduler->scheduleMessage(module, packet);
}

bool PcapCapturer::notify(int fd)
{
    ASSERT(this->fd == fd);
    int32 n = pcap_dispatch(pd, 1, ext_packet_handler, (u_char *)this);
    if (n < 0)
        throw cRuntimeError("PcapCapturer::notify(): An error occured: %s", pcap_geterr(pd));
    return n > 0;
}

PcapCapturer::~PcapCapturer()
{
    if (pd != nullptr)
        pcap_close(pd);
    realTimeScheduler->removeCallback(fd, this);
}

void PcapCapturer::initialize(int stage)
{
    cSimpleModule::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        device = par("device");
        realTimeScheduler = dynamic_cast<RealTimeScheduler *>(getSimulation()->getScheduler());
        openPcap(device, par("filterString"));
        realTimeScheduler->addCallback(fd, this);
        WATCH(numRcvd);
    }
}

void PcapCapturer::openPcap(const char *device, const char *filter)
{
    char errbuf[PCAP_ERRBUF_SIZE];
    struct bpf_program fcode;
    pcap_t *pd;
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

    this->pd = pd;
#ifdef __linux__
    fd = pcap_get_selectable_fd(pd);
    if (fd == -1)
        throw cRuntimeError("Cannot get pcap fd");
#endif

    EV << "Opened pcap device " << device << " with filter " << filter << " and datalink " << datalink << ".\n";
}

void PcapCapturer::handleMessage(cMessage *msg)
{
    Packet *packet = check_and_cast<Packet *>(msg);
    if (msg->isSelfMessage()) {
        send(packet, "upperLayerOut");
        numRcvd++;
    }
    else
        throw cRuntimeError("Unknown message");
}

void PcapCapturer::refreshDisplay() const
{
    char buf[80];
    sprintf(buf, "device: %s\nrcv:%d", device, numRcvd);
    getDisplayString().setTagArg("t", 0, buf);
}

void PcapCapturer::finish()
{
    realTimeScheduler->removeCallback(fd, this);
    std::cout << getFullPath() << ": " << numRcvd << " packets received.\n";
    pcap_stat ps;
    if (pcap_stats(pd, &ps) < 0)
        throw cRuntimeError("Cannot query pcap statistics: %s", pcap_geterr(pd));
    EV << "Received Packets: " << ps.ps_recv << " Dropped Packets: " << ps.ps_drop << ".\n";
    recordScalar("Received Packets", ps.ps_recv);
    recordScalar("Dropped Packets", ps.ps_drop);
    if (pd != nullptr) {
        pcap_close(pd);
        pd = nullptr;
    }
    fd = -1;
}

} // namespace inet


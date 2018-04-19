//
// Copyright (C) 2005-2009 Andras Varga,
//                         Christian Dankbar,
//                         Irene Ruengeler,
//                         Michael Tuexen
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

// This file is based on the cSocketRTScheduler.cc of OMNeT++ written by
// Andras Varga.

#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/serializer/headers/ethernethdr.h"
#include "inet/linklayer/common/Ieee802Ctrl_m.h"
#include "inet/linklayer/common/EtherType_m.h"
#include "inet/linklayer/ext/cSocketRTScheduler.h"

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32) || defined(__CYGWIN__) || defined(_WIN64)
#include <ws2tcpip.h>
#endif // if defined(_WIN32) || defined(__WIN32__) || defined(WIN32) || defined(__CYGWIN__) || defined(_WIN64)

#define PCAP_SNAPLEN    65536 /* capture all data packets with up to pcap_snaplen bytes */

#ifdef __linux__
#define UI_REFRESH_TIME 100000 /* refresh time of the UI in us */
#else
#define UI_REFRESH_TIME 500
#endif

namespace inet {

#define FES(sim) (sim->getFES())

std::vector<cSocketRTScheduler::ExtConn> cSocketRTScheduler::conn;

int64_t cSocketRTScheduler::baseTime;

Register_Class(cSocketRTScheduler);

cSocketRTScheduler::cSocketRTScheduler() : cScheduler()
{
    fd = INVALID_SOCKET;
}

cSocketRTScheduler::~cSocketRTScheduler()
{
}

void cSocketRTScheduler::startRun()
{
    baseTime = opp_get_monotonic_clock_usecs();

    // Enabling sending makes no sense when we can't receive...
    fd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    if (fd == INVALID_SOCKET)
        throw cRuntimeError("cSocketRTScheduler: Root privileges needed");
    const int32 on = 1;
    if (setsockopt(fd, IPPROTO_IP, IP_HDRINCL, (char *)&on, sizeof(on)) < 0)
        throw cRuntimeError("cSocketRTScheduler: couldn't set sockopt for raw socket");
}

void cSocketRTScheduler::endRun()
{
    close(fd);
    fd = INVALID_SOCKET;

    for (uint16 i = 0; i < conn.size(); i++) {
        pcap_stat ps;
        if (pcap_stats(conn.at(i).pd, &ps) < 0)
            throw cRuntimeError("cSocketRTScheduler::endRun(): Cannot query pcap statistics: %s", pcap_geterr(conn.at(i).pd));
        else
            EV << conn.at(i).module->getFullPath() << ": Received Packets: " << ps.ps_recv << " Dropped Packets: " << ps.ps_drop << ".\n";
        pcap_close(conn.at(i).pd);
    }

    conn.clear();
}

void cSocketRTScheduler::executionResumed()
{
    baseTime = opp_get_monotonic_clock_usecs();
    baseTime = baseTime - sim->getSimTime().inUnit(SIMTIME_US);
}

void cSocketRTScheduler::setInterfaceModule(cModule *mod, const char *dev, const char *filter)
{
    char errbuf[PCAP_ERRBUF_SIZE];
    struct bpf_program fcode;
    pcap_t *pd;
    int32 datalink;
    int32 headerLength;

    if (!mod || !dev || !filter)
        throw cRuntimeError("cSocketRTScheduler::setInterfaceModule(): arguments must be non-nullptr");

    /* get pcap handle */
    memset(&errbuf, 0, sizeof(errbuf));
    if ((pd = pcap_create(dev, errbuf)) == nullptr)
        throw cRuntimeError("cSocketRTScheduler::setInterfaceModule(): Cannot create pcap device, error = %s", errbuf);
    else if (strlen(errbuf) > 0)
        EV << "cSocketRTScheduler::setInterfaceModule(): pcap_open_live returned warning: " << errbuf << "\n";

    /* apply the immediate mode to pcap */
    if (pcap_set_immediate_mode(pd, 1) != 0)
            throw cRuntimeError("cSocketRTScheduler::setInterfaceModule(): Cannot set immediate mode to pcap device");

    if (pcap_activate(pd) != 0)
        throw cRuntimeError("cSocketRTScheduler::setInterfaceModule(): Cannot activate pcap device");

    /* compile this command into a filter program */
    if (pcap_compile(pd, &fcode, (char *)filter, 0, 0) < 0)
        throw cRuntimeError("cSocketRTScheduler::setInterfaceModule(): Cannot compile pcap filter: %s", pcap_geterr(pd));

    /* apply the compiled filter to the packet capture device */
    if (pcap_setfilter(pd, &fcode) < 0)
        throw cRuntimeError("cSocketRTScheduler::setInterfaceModule(): Cannot apply compiled pcap filter: %s", pcap_geterr(pd));

    if ((datalink = pcap_datalink(pd)) < 0)
        throw cRuntimeError("cSocketRTScheduler::setInterfaceModule(): Cannot query pcap link-layer header type: %s", pcap_geterr(pd));

    /* bind to interface
     struct ifreq ifr;

    memset(&ifr, 0, sizeof(ifr));
    snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "eth0");
    if (setsockopt(s, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr, sizeof(ifr)) < 0) {
        ... error handling ...
    }
     */


#ifndef __linux__
    if (pcap_setnonblock(pd, 1, errbuf) < 0)
        throw cRuntimeError("cSocketRTScheduler::setInterfaceModule(): Cannot put pcap device into non-blocking mode, error: %s", errbuf);
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
            throw cRuntimeError("cSocketRTScheduler::setInterfaceModule(): Unsupported datalink: %d", datalink);
    }
    ExtConn newConn;
    newConn.module = mod;
    newConn.pd = pd;
    newConn.datalink = datalink;
    newConn.headerLength = headerLength;
    conn.push_back(newConn);

    EV << "Opened pcap device " << dev << " with filter " << filter << " and datalink " << datalink << ".\n";
}

static void packet_handler(u_char *user, const struct pcap_pkthdr *hdr, const u_char *bytes)
{
    unsigned i;
    int32 headerLength;
    int32 datalink;
    cModule *module;

    i = *(uint16 *)user;
    datalink = cSocketRTScheduler::conn.at(i).datalink;
    headerLength = cSocketRTScheduler::conn.at(i).headerLength;
    module = cSocketRTScheduler::conn.at(i).module;

    //FIXME Why? Could we use the pcap for filtering incoming IPv4 packet?
    //FIXME Why filtering IPv4 only on eth interface? why not filtering on PPP or other interfaces?
    // skip ethernet frames not encapsulating an IP packet.
    // TODO: how about ipv6 and other protocols?
    if (datalink == DLT_EN10MB && hdr->caplen > ETHER_HDR_LEN) {
        uint16_t etherType = (uint16_t)(bytes[ETHER_ADDR_LEN * 2]) << 8 | bytes[ETHER_ADDR_LEN * 2 + 1];
        //TODO get ethertype from snap header when packet has snap header
        if (etherType != ETHERTYPE_IPv4) // ipv4
            return;
    }

    // put the IP packet from wire into Packet
    uint32_t pklen = hdr->caplen - headerLength;
    Packet *notificationMsg = new Packet("rtEvent");
    const auto& bytesChunk = makeShared<BytesChunk>(bytes + headerLength, pklen);
    notificationMsg->insertAtBack(bytesChunk);

    // signalize new incoming packet to the interface via cMessage
    EV << "Captured " << pklen << " bytes for an IP packet.\n";
    int64_t curTime = opp_get_monotonic_clock_usecs();
    simtime_t t(curTime - cSocketRTScheduler::baseTime, SIMTIME_US);
    // TBD assert that it's somehow not smaller than previous event's time
    notificationMsg->setArrival(module->getId(), -1, t);
    FES(getSimulation())->insert(notificationMsg);
}

bool cSocketRTScheduler::receiveWithTimeout(long usec)
{
    bool found;
    timeval timeout;
    int32 n;
#ifdef __linux__
    int32 fdVec[FD_SETSIZE], maxfd;
    fd_set rdfds;
#endif

    found = false;
    timeout.tv_sec = 0;
    timeout.tv_usec = usec;
#ifdef __linux__
    FD_ZERO(&rdfds);
    maxfd = -1;
    for (uint16 i = 0; i < conn.size(); i++) {
        fdVec[i] = pcap_get_selectable_fd(conn.at(i).pd);
        if (fdVec[i] > maxfd)
            maxfd = fdVec[i];
        FD_SET(fdVec[i], &rdfds);
    }
    if (select(maxfd + 1, &rdfds, nullptr, nullptr, &timeout) < 0) {
        return found;
    }
#endif
    for (uint16 i = 0; i < conn.size(); i++) {
#ifdef __linux__
        if (!(FD_ISSET(fdVec[i], &rdfds)))
            continue;
#endif // ifdef __linux__
        if ((n = pcap_dispatch(conn.at(i).pd, 1, packet_handler, (uint8 *)&i)) < 0)
            throw cRuntimeError("cSocketRTScheduler::pcap_dispatch(): An error occured: %s", pcap_geterr(conn.at(i).pd));
        if (n > 0)
            found = true;
    }
#ifndef __linux__
    if (!found)
        select(0, nullptr, nullptr, nullptr, &timeout);
#endif
    return found;
}

int cSocketRTScheduler::receiveUntil(int64_t targetTime)
{
    // if there's more than 2*UI_REFRESH_TIME to wait, wait in UI_REFRESH_TIME chunks
    // in order to keep UI responsiveness by invoking getEnvir()->idle()
    int64_t curTime = opp_get_monotonic_clock_usecs();

    while ((targetTime - curTime) >= 2000000 || (targetTime - curTime) >= 2*UI_REFRESH_TIME)
    {
        if (receiveWithTimeout(UI_REFRESH_TIME))
            return 1;
        if (getEnvir()->idle())
            return -1;
        curTime = opp_get_monotonic_clock_usecs();
    }

    // difference is now at most UI_REFRESH_TIME, do it at once
    int64_t remaining = targetTime - curTime;
    if (remaining > 0)
        if (receiveWithTimeout(remaining))
            return 1;
    return 0;
}

cEvent *cSocketRTScheduler::guessNextEvent()
{
    return FES(sim)->peekFirst();
}

cEvent *cSocketRTScheduler::takeNextEvent()
{
    int64_t targetTime;

    // calculate target time
    cEvent *event = FES(sim)->peekFirst();
    if (!event) {
        // This way targetTime will always be "as far in the future as possible", considering
        // how integer overflows work in conjunction with comparisons in C++ (in practice...)
        targetTime = baseTime + INT64_MAX;
    }
    else {
        // use time of next event
        simtime_t eventSimtime = event->getArrivalTime();
        targetTime = baseTime + eventSimtime.inUnit(SIMTIME_US);
    }

    // if needed, wait until that time arrives
    int64_t curTime = opp_get_monotonic_clock_usecs();

    if (targetTime > curTime)
    {
        int status = receiveUntil(targetTime);
        if (status == -1)
            return nullptr; // interrupted by user
        if (status == 1)
            event = FES(sim)->peekFirst(); // received something
    }
    else {
        // we're behind -- customized versions of this class may
        // alert if we're too much behind, whatever that means
        int64_t diffTime = curTime - targetTime;
        EV << "We are behind: " << diffTime * 1e-6 << " seconds\n";
    }
    cEvent *tmp = FES(sim)->removeFirst();
    ASSERT(tmp == event);
    return event;
}

void cSocketRTScheduler::putBackEvent(cEvent *event)
{
    FES(sim)->putBackFirst(event);
}

void cSocketRTScheduler::sendBytes(uint8 *buf, size_t numBytes, struct sockaddr *to, socklen_t addrlen)
{
    if (fd == INVALID_SOCKET)
        throw cRuntimeError("cSocketRTScheduler::sendBytes(): no raw socket.");

    int sent = sendto(fd, (char *)buf, numBytes, 0, to, addrlen);    //note: no ssize_t on MSVC

    if ((size_t)sent == numBytes)
        EV << "Sent an IP packet with length of " << sent << " bytes.\n";
    else
        EV << "Sending of an IP packet FAILED! (sendto returned " << sent << " (" << strerror(errno) << ") instead of " << numBytes << ").\n";
}

} // namespace inet


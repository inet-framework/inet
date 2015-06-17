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

#include "inet/linklayer/ext/cSocketRTScheduler.h"

#include "inet/linklayer/common/Ieee802Ctrl_m.h"
#include "inet/common/serializer/headers/ethernethdr.h"

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

#if OMNETPP_BUILDNUM <= 1003
#define FES(sim) (&sim->msgQueue)
#else
#define FES(sim) (sim->getFES())
#endif

std::vector<cModule *> cSocketRTScheduler::modules;
std::vector<pcap_t *> cSocketRTScheduler::pds;
std::vector<int32> cSocketRTScheduler::datalinks;
std::vector<int32> cSocketRTScheduler::headerLengths;

timeval cSocketRTScheduler::baseTime;

Register_Class(cSocketRTScheduler);

inline std::ostream& operator<<(std::ostream& out, const timeval& tv)
{
    return out << (uint32)tv.tv_sec << "s" << tv.tv_usec << "us";
}


cSocketRTScheduler::cSocketRTScheduler() : cScheduler()
{
    fd = INVALID_SOCKET;
}

cSocketRTScheduler::~cSocketRTScheduler()
{
}

void cSocketRTScheduler::startRun()
{
    gettimeofday(&baseTime, nullptr);

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

    for (uint16 i = 0; i < pds.size(); i++) {
        pcap_stat ps;
        if (pcap_stats(pds.at(i), &ps) < 0)
            throw cRuntimeError("cSocketRTScheduler::endRun(): Cannot query pcap statistics: %s", pcap_geterr(pds.at(i)));
        else
            EV << modules.at(i)->getFullPath() << ": Received Packets: " << ps.ps_recv << " Dropped Packets: " << ps.ps_drop << ".\n";
        pcap_close(pds.at(i));
    }

    pds.clear();
    modules.clear();
    pds.clear();
    datalinks.clear();
    headerLengths.clear();
}

void cSocketRTScheduler::executionResumed()
{
    gettimeofday(&baseTime, nullptr);
    baseTime = timeval_substract(baseTime, sim->getSimTime().dbl());
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
    modules.push_back(mod);
    pds.push_back(pd);
    datalinks.push_back(datalink);
    headerLengths.push_back(headerLength);

    EV << "Opened pcap device " << dev << " with filter " << filter << " and datalink " << datalink << ".\n";
}

static void packet_handler(u_char *user, const struct pcap_pkthdr *hdr, const u_char *bytes)
{
    unsigned i;
    int32 headerLength;
    int32 datalink;
    cModule *module;
    struct serializer::ether_header *ethernet_hdr;

    i = *(uint16 *)user;
    datalink = cSocketRTScheduler::datalinks.at(i);
    headerLength = cSocketRTScheduler::headerLengths.at(i);
    module = cSocketRTScheduler::modules.at(i);

    // skip ethernet frames not encapsulating an IP packet.
    if (datalink == DLT_EN10MB) {
        ethernet_hdr = (struct serializer::ether_header *)bytes;
        if (ntohs(ethernet_hdr->ether_type) != ETHERTYPE_IPv4)
            return;
    }

    // put the IP packet from wire into data[] array of ExtFrame
    ExtFrame *notificationMsg = new ExtFrame("rtEvent");
    notificationMsg->setDataArraySize(hdr->caplen - headerLength);
    for (uint16 j = 0; j < hdr->caplen - headerLength; j++)
        notificationMsg->setData(j, bytes[j + headerLength]);

    // signalize new incoming packet to the interface via cMessage
    EV << "Captured " << hdr->caplen - headerLength << " bytes for an IP packet.\n";
    timeval curTime;
    gettimeofday(&curTime, nullptr);
    curTime = timeval_substract(curTime, cSocketRTScheduler::baseTime);
    simtime_t t = curTime.tv_sec + curTime.tv_usec * 1e-6;
    // TBD assert that it's somehow not smaller than previous event's time
#if OMNETPP_BUILDNUM <= 1003
    notificationMsg->setArrival(module, -1, t);
#else
    notificationMsg->setArrival(module->getId(), -1, t);
#endif
    FES(getSimulation())->insert(notificationMsg);
}

bool cSocketRTScheduler::receiveWithTimeout(long usec)
{
    bool found;
    struct timeval timeout;
    int32 n;
#ifdef __linux__
    int32 fd[FD_SETSIZE], maxfd;
    fd_set rdfds;
#endif

    found = false;
    timeout.tv_sec = 0;
    timeout.tv_usec = usec;
#ifdef __linux__
    FD_ZERO(&rdfds);
    maxfd = -1;
    for (uint16 i = 0; i < pds.size(); i++) {
        fd[i] = pcap_get_selectable_fd(pds.at(i));
        if (fd[i] > maxfd)
            maxfd = fd[i];
        FD_SET(fd[i], &rdfds);
    }
    if (select(maxfd + 1, &rdfds, nullptr, nullptr, &timeout) < 0) {
        return found;
    }
#endif
    for (uint16 i = 0; i < pds.size(); i++) {
#ifdef __linux__
        if (!(FD_ISSET(fd[i], &rdfds)))
            continue;
#endif // ifdef __linux__
        if ((n = pcap_dispatch(pds.at(i), 1, packet_handler, (uint8 *)&i)) < 0)
            throw cRuntimeError("cSocketRTScheduler::pcap_dispatch(): An error occured: %s", pcap_geterr(pds.at(i)));
        if (n > 0)
            found = true;
    }
#ifndef __linux__
    if (!found)
        select(0, nullptr, nullptr, nullptr, &timeout);
#endif
    return found;
}

int cSocketRTScheduler::receiveUntil(const timeval& targetTime)
{
    // if there's more than 2*UI_REFRESH_TIME to wait, wait in UI_REFRESH_TIME chunks
    // in order to keep UI responsiveness by invoking getEnvir()->idle()
    timeval curTime;
    gettimeofday(&curTime, nullptr);
    while (targetTime.tv_sec-curTime.tv_sec >=2 ||
           timeval_diff_usec(targetTime, curTime) >= 2*UI_REFRESH_TIME)
    {
        if (receiveWithTimeout(UI_REFRESH_TIME))
            return 1;
        if (getEnvir()->idle())
            return -1;
        gettimeofday(&curTime, nullptr);
    }

    // difference is now at most UI_REFRESH_TIME, do it at once
    long usec = timeval_diff_usec(targetTime, curTime);
    if (usec>0)
        if (receiveWithTimeout(usec))
            return 1;
    return 0;
}

#if OMNETPP_VERSION >= 0x0500
cEvent *cSocketRTScheduler::guessNextEvent()
{
    return FES(sim)->peekFirst();
}

cEvent *cSocketRTScheduler::takeNextEvent()
#else // if OMNETPP_VERSION >= 0x0500
cMessage * cSocketRTScheduler::getNextEvent()
#define cEvent    cMessage
#endif // if OMNETPP_VERSION >= 0x0500
{
    timeval targetTime;

    // calculate target time
    cEvent *event = FES(sim)->peekFirst();
    if (!event) {
        targetTime.tv_sec = LONG_MAX;
        targetTime.tv_usec = 0;
    }
    else {
        // use time of next event
        simtime_t eventSimtime = event->getArrivalTime();
        targetTime = timeval_add(baseTime, eventSimtime.dbl());
    }

    // if needed, wait until that time arrives
    timeval curTime;
    gettimeofday(&curTime, NULL);
    if (timeval_greater(targetTime, curTime))
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
        timeval diffTime = timeval_substract(curTime, targetTime);
        EV << "We are behind: " << diffTime.tv_sec + diffTime.tv_usec * 1e-6 << " seconds\n";
    }
#if OMNETPP_VERSION >= 0x0500
    cEvent *tmp = FES(sim)->removeFirst();
    ASSERT(tmp == event);
#endif // if OMNETPP_VERSION >= 0x0500
    return event;
}
#undef cEvent

#if OMNETPP_VERSION >= 0x0500
void cSocketRTScheduler::putBackEvent(cEvent *event)
{
    FES(sim)->putBackFirst(event);
}

#endif // if OMNETPP_VERSION >= 0x0500

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


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

#include "cSocketRTScheduler.h"

#ifndef IPPROTO_SCTP
#define IPPROTO_SCTP 132
#endif

#include <headers/ethernet.h>

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32) || defined(__CYGWIN__) || defined(_WIN64)
#include <ws2tcpip.h>
#endif

#define PCAP_SNAPLEN 65536 /* capture all data packets with up to pcap_snaplen bytes */
#define PCAP_TIMEOUT 10    /* Timeout in ms */

#ifdef HAVE_PCAP
std::vector<cModule *>cSocketRTScheduler::modules;
std::vector<pcap_t *>cSocketRTScheduler::pds;
std::vector<int32>cSocketRTScheduler::datalinks;
std::vector<int32>cSocketRTScheduler::headerLengths;
#endif
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
#ifdef HAVE_PCAP
    const int32 on = 1;

#endif
    gettimeofday(&baseTime, NULL);
#ifdef HAVE_PCAP
    // Enabling sending makes no sense when we can't receive...
    fd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    if (fd == INVALID_SOCKET)
        throw cRuntimeError("cSocketRTScheduler: Root priviledges needed");
    if (setsockopt(fd, IPPROTO_IP, IP_HDRINCL, (char *)&on, sizeof(on)) < 0)
        throw cRuntimeError("cSocketRTScheduler: couldn't set sockopt for raw socket");
#endif
}


void cSocketRTScheduler::endRun()
{
#ifdef HAVE_PCAP
    pcap_stat ps;

#endif
    close(fd);
    fd = INVALID_SOCKET;
#ifdef HAVE_PCAP

    for (uint16 i=0; i<pds.size(); i++)
    {
        if (pcap_stats(pds.at(i), &ps) < 0)
            throw cRuntimeError("cSocketRTScheduler::endRun(): Can not get pcap statistics: %s", pcap_geterr(pds.at(i)));
        else
            EV << modules.at(i)->getFullPath() << ": Received Packets: " << ps.ps_recv << " Dropped Packets: " << ps.ps_drop << ".\n";
        pcap_close(pds.at(i));
    }

    pds.clear();
    modules.clear();
    pds.clear();
    datalinks.clear();
    headerLengths.clear();
#endif
}

void cSocketRTScheduler::executionResumed()
{
    gettimeofday(&baseTime, NULL);
    baseTime = timeval_substract(baseTime, sim->getSimTime().dbl());
}

void cSocketRTScheduler::setInterfaceModule(cModule *mod, const char *dev, const char *filter)
{
#ifdef HAVE_PCAP
    char errbuf[PCAP_ERRBUF_SIZE];
    struct bpf_program fcode;
    pcap_t * pd;
    int32 datalink;
    int32 headerLength;

    if (!mod || !dev || !filter)
        throw cRuntimeError("cSocketRTScheduler::setInterfaceModule(): arguments must be non-NULL");

    /* get pcap handle */
    memset(&errbuf, 0, sizeof(errbuf));
    if ((pd = pcap_open_live(dev, PCAP_SNAPLEN, 0, PCAP_TIMEOUT, errbuf)) == NULL)
        throw cRuntimeError("cSocketRTScheduler::setInterfaceModule(): Can not open pcap device, error = %s", errbuf);
    else if(strlen(errbuf) > 0)
        EV << "cSocketRTScheduler::setInterfaceModule: pcap_open_live returned waring: " << errbuf << "\n";

    /* compile this command into a filter program */
    if (pcap_compile(pd, &fcode, (char *)filter, 0, 0) < 0)
        throw cRuntimeError("cSocketRTScheduler::setInterfaceModule(): Can not compile filter: %s", pcap_geterr(pd));

    /* apply the compiled filter to the packet capture device */
    if (pcap_setfilter(pd, &fcode) < 0)
        throw cRuntimeError("cSocketRTScheduler::setInterfaceModule(): Can not apply compiled filter: %s", pcap_geterr(pd));

    if ((datalink = pcap_datalink(pd)) < 0)
        throw cRuntimeError("cSocketRTScheduler::setInterfaceModule(): Can not get datalink: %s", pcap_geterr(pd));

#ifndef LINUX
    if (pcap_setnonblock(pd, 1, errbuf) < 0)
        throw cRuntimeError("cSocketRTScheduler::pcap_setnonblock(): Can not put pcap device into non-blocking mode, error = %s", errbuf);
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
#else
    throw cRuntimeError("cSocketRTScheduler::setInterfaceModule(): pcap devices not supported");
#endif
}

#ifdef HAVE_PCAP
static void packet_handler(u_char *user, const struct pcap_pkthdr *hdr, const u_char *bytes)
{
    unsigned i;
    int32 headerLength;
    int32 datalink;
    cModule *module;
    struct ether_header *ethernet_hdr;

    i = *(uint16 *)user;
    datalink = cSocketRTScheduler::datalinks.at(i);
    headerLength = cSocketRTScheduler::headerLengths.at(i);
    module = cSocketRTScheduler::modules.at(i);

    // skip ethernet frames not encapsulating an IP packet.
    if (datalink == DLT_EN10MB)
    {
        ethernet_hdr = (struct ether_header *)bytes;
        if (ntohs(ethernet_hdr->ether_type) != ETHERTYPE_IP)
            return;
    }

    // put the IP packet from wire into data[] array of ExtFrame
    ExtFrame *notificationMsg = new ExtFrame("rtEvent");
    notificationMsg->setDataArraySize(hdr->caplen - headerLength);
    for (uint16 j=0; j< hdr->caplen - headerLength; j++)
        notificationMsg->setData(j, bytes[j + headerLength]);

    // signalize new incoming packet to the interface via cMessage
    EV << "Captured " << hdr->caplen - headerLength << " bytes for an IP packet.\n";
    timeval curTime;
    gettimeofday(&curTime, NULL);
    curTime = timeval_substract(curTime, cSocketRTScheduler::baseTime);
    simtime_t t = curTime.tv_sec + curTime.tv_usec*1e-6;
    // TBD assert that it's somehow not smaller than previous event's time
    notificationMsg->setArrival(module, -1, t);

    simulation.msgQueue.insert(notificationMsg);
}
#endif

bool cSocketRTScheduler::receiveWithTimeout()
{
    bool found;
    struct timeval timeout;
#ifdef HAVE_PCAP
    int32 n;
#ifdef LINUX
    int32 fd[FD_SETSIZE], maxfd;
    fd_set rdfds;
#endif
#endif

    found = false;
    timeout.tv_sec  = 0;
    timeout.tv_usec = PCAP_TIMEOUT * 1000;
#ifdef HAVE_PCAP
#ifdef LINUX
    FD_ZERO(&rdfds);
    maxfd = -1;
    for (uint16 i = 0; i < pds.size(); i++)
    {
        fd[i] = pcap_get_selectable_fd(pds.at(i));
        if (fd[i] > maxfd)
            maxfd = fd[i];
        FD_SET(fd[i], &rdfds);
    }
    if (select(maxfd + 1, &rdfds, NULL, NULL, &timeout) < 0)
    {
        return found;
    }
#endif
    for (uint16 i = 0; i < pds.size(); i++)
    {
#ifdef LINUX
        if (!(FD_ISSET(fd[i], &rdfds)))
            continue;
#endif
        if ((n = pcap_dispatch(pds.at(i), 1, packet_handler, (uint8 *)&i)) < 0)
            throw cRuntimeError("cSocketRTScheduler::pcap_dispatch(): An error occired: %s", pcap_geterr(pds.at(i)));
        if (n > 0)
            found = true;
    }
#ifndef LINUX
    if (!found)
        select(0, NULL, NULL, NULL, &timeout);
#endif
#else
    select(0, NULL, NULL, NULL, &timeout);
#endif
    return found;
}

int32 cSocketRTScheduler::receiveUntil(const timeval& targetTime)
{
    // wait until targetTime or a bit longer, wait in PCAP_TIMEOUT chunks
    // in order to keep UI responsiveness by invoking ev.idle()
    timeval curTime;
    gettimeofday(&curTime, NULL);
    while (timeval_greater(targetTime, curTime))
    {
        if (receiveWithTimeout())
            return 1;
        if (ev.idle())
            return -1;
        gettimeofday(&curTime, NULL);
    }
    return 0;
}

cMessage *cSocketRTScheduler::getNextEvent()
{
    timeval targetTime, curTime, diffTime;

    // calculate target time
    cMessage *msg = sim->msgQueue.peekFirst();
    if (!msg)
    {
        targetTime.tv_sec = LONG_MAX;
        targetTime.tv_usec = 0;
    }
    else
    {
        simtime_t eventSimtime = msg->getArrivalTime();
        targetTime = timeval_add(baseTime, eventSimtime.dbl());
    }

    gettimeofday(&curTime, NULL);
    if (timeval_greater(targetTime, curTime))
    {
        int32 status = receiveUntil(targetTime);
        if (status == -1)
            return NULL; // interrupted by user
        if (status == 1)
            msg = sim->msgQueue.peekFirst(); // received something
    }
    else
    {
        // we're behind -- customized versions of this class may
        // alert if we're too much behind, whatever that means
        diffTime = timeval_substract(curTime, targetTime);
        EV << "We are behind: " << diffTime.tv_sec + diffTime.tv_usec * 1e-6 << " seconds\n";
    }
    return msg;
}

void cSocketRTScheduler::sendBytes(uint8 *buf, size_t numBytes, struct sockaddr *to, socklen_t addrlen)
{
    if (fd == INVALID_SOCKET)
        throw cRuntimeError("cSocketRTScheduler::sendBytes(): no raw socket.");

    int sent = sendto(fd, (char *)buf, numBytes, 0, to, addrlen);  //note: no ssize_t on MSVC

    if (sent == numBytes)
        EV << "Sent an IP packet with length of " << sent << " bytes.\n";
    else
        EV << "Sending of an IP packet FAILED! (sendto returned " << sent << " (" << strerror(errno) << ") instead of " << numBytes << ").\n";
}

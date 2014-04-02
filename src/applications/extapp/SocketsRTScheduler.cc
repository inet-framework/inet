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

// This file is based on the SocketsRTScheduler.cc of OMNeT++ written by
// Andras Varga.

#include "SocketsRTScheduler.h"

#include <headers/ethernet.h>

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32) || defined(__CYGWIN__) || defined(_WIN64)
#include <ws2tcpip.h>
#endif

std::vector<SocketsRTScheduler::Socket>SocketsRTScheduler::sockets;
timeval SocketsRTScheduler::baseTime;

Register_Class(SocketsRTScheduler);

inline std::ostream& operator<<(std::ostream& out, const timeval& tv)
{
    return out << (uint32)tv.tv_sec << "s" << tv.tv_usec << "us";
}

SocketsRTScheduler::SocketsRTScheduler() : cScheduler()
{
}

SocketsRTScheduler::~SocketsRTScheduler()
{
}

void SocketsRTScheduler::startRun()
{
    gettimeofday(&baseTime, NULL);
}


void SocketsRTScheduler::endRun()
{
    for (uint16 i = 0; i < sockets.size(); i++)
    {
    }

    sockets.clear();
}

void SocketsRTScheduler::executionResumed()
{
    gettimeofday(&baseTime, NULL);
    baseTime = timeval_substract(baseTime, sim->getSimTime().dbl());
}

static void packet_handler(int socket, const struct pcap_pkthdr *hdr, const u_char *bytes)
{
    unsigned i;
    int32 headerLength;
    int32 datalink;
    struct ether_header *ethernet_hdr;

    i = *(uint16 *)user;
    cModule *module = SocketsRTScheduler::sockets[i].module;

    // put the IP packet from wire into data[] array of ExtFrame
    ExtFrame *notificationMsg = new ExtFrame("rtEvent");
    notificationMsg->setDataArraySize(hdr->caplen - headerLength);
    for (uint16 j=0; j < hdr->caplen - headerLength; j++)
        notificationMsg->setData(j, bytes[j + headerLength]);

    // signalize new incoming packet to the interface via cMessage
    EV << "Captured " << hdr->caplen - headerLength << " bytes for an IP packet.\n";
    timeval curTime;
    gettimeofday(&curTime, NULL);
    curTime = timeval_substract(curTime, SocketsRTScheduler::baseTime);
    simtime_t t = curTime.tv_sec + curTime.tv_usec*1e-6;
    // TBD assert that it's somehow not smaller than previous event's time
    notificationMsg->setArrival(module, -1, t);

    simulation.msgQueue.insert(notificationMsg);
}

bool SocketsRTScheduler::receiveWithTimeout()
{
    bool found;
    struct timeval timeout;
    int32 n;
#ifdef LINUX
    int32 fd[FD_SETSIZE], maxfd;
    fd_set rdfds;
#endif

    found = false;
    timeout.tv_sec = 0;
    timeout.tv_usec = PCAP_TIMEOUT * 1000;
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
            throw cRuntimeError("SocketsRTScheduler::pcap_dispatch(): An error occured: %s", pcap_geterr(pds.at(i)));
        if (n > 0)
            found = true;
    }
#ifndef LINUX
    if (!found)
        select(0, NULL, NULL, NULL, &timeout);
#endif
    return found;
}

int32 SocketsRTScheduler::receiveUntil(const timeval& targetTime)
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

#if OMNETPP_VERSION >= 0x0500
cEvent *SocketsRTScheduler::guessNextEvent()
{
    return sim->msgQueue.peekFirst();
}
cEvent *SocketsRTScheduler::takeNextEvent()
#else
cMessage *SocketsRTScheduler::getNextEvent()
#define cEvent cMessage
#endif
{
    timeval targetTime, curTime, diffTime;

    // calculate target time
    cEvent *event = sim->msgQueue.peekFirst();
    if (!event)
    {
        targetTime.tv_sec = LONG_MAX;
        targetTime.tv_usec = 0;
    }
    else
    {
        simtime_t eventSimtime = event->getArrivalTime();
        targetTime = timeval_add(baseTime, eventSimtime.dbl());
    }

    gettimeofday(&curTime, NULL);
    if (timeval_greater(targetTime, curTime))
    {
        int32 status = receiveUntil(targetTime);
        if (status == -1)
            return NULL; // interrupted by user
        if (status == 1)
            event = sim->msgQueue.peekFirst(); // received something
    }
    else
    {
        // we're behind -- customized versions of this class may
        // alert if we're too much behind, whatever that means
        diffTime = timeval_substract(curTime, targetTime);
        EV << "We are behind: " << diffTime.tv_sec + diffTime.tv_usec * 1e-6 << " seconds\n";
    }
    cEvent *tmp = sim->msgQueue.removeFirst();
    ASSERT(tmp == event);
    return event;
}
#undef cEvent

#if OMNETPP_VERSION >= 0x0500
void SocketsRTScheduler::putBackEvent(cEvent *event)
{
    sim->msgQueue.putBackFirst(event);
}
#endif

void SocketsRTScheduler::sendBytes(uint8 *buf, size_t numBytes, struct sockaddr *to, socklen_t addrlen)
{
    if (fd == INVALID_SOCKET)
        throw cRuntimeError("SocketsRTScheduler::sendBytes(): no raw socket.");

    int sent = sendto(fd, (char *)buf, numBytes, 0, to, addrlen);  //note: no ssize_t on MSVC

    if ((size_t)sent == numBytes)
        EV << "Sent an IP packet with length of " << sent << " bytes.\n";
    else
        EV << "Sending of an IP packet FAILED! (sendto returned " << sent << " (" << strerror(errno) << ") instead of " << numBytes << ").\n";
}

void SocketsRTScheduler::addSocket(ISocketRT *mod, int fd, bool isListener)
{
    //TODO search fd: error when already exists
    Socket s(mod, fd, isListener);
    sockets.push_back(s);

}

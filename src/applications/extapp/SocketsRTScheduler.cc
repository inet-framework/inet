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

#include "ByteArrayMessage.h"

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32) || defined(__CYGWIN__) || defined(_WIN64)
#include <ws2tcpip.h>
#endif

SocketsRTScheduler::SocketVector SocketsRTScheduler::sockets;
timeval SocketsRTScheduler::baseTime;

Register_Class(SocketsRTScheduler);

inline std::ostream& operator<<(std::ostream& out, const timeval& tv)
{
    return out << (uint32)tv.tv_sec << "s" << tv.tv_usec << "us";
}

std::ostream& operator<<(std::ostream& out, const SocketsRTScheduler::Socket& socket)
{
    return out << "{" << socket.module->getFullPath() << " " << socket.gate->getName() << " " << socket.fd << " " << socket.isListener << "}";
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

bool SocketsRTScheduler::receiveWithTimeout()
{
    bool found;
    struct timeval timeout;
    int32 n;
#if defined(linux) || defined(__linux)
    int32 fd[FD_SETSIZE], maxfd;
    fd_set rdfds;
#endif

    found = false;
    timeout.tv_sec = 0;
    timeout.tv_usec = TIMEOUT;
#if  defined(linux) || defined(__linux)
    FD_ZERO(&rdfds);
    maxfd = -1;
    for (uint16_t i = 0; i < sockets.size(); i++)
    {
        ASSERT(i < FD_SETSIZE);
        if (sockets[i].fd != INVALID_SOCKET)
        {
            fd[i] = sockets[i].fd;
            if (fd[i] > maxfd)
                maxfd = fd[i];
            FD_SET(fd[i], &rdfds);
        }
    }
    if (maxfd == -1)
        return false;
    if (select(maxfd + 1, &rdfds, NULL, NULL, &timeout) < 0)
        return false;
#endif

    for (uint16_t i = 0; i < sockets.size(); i++)
    {
#if defined(linux) || defined(__linux)
        if (!(FD_ISSET(fd[i], &rdfds)))
            continue;
#endif
        if (sockets[i].isListener)
        {
            // accept connection, and store FD in sockets
            sockaddr_in sinRemote;
            int addrSize = sizeof(sinRemote);
            int newFd = accept(sockets[i].fd, (sockaddr*)&sinRemote, (socklen_t*)&addrSize);
            if (newFd == INVALID_SOCKET)
                throw cRuntimeError("SocketsRTScheduler: accept() failed");
            EV << "SocketsRTScheduler: connected!\n";
        }
        else
        {
            int nBytes = recv(sockets[i].fd, buffer, BUFFERSIZE, 0);
            if (nBytes > 0)
            {
                packetHandler(i, buffer, nBytes);
            }
            else if (nBytes == 0)
            {
                EV << "Socket " << sockets[i] << " closed by the client\n";
                if (shutdown(sockets[i].fd, SHUT_WR) == SOCKET_ERROR)
                    throw cRuntimeError("SocketsRTScheduler: shutdown() failed");
                closeSocket(i);
            }
            else if (nBytes==SOCKET_ERROR)
            {
                EV << "cSocketRTScheduler: socket error " << sock_errno() << "\n";
                closeSocket(i);
            }
        }
        found = true;
    }
#if !defined(linux) && !defined(__linux)
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
        EV << "We are behind: " << diffTime << "\n";
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

void SocketsRTScheduler::addSocket(ISocketRT *mod, cGate *gate, int fd, bool isListener)
{
    for (SocketVector::iterator i = sockets.begin(); i != sockets.end(); ++i)
        if (i->fd == fd)
            throw cRuntimeError("fd already added");
    sockets.push_back(Socket(mod, gate, fd, isListener));
}

void SocketsRTScheduler::removeSocket(ISocketRT *mod, int fd)
{
    for (SocketVector::iterator i = sockets.begin(); i != sockets.end(); ++i)
    {
        if (i->fd == fd)
        {
            if (i->module != mod)
                throw cRuntimeError("Invalid module");
            sockets.erase(i);
            break;
        }
    }
}

void SocketsRTScheduler::closeSocket(int socketIndex)
{
    cMessage *msg = new cMessage("socketClosed");
    msg->setKind(CLOSED);
    insertMsg(socketIndex, msg);
    closesocket(sockets[socketIndex].fd);
    sockets[socketIndex].fd = INVALID_SOCKET;
}

void SocketsRTScheduler::insertMsg(int socketIndex, cMessage *msg)
{
    timeval curTime;
    gettimeofday(&curTime, NULL);
    curTime = timeval_substract(curTime, SocketsRTScheduler::baseTime);
    simtime_t t = SimTime(curTime.tv_sec, SIMTIME_S) + SimTime(curTime.tv_usec, SIMTIME_US);
    // TBD assert that it's somehow not smaller than previous event's time
    msg->setArrival(sockets[socketIndex].module->getId(), -1, t);
    simulation.msgQueue.insert(msg);
}

void SocketsRTScheduler::packetHandler(int socketIndex, const void *bytes, unsigned int length)
{
    ByteArrayMessage *notificationMsg = new ByteArrayMessage("rtEvent");
    notificationMsg->setDataFromBuffer(bytes, length);
    notificationMsg->setKind(DATA);
    insertMsg(socketIndex, notificationMsg);
}

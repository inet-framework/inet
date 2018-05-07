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

#include "inet/linklayer/ext/EmulationScheduler.h"
#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/common/packet/Packet.h"

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32) || defined(__CYGWIN__) || defined(_WIN64)
#include <ws2tcpip.h>
#endif // if defined(_WIN32) || defined(__WIN32__) || defined(WIN32) || defined(__CYGWIN__) || defined(_WIN64)

#ifdef __linux__
#define UI_REFRESH_TIME 100000 /* refresh time of the UI in us */
#else
#define UI_REFRESH_TIME 500
#endif

namespace inet {

#define FES(sim) (sim->getFES())

std::vector<EmulationScheduler::ExtConn> EmulationScheduler::conn;

int64_t EmulationScheduler::baseTime;

Register_Class(EmulationScheduler);

EmulationScheduler::EmulationScheduler() : cScheduler()
{
}

EmulationScheduler::~EmulationScheduler()
{
}

void EmulationScheduler::startRun()
{
    baseTime = opp_get_monotonic_clock_usecs();
}

void EmulationScheduler::endRun()
{
    conn.clear();
}

void EmulationScheduler::executionResumed()
{
    baseTime = opp_get_monotonic_clock_usecs();
    baseTime = baseTime - sim->getSimTime().inUnit(SIMTIME_US);
}

void EmulationScheduler::setInterfaceModule(cModule *mod, int recv_socket)
{
    if (!mod)
        throw cRuntimeError("EmulationScheduler::setInterfaceModule(): arguments must be non-nullptr");

    ExtConn newConn;
    newConn.module = mod;
    newConn.callback = check_and_cast<EmulationScheduler::ICallback *>(mod);
    newConn.recv_socket = recv_socket;
    conn.push_back(newConn);

    EV << "Opened pcap device.\n";
}

void EmulationScheduler::dropInterfaceModule(cModule *mod)
{
    for (auto it = conn.begin(); it != conn.end(); ) {
        if (mod == it->module)
            it = conn.erase(it);
        else
            ++it;
    }
}

bool EmulationScheduler::receiveWithTimeout(long usec)
{
#ifdef __linux__
    bool found = false;
    timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = usec;

    int32 fdVec[FD_SETSIZE], maxfd;
    fd_set rdfds;
    FD_ZERO(&rdfds);
    maxfd = -1;
    for (uint16 i = 0; i < conn.size(); i++) {
        fdVec[i] = conn.at(i).recv_socket;
        if (fdVec[i] > maxfd)
            maxfd = fdVec[i];
        FD_SET(fdVec[i], &rdfds);
    }
    if (select(maxfd + 1, &rdfds, nullptr, nullptr, &timeout) < 0) {
        return found;
    }
    for (uint16 i = 0; i < conn.size(); i++) {
        if (!(FD_ISSET(fdVec[i], &rdfds)))
            continue;
        if (conn.at(i).callback->dispatch(fdVec[i]))
            found = true;
    }
    return found;
#else
    bool found = false;
    timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = usec;

    for (uint16 i = 0; i < conn.size(); i++) {
        if (conn.at(i).callback->dispatch(fdVec[i]))
            found = true;
    }
    if (!found)
        select(0, nullptr, nullptr, nullptr, &timeout);
    return found;
#endif
}

int EmulationScheduler::receiveUntil(int64_t targetTime)
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

cEvent *EmulationScheduler::guessNextEvent()
{
    return FES(sim)->peekFirst();
}

cEvent *EmulationScheduler::takeNextEvent()
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

void EmulationScheduler::putBackEvent(cEvent *event)
{
    FES(sim)->putBackFirst(event);
}

} // namespace inet


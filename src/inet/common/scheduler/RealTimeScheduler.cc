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
#include "inet/common/scheduler/RealTimeScheduler.h"

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

int64_t RealTimeScheduler::baseTime;

Register_Class(RealTimeScheduler);

RealTimeScheduler::RealTimeScheduler() : cScheduler()
{
}

RealTimeScheduler::~RealTimeScheduler()
{
}

void RealTimeScheduler::addCallback(int fd, ICallback *callback)
{
    if (!callback)
        throw cRuntimeError("RealTimeScheduler::addCallback(): callback must be non-nullptr");
    callbackEntries.push_back(Entry(fd, callback));
}

void RealTimeScheduler::removeCallback(int fd, ICallback *callback)
{
    for (auto it = callbackEntries.begin(); it != callbackEntries.end(); ) {
        if (fd == it->fd && callback == it->callback)
            it = callbackEntries.erase(it);
        else
            ++it;
    }
}

void RealTimeScheduler::startRun()
{
    baseTime = opp_get_monotonic_clock_usecs();
}

void RealTimeScheduler::endRun()
{
    callbackEntries.clear();
}

void RealTimeScheduler::executionResumed()
{
    baseTime = opp_get_monotonic_clock_usecs();
    baseTime = baseTime - sim->getSimTime().inUnit(SIMTIME_US);
}

bool RealTimeScheduler::receiveWithTimeout(long usec)
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
    for (uint16 i = 0; i < callbackEntries.size(); i++) {
        fdVec[i] = callbackEntries.at(i).fd;
        if (fdVec[i] > maxfd)
            maxfd = fdVec[i];
        FD_SET(fdVec[i], &rdfds);
    }
    if (select(maxfd + 1, &rdfds, nullptr, nullptr, &timeout) < 0) {
        return found;
    }
    for (uint16 i = 0; i < callbackEntries.size(); i++) {
        if (!(FD_ISSET(fdVec[i], &rdfds)))
            continue;
        if (callbackEntries.at(i).callback->notify(fdVec[i]))
            found = true;
    }
    return found;
#else
    bool found = false;
    timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = usec;

    for (uint16 i = 0; i < callbackEntries.size(); i++) {
        if (callbackEntries.at(i).callback->notify(callbackEntries.at(i).fd))
            found = true;
    }
    if (!found)
        select(0, nullptr, nullptr, nullptr, &timeout);
    return found;
#endif
}

int RealTimeScheduler::receiveUntil(int64_t targetTime)
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

cEvent *RealTimeScheduler::guessNextEvent()
{
    return FES(sim)->peekFirst();
}

cEvent *RealTimeScheduler::takeNextEvent()
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

void RealTimeScheduler::putBackEvent(cEvent *event)
{
    FES(sim)->putBackFirst(event);
}

void RealTimeScheduler::scheduleMessage(cModule *module, cMessage *msg)
{
    int64_t curTime = opp_get_monotonic_clock_usecs();
    simtime_t t(curTime - RealTimeScheduler::baseTime, SIMTIME_US);
    // TBD assert that it's somehow not smaller than previous event's time
    msg->setArrival(module->getId(), -1, t);
    getSimulation()->getFES()->insert(msg);
}

} // namespace inet


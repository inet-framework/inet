//
// Copyright (C) 2005-2009 Andras Varga,
//                         Christian Dankbar,
//                         Irene Ruengeler,
//                         Michael Tuexen
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


// This file is based on the cSocketRTScheduler.cc of OMNeT++ written by
// Andras Varga.

#include "inet/common/scheduler/RealTimeScheduler.h"

#include "inet/common/packet/Packet.h"
#include "inet/common/packet/chunk/BytesChunk.h"

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32) || defined(__CYGWIN__) || defined(_WIN64)
#include <ws2tcpip.h>
#endif // if defined(_WIN32) || defined(__WIN32__) || defined(WIN32) || defined(__CYGWIN__) || defined(_WIN64)

#ifdef __linux__
#define UI_REFRESH_TIME    (in64_t)100000000L
#else
#define UI_REFRESH_TIME    (in64_t)500000
#endif

namespace inet {

Register_Class(RealTimeScheduler);

RealTimeScheduler::RealTimeScheduler() : cScheduler()
{
}

RealTimeScheduler::~RealTimeScheduler()
{
}

void RealTimeScheduler::addCallback(int fd, ICallback *callback)
{
    if (fd < 0)
        throw cRuntimeError("RealTimeScheduler::addCallback(): fd is invalid");
    if (!callback)
        throw cRuntimeError("RealTimeScheduler::addCallback(): callback must be non-nullptr");
    callbackEntries.push_back(Entry(fd, callback));
}

void RealTimeScheduler::removeCallback(int fd, ICallback *callback)
{
    for (auto it = callbackEntries.begin(); it != callbackEntries.end();) {
        if (fd == it->fd && callback == it->callback)
            it = callbackEntries.erase(it);
        else
            ++it;
    }
}

void RealTimeScheduler::startRun()
{
    baseTime = opp_get_monotonic_clock_nsecs();
}

void RealTimeScheduler::endRun()
{
    callbackEntries.clear();
}

void RealTimeScheduler::executionResumed()
{
    baseTime = opp_get_monotonic_clock_nsecs();
    baseTime = baseTime - sim->getSimTime().inUnit(SIMTIME_NS);
}

void RealTimeScheduler::advanceSimTime()
{
    int64_t curTime = opp_get_monotonic_clock_nsecs();
    simtime_t t(curTime - baseTime, SIMTIME_NS);
    if (!sim->getFES()->isEmpty())
        t = std::min(t, sim->getFES()->peekFirst()->getArrivalTime());
    sim->setSimTime(t);
}

bool RealTimeScheduler::receiveWithTimeout(int64_t timeout)
{
#ifdef __linux__
    timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = timeout / 1000;

    int32_t maxfd = -1;
    fd_set rdfds;
    FD_ZERO(&rdfds);
    // needs to be copied because notify() calls can modify the callbacks
    std::vector<Entry> callbackEntriesCopy = callbackEntries;
    for (auto& entry : callbackEntriesCopy) {
        if (entry.fd > maxfd)
            maxfd = entry.fd;
        FD_SET(entry.fd, &rdfds);
    }
    if (select(maxfd + 1, &rdfds, nullptr, nullptr, &tv) < 0)
        return false;
    advanceSimTime();
    bool eventInserted = false;
    for (auto& entry : callbackEntriesCopy) {
        if (FD_ISSET(entry.fd, &rdfds)) {
            if (entry.callback->notify(entry.fd))
                eventInserted = true;
        }
    }
    return eventInserted;
#else
    bool found = false;
    timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = timeout / 1000;
    advanceSimTime();
    for (uint16_t i = 0; i < callbackEntries.size(); i++) {
        if (callbackEntries.at(i).callback->notify(callbackEntries.at(i).fd))
            found = true;
    }
    if (!found)
        select(0, nullptr, nullptr, nullptr, &tv);
    return found;
#endif
}

int RealTimeScheduler::receiveUntil(int64_t targetTime)
{
    int64_t currentTime = opp_get_monotonic_clock_nsecs();
    while (targetTime > currentTime) {
        // if there's more than UI_REFRESH_TIME to wait, wait in UI_REFRESH_TIME chunks
        // in order to keep UI responsiveness by invoking getEnvir()->idle()
        int64_t timeout = std::min(UI_REFRESH_TIME, targetTime - currentTime);
        if (receiveWithTimeout(timeout))
            return 1;
        // if we have enough time left we can refresh the UI, because we expect the refresh
        // to take less time than UI_REFRESH_TIME, we waited at most UI_REFRESH_TIME above
        if (targetTime - currentTime > 2 * UI_REFRESH_TIME) {
            if (getEnvir()->idle())
                return -1;
        }
        currentTime = opp_get_monotonic_clock_nsecs();
    }
    return 0;
}

cEvent *RealTimeScheduler::guessNextEvent()
{
    advanceSimTime();
    return sim->getFES()->peekFirst();
}

cEvent *RealTimeScheduler::takeNextEvent()
{
    int64_t targetTime;

    // calculate target time
    cEvent *event = sim->getFES()->peekFirst();
    if (!event)
        // as far into the future as reasonable (hoping we will never overflow - it is unlikely)
        targetTime = INT64_MAX;
    else {
        // use time of next event
        simtime_t eventSimtime = event->getArrivalTime();
        targetTime = baseTime + eventSimtime.inUnit(SIMTIME_NS);
    }

    // if needed, wait until that time arrives
    int64_t curTime = opp_get_monotonic_clock_nsecs();

    if (targetTime > curTime) {
        switch (receiveUntil(targetTime)) {
            case -1: return nullptr; // interrupted by user
            case 0:  break; // nothing to do
            case 1:  event = sim->getFES()->peekFirst(); break; // received something
            default: ASSERT(false); break;
        }
    }
    else {
        // we're behind -- customized versions of this class may
        // alert if we're too much behind, whatever that means
        // NOTE: this is commented out because it generates too much noise by default
//        int64_t diffTime = curTime - targetTime;
//        EV_TRACE << "We are behind: " << diffTime * 1e-9 << " seconds\n";
    }
    cEvent *tmp = sim->getFES()->removeFirst();
    ASSERT(tmp == event);
    return event;
}

void RealTimeScheduler::putBackEvent(cEvent *event)
{
    sim->getFES()->putBackFirst(event);
}

} // namespace inet


// SPDX-License-Identifier: LGPL-3.0-or-later
#include "inet/transportlayer/dtls/DtlsProcessingQueue.h"

#include <algorithm>

namespace inet::dtls {

DtlsProcessingQueue::DtlsProcessingQueue(cSimpleModule *owner, const char *timerName, size_t limit) :
    owner(owner), limit(limit)
{
    if (!owner || !limit)
        throw cRuntimeError("DTLS processing queue requires an owner and a positive byte limit");
    timer = new cMessage(timerName);
}

DtlsProcessingQueue::~DtlsProcessingQueue()
{
    clear();
    delete timer;
}

bool DtlsProcessingQueue::enqueue(AssociationId associationId, size_t bytes, simtime_t delay, Completion complete)
{
    bytes = std::max(size_t(1), bytes);
    if (delay < SIMTIME_ZERO || !complete)
        throw cRuntimeError("Invalid DTLS processing operation");
    if (bytes > limit - occupied)
        return false;
    occupied += bytes;
    pending.push_back({associationId, bytes, delay, std::move(complete)});
    pump();
    return true;
}

void DtlsProcessingQueue::pump()
{
    if (active || pumping)
        return;
    pumping = true;
    try {
        while (!pending.empty() && !active) {
            auto delay = pending.front().delay;
            if (delay > SIMTIME_ZERO) {
                active = true;
                owner->scheduleAt(simTime() + delay, timer);
            }
            else {
                auto operation = std::move(pending.front());
                pending.pop_front();
                occupied -= operation.bytes;
                operation.complete();
            }
        }
    }
    catch (...) {
        pumping = false;
        throw;
    }
    pumping = false;
}

void DtlsProcessingQueue::handleTimer()
{
    if (!active || pending.empty())
        throw cRuntimeError("Unexpected DTLS processing timer");
    auto operation = std::move(pending.front());
    pending.pop_front();
    occupied -= operation.bytes;
    active = false;
    operation.complete();
    pump();
}

void DtlsProcessingQueue::cancel(AssociationId associationId)
{
    if (active && pending.front().associationId == associationId) {
        owner->cancelEvent(timer);
        active = false;
    }
    for (auto it = pending.begin(); it != pending.end(); ) {
        if (it->associationId == associationId) {
            occupied -= it->bytes;
            it = pending.erase(it);
        }
        else
            ++it;
    }
    pump();
}

void DtlsProcessingQueue::clear()
{
    if (active)
        owner->cancelEvent(timer);
    active = false;
    pending.clear();
    occupied = 0;
}

} // namespace inet::dtls

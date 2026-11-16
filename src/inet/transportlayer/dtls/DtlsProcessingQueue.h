// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_DTLSPROCESSINGQUEUE_H
#define INET_DTLSPROCESSINGQUEUE_H

#include <deque>
#include <functional>
#include <set>

#include "inet/common/INETDefs.h"

namespace inet::dtls {

/** A single directional processor. The owning module dispatches its timer. */
class INET_API DtlsProcessingQueue
{
  public:
    using AssociationId = uint64_t;
    using Completion = std::function<void()>;
    using Eligibility = std::function<bool()>;

  protected:
    struct Operation {
        AssociationId associationId;
        size_t bytes;
        simtime_t delay;
        Completion complete;
        Eligibility eligible;
        Completion skipped;
        Completion started;
    };
    cSimpleModule *owner;
    cMessage *timer = nullptr;
    size_t limit;
    size_t occupied = 0;
    std::deque<Operation> pending;
    bool active = false;
    bool pumping = false;

    void pump();

  public:
    DtlsProcessingQueue(cSimpleModule *owner, const char *timerName, size_t limit);
    ~DtlsProcessingQueue();
    DtlsProcessingQueue(const DtlsProcessingQueue&) = delete;
    DtlsProcessingQueue& operator=(const DtlsProcessingQueue&) = delete;

    // Admission reserves at least one byte, including for an empty datagram.
    // An idle zero-delay operation completes before this method returns.
    bool enqueue(AssociationId associationId, size_t bytes, simtime_t delay, Completion complete, Eligibility eligible = {}, Completion skipped = {}, Completion started = {});
    // Continue the just-completed operation before later arrivals, in the same processor.
    bool enqueueContinuation(AssociationId associationId, size_t bytes, simtime_t delay, Completion complete, Eligibility eligible = {}, Completion skipped = {}, Completion started = {});
    bool isTimer(const cMessage *message) const { return message == timer; }
    void handleTimer();
    void cancel(AssociationId associationId);
    void cancelExcept(const std::set<AssociationId>& retainedIds);
    void clear();
    // Capacity reserved by active and pending work, including minimum charges.
    [[nodiscard]] size_t getOccupiedBytes() const { return occupied; }
};

} // namespace inet::dtls
#endif

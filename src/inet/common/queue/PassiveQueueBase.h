//
// Copyright (C) 2005 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_PASSIVEQUEUEBASE_H
#define __INET_PASSIVEQUEUEBASE_H

#include <map>

#include "inet/common/INETDefs.h"

#include "inet/common/queue/IPassiveQueue.h"

namespace inet {

/**
 * Abstract base class for passive queues. Implements IPassiveQueue.
 * Enqueue/dequeue have to be implemented in virtual functions in
 * subclasses; the actual queue or piority queue data structure
 * also goes into subclasses.
 */
class INET_API PassiveQueueBase : public cSimpleModule, public IPassiveQueue
{
  protected:
    std::list<IPassiveQueueListener *> listeners;

    // state
    int packetRequested;

    // statistics
    int numQueueReceived;
    int numQueueDropped;

    /** Signal with packet when received it */
    static simsignal_t rcvdPkSignal;
    /** Signal with packet when enqueued it */
    static simsignal_t enqueuePkSignal;
    /** Signal with packet when sent out it */
    static simsignal_t dequeuePkSignal;
    /** Signal with packet when dropped it */
    static simsignal_t dropPkByQueueSignal;
    /** Signal with value of delaying time when sent out a packet. */
    static simsignal_t queueingTimeSignal;

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;

    virtual void notifyListeners();

    /**
     * Inserts packet into the queue or the priority queue, or drops it
     * (or another packet). Returns nullptr if successful, or the pointer of the dropped packet.
     */
    virtual cMessage *enqueue(cMessage *msg) = 0;

    /**
     * Returns a packet from the queue, or nullptr if the queue is empty.
     */
    virtual cMessage *dequeue() = 0;

    /**
     * Should be redefined to send out the packet; e.g. <tt>send(msg,"out")</tt>.
     */
    virtual void sendOut(cMessage *msg) = 0;

  public:
    /**
     * The queue should send a packet whenever this method is invoked.
     * If the queue is currently empty, it should send a packet when
     * when one becomes available.
     */
    virtual void requestPacket() override;

    /**
     * Returns number of pending requests.
     */
    virtual int getNumPendingRequests() override { return packetRequested; }

    /**
     * Clear all queued packets and stored requests.
     */
    virtual void clear() override;

    /**
     * Return a packet from the queue directly.
     */
    virtual cMessage *pop() override;

    /**
     * Implementation of IPassiveQueue::addListener().
     */
    virtual void addListener(IPassiveQueueListener *listener) override;

    /**
     * Implementation of IPassiveQueue::removeListener().
     */
    virtual void removeListener(IPassiveQueueListener *listener) override;
};

} // namespace inet

#endif // ifndef __INET_PASSIVEQUEUEBASE_H


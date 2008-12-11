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

#include <omnetpp.h>
#include "IPassiveQueue.h"


/**
 * Abstract base class for passive queues. Implements IPassiveQueue.
 * Enqueue/dequeue have to be implemented in virtual functions in
 * subclasses; the actual queue or piority queue data structure
 * also goes into subclasses.
 */
class INET_API PassiveQueueBase : public cSimpleModule, public IPassiveQueue
{
  protected:
    // state
    int packetRequested;

    // statistics
    int numQueueReceived;
    int numQueueDropped;

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();

    /**
     * Inserts packet into the queue or the priority queue, or drops it
     * (or another packet). Returns true if a packet was dropped.
     */
    virtual bool enqueue(cMessage *msg) = 0;

    /**
     * Returns a packet from the queue, or NULL if the queue is empty.
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
    virtual void requestPacket();
};

#endif



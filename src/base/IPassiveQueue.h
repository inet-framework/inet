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


#ifndef __INET_IPASSIVEQUEUE_H
#define __INET_IPASSIVEQUEUE_H

#include "INETDefs.h"

class IPassiveQueueListener;

/**
 * Abstract interface for passive queues.
 */
class INET_API IPassiveQueue
{
  public:
    virtual ~IPassiveQueue() {}

    /**
     * The queue should send a packet whenever this method is invoked.
     * If the queue is currently empty, it should send a packet when
     * when one becomes available.
     */
    virtual void requestPacket() = 0;

    /**
     * Returns number of pending requests.
     */
    virtual int getNumPendingRequests() = 0;

    /**
     * Return true when queue is empty, otherwise return false.
     */
    virtual bool isEmpty() = 0;

    /**
     * Clear all queued packets and stored requests.
     */
    virtual void clear() = 0;

    /**
     * Adds a new listener to the listener list.
     * It does nothing, if the listener list already contains the listener
     * (by pointer equality).
     */
    virtual void addListener(IPassiveQueueListener *listener) = 0;

    /**
     * Removes a listener from the listener list.
     * It does nothing if the listener was not found on the listener list.
     */
    virtual void removeListener(IPassiveQueueListener *listener) = 0;
};

/**
 * Interface for notifying listeners about passive queue events.
 */
class INET_API IPassiveQueueListener
{
    public:

      virtual ~IPassiveQueueListener() {};

      /**
       * A packet arrived and it was added to the queue (the queue length
       * increased by one). Therefore a subsequent requestPacket() call
       * can deliver a packet immediately.
       */
      virtual void packetEnqueued(IPassiveQueue *queue) = 0;
};

#endif


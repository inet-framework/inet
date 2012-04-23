//
// Copyright (C) 2012 Opensim Ltd.
// Author: Tamas Borbely
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

#ifndef SCHEDULERBASE_H_
#define SCHEDULERBASE_H_

#include "INETDefs.h"
#include "IPassiveQueue.h"

/**
 * Base class for packet schedulers.
 *
 * Schedulers are attached to the 'out' end of queues
 * and decide from which queue a packet needs to be
 * dequeued.
 *
 * Schedulers behaves as a passive queue (the provide a view
 * for the actual queues behind them), and can be cascaded.
 * They also must be able to notice if a new packet arrived
 * at one of its input without dequeueing it, so they
 * registers themselves as listener of their inputs.
 */
class INET_API SchedulerBase : public cSimpleModule, public IPassiveQueue, public IPassiveQueueListener
{
    protected:
        // state
        int packetsRequestedFromUs;
        int packetsToBeRequestedFromInputs;
        std::vector<IPassiveQueue*> inputQueues;
        cGate *outGate;
        std::list<IPassiveQueueListener*> listeners;

    public:
        SchedulerBase();
        virtual ~SchedulerBase();

    protected:
      virtual void initialize();
      virtual void finalize();
      virtual void handleMessage(cMessage *msg);
      virtual void sendOut(cMessage *msg);
      virtual void notifyListeners();
      virtual bool schedulePacket() = 0;

    public:
      virtual void requestPacket();
      virtual int getNumPendingRequests() { return packetsRequestedFromUs; }
      virtual bool isEmpty();
      virtual void clear();
      virtual void packetEnqueued(IPassiveQueue *inputQueue);
      virtual void addListener(IPassiveQueueListener *listener);
      virtual void removeListener(IPassiveQueueListener *listener);
};

#endif /* SCHEDULERBASE_H_ */

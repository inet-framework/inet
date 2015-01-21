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

#ifndef __INET_DROPTAILQUEUE_H
#define __INET_DROPTAILQUEUE_H

#include "inet/common/INETDefs.h"

#include "inet/common/queue/PassiveQueueBase.h"

namespace inet {

/**
 * Drop-front queue. See NED for more info.
 */
class INET_API DropTailQueue : public PassiveQueueBase
{
  protected:
    // configuration
    int frameCapacity;

    // state
    cQueue queue;
    cGate *outGate;

    // statistics
    static simsignal_t queueLengthSignal;

  protected:
    virtual void initialize() override;

    /**
     * Redefined from PassiveQueueBase.
     */
    virtual cMessage *enqueue(cMessage *msg) override;

    /**
     * Redefined from PassiveQueueBase.
     */
    virtual cMessage *dequeue() override;

    /**
     * Redefined from PassiveQueueBase.
     */
    virtual void sendOut(cMessage *msg) override;

    /**
     * Redefined from IPassiveQueue.
     */
    virtual bool isEmpty() override;
};

} // namespace inet

#endif // ifndef __INET_DROPTAILQUEUE_H


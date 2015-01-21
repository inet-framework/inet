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

#ifndef __INET_FIFOQUEUE_H
#define __INET_FIFOQUEUE_H

#include "inet/common/INETDefs.h"
#include "inet/common/queue/PassiveQueueBase.h"
#include "inet/common/queue/IQueueAccess.h"

namespace inet {

/**
 * Passive FIFO Queue with unlimited buffer space.
 */
class INET_API FIFOQueue : public PassiveQueueBase, public IQueueAccess
{
  protected:
    // state
    cQueue queue;
    cGate *outGate;
    int byteLength;

    // statistics
    static simsignal_t queueLengthSignal;

  public:
    FIFOQueue() : outGate(nullptr), byteLength(0) {}

  protected:
    virtual void initialize() override;

    virtual cMessage *enqueue(cMessage *msg) override;

    virtual cMessage *dequeue() override;

    virtual void sendOut(cMessage *msg) override;

    virtual bool isEmpty() override;

    virtual int getLength() const override { return queue.getLength(); }

    virtual int getByteLength() const override { return byteLength; }
};

} // namespace inet

#endif // ifndef __INET_FIFOQUEUE_H


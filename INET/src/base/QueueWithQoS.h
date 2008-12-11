//
// Copyright (C) 2004 Andras Varga
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


#ifndef __INET_QUEUEWITHQOS_H
#define __INET_QUEUEWITHQOS_H

#include "AbstractQueue.h"
#include "EnqueueHook.h"


// TBD this whole thing should probably be removed, together with IPv4d

/**
 * Queue with constant processing time, with integrated QoS behaviour.
 * Processing time is taken from the "procDelay" module parameter,
 * and QoS class from the "qosBehaviourClass" module parameter.
 *
 * Leaves the endService(cMessage *msg) method of AbstractQueue undefined.
 */
class INET_API QueueWithQoS : public AbstractQueue
{
  protected:
    simtime_t delay;
    EnqueueHook *qosHook;

  public:
    QueueWithQoS() {}

  protected:
    virtual void initialize();
    virtual void arrival(cPacket *msg);
    virtual cPacket *arrivalWhenIdle(cPacket *msg);
    virtual simtime_t startService(cPacket *msg);
};

#endif


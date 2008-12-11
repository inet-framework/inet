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


#include "QueueWithQoS.h"


void QueueWithQoS::initialize()
{
    AbstractQueue::initialize();

    delay = par("procDelay");
    qosHook = check_and_cast<EnqueueHook *>(createOne(par("qosBehaviorClass")));
    qosHook->setModule(this);
}


void QueueWithQoS::arrival(cPacket *msg)
{
    qosHook->enqueue(msg, queue);
}

cPacket *QueueWithQoS::arrivalWhenIdle(cPacket *msg)
{
    return PK(qosHook->dropIfNotNeeded(msg));
}

simtime_t QueueWithQoS::startService(cPacket *msg)
{
    return delay;
}


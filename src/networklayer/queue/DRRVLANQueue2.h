//
// Copyright (C) 2013 Kyeong Soo (Joseph) Kim
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


#ifndef __INET_DRRVLANQUEUE2_H
#define __INET_DRRVLANQUEUE2_H

#include <algorithm>
#include <list>
#include "DRRVLANQueue.h"

/**
 * Priority queueing (PQ) over FIFO queue and per-flow queues with
 * deficit round-robin (DRR) scheduler.
 * Incoming packets are classified by an external VLAN classifier and
 * metered by an external token bucket meters before being put into
 * a FIFO or per-flow queues.
 * See NED for more info.
 */
class INET_API DRRVLANQueue2 : public DRRVLANQueue
{
protected:
    // type definitions for member variables
    typedef std::list<int> IntList;

  protected:

    // DRR scheduler
    IntVector conformedCounters;    ///< vector of counters for conformed bytes
    IntVector nonconformedCounters; ///< vector of counters for non-conformed bytes
    IntList conformedList;          ///< list of queues with non-zero conformed bytes
    IntList nonconformedList;       ///< list of queues with non-zero non-conformed bytes

    // debugging
#ifndef NDEBUG
    typedef std::vector<cOutVector *> OutVectorVector;
    OutVectorVector conformedCounterVector;
    OutVectorVector nonconformedCounterVector;
#endif

  public:
    DRRVLANQueue2();
    virtual ~DRRVLANQueue2();

  protected:
    virtual void initialize();

    /**
     * Redefined from DRRVLANQueue.
     */
    virtual void handleMessage(cMessage *msg);

    /**
     * Redefined from DRRVLANQueue.
     */
    virtual cMessage *dequeue();
};

#endif

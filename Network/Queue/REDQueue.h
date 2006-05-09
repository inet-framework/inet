//
// Copyright (C) 2005 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


#ifndef __REDQUEUE_H__
#define __REDQUEUE_H__

#include <omnetpp.h>
#include "PassiveQueueBase.h"

/**
 * RED queue. See NED for more info.
 */
class INET_API REDQueue : public PassiveQueueBase
{
  protected:
    // configuration (see NED file and paper for meaning of RED variables)
    double wq;    // queue weight
    double minth; // minimum threshold for avg queue length
    double maxth; // maximum threshold for avg queue length
    double maxp;  // maximum value for pb
    double pkrate; // number of packets expected to arrive per second (used for f())

    // state (see NED file and paper for meaning of RED variables)
    cQueue queue;
    double avg;       // average queue size
    simtime_t q_time; // start of the queue idle time
    int count;        // packets since last marked packet

    // statistics
    cOutVector avgQlenVec;
    cOutVector qlenVec;
    cOutVector dropVec;
    long numEarlyDrops;

  protected:
    virtual void initialize();
    virtual void finish();

    /**
     * Redefined from PassiveQueueBase.
     */
    virtual bool enqueue(cMessage *msg);

    /**
     * Redefined from PassiveQueueBase.
     */
    virtual cMessage *dequeue();

    /**
     * Redefined from PassiveQueueBase.
     */
    virtual void sendOut(cMessage *msg);

};

#endif



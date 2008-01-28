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


#ifndef __IQOSCLASSIFIER_H__
#define __IQOSCLASSIFIER_H__

#include <omnetpp.h>
#include "INETDefs.h"

/**
 * Abstract interface for QoS classifiers, used in QoS queues.
 * A QoS classifier looks at a packet, determines its priority,
 * and eventually returns the index of the subqueue the packet
 * should be inserted into. DropTailQoSQueue is one of the queue
 * modules which expect a C++ class subclassed from IQoSClassifier.
 *
 * @see DropTailQoSQueue
 */
class INET_API IQoSClassifier : public cPolymorphic
{
  public:
    /**
     * Returns the largest value plus one classifyPacket() returns.
     */
    virtual int numQueues() = 0;

    /**
     * The method should return the priority (the index of subqueue)
     * for the given packet, a value between 0 and numQueues()-1
     * (inclusive), with 0 representing the highest priority.
     */
    virtual int classifyPacket(cMessage *msg) = 0;
};

#endif


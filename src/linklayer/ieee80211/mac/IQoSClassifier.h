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


#ifndef __INET_IQOSCLASSIFIER_H
#define __INET_IQOSCLASSIFIER_H

#include "INETDefs.h"

/**
 * Abstract interface for QoS classifiers, used in QoS queues.
 * A QoS classifier looks at a packet, determines its priority,
 * and eventually returns the index of the subqueue the packet
 * should be inserted into.
 */
class INET_API IQoSClassifier : public cObject
{
  public:
    /**
     * Returns the largest value plus one classifyPacket() returns.
     */
    virtual int getNumQueues() = 0;

    /**
     * The method should return the priority (the index of subqueue)
     * for the given packet, a value between 0 and getNumQueues()-1
     * (inclusive), with 0 representing the highest priority.
     */
    virtual int classifyPacket(cMessage *msg) = 0;
};

#endif


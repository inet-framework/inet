//
// Copyright (C) 2011 Zoltan Bojthe
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


#ifndef __INET_ETHERFRAMECLASSIFIER_H
#define __INET_ETHERFRAMECLASSIFIER_H

#include "IQoSClassifier.h"

/**
 * Ethernet Frame classifier:
 * The PAUSE frames have higher priority than other frames
 */
class INET_API EtherFrameClassifier : public IQoSClassifier
{
  public:
    /**
     * Returns the largest value plus one classifyPacket() returns.
     */
    virtual int getNumQueues();

    /**
     * The method should return the priority (the index of subqueue)
     * for the given packet, a value between 0 and getNumQueues()-1
     * (inclusive).
     */
    virtual int classifyPacket(cMessage *msg);
};

#endif


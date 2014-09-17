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

#ifndef __INET_ALGORITHMICDROPPERBASE_H
#define __INET_ALGORITHMICDROPPERBASE_H

#include "inet/common/INETDefs.h"
#include "inet/common/queue/IQueueAccess.h"

namespace inet {

/**
 * Base class for algorithmic droppers (RED, DropTail, etc.).
 */
class INET_API AlgorithmicDropperBase : public cSimpleModule, public IQueueAccess
{
  protected:
    int numGates;
    std::vector<IQueueAccess *> outQueues;    // vector of out queues indexed by gate index (may contain duplicate elements)
    std::set<IQueueAccess *> outQueueSet;    // set of out queues; comparing pointers is ok

  public:
    AlgorithmicDropperBase() : numGates(0) {};
    virtual ~AlgorithmicDropperBase() {};

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual bool shouldDrop(cPacket *packet) = 0;
    virtual void dropPacket(cPacket *packet);
    virtual void sendOut(cPacket *packet);

    virtual int getLength() const;
    virtual int getByteLength() const;
};

} // namespace inet

#endif // ifndef __INET_ALGORITHMICDROPPERBASE_H


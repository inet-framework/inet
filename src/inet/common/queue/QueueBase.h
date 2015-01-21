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

#ifndef __INET_QUEUEBASE_H
#define __INET_QUEUEBASE_H

#include "inet/common/queue/AbstractQueue.h"

namespace inet {

/**
 * Queue with constant processing time.
 * Leaves the endService(cMessage *msg) method of AbstractQueue undefined.
 */
class INET_API QueueBase : public AbstractQueue
{
  protected:
    simtime_t delay;

  public:
    QueueBase() {}

  protected:
    virtual void initialize() override;
    virtual void arrival(cPacket *msg) override;
    virtual cPacket *arrivalWhenIdle(cPacket *msg) override;
    virtual simtime_t startService(cPacket *msg) override;
};

} // namespace inet

#endif // ifndef __INET_QUEUEBASE_H


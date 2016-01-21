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

#ifndef __INET_WRRSCHEDULER_H
#define __INET_WRRSCHEDULER_H

#include "inet/common/INETDefs.h"
#include "inet/common/queue/SchedulerBase.h"

namespace inet {

/**
 * This module implements a Weighted Round Robin Scheduler.
 */
class INET_API WRRScheduler : public SchedulerBase
{
  protected:
    int numInputs;    // number of input gates
    int *weights;    // array of weights (has numInputs elements)
    int *buckets;    // array of tokens in buckets (has numInputs elements)

  public:
    WRRScheduler() : numInputs(0), weights(nullptr), buckets(nullptr) {}

  protected:
    virtual ~WRRScheduler();
    virtual void initialize() override;
    virtual bool schedulePacket() override;
};

} // namespace inet

#endif // ifndef __INET_WRRSCHEDULER_H


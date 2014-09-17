//
// Copyright (C) 2013 OpenSim Ltd.
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

#ifndef __INET_SIMPLEPOWERCONSUMER_H
#define __INET_SIMPLEPOWERCONSUMER_H

#include "inet/power/contract/IPowerConsumer.h"
#include "inet/power/contract/IPowerSource.h"

namespace inet {

namespace power {

/**
 * This class implements a simple power consumer.
 *
 * @author Levente Meszaros
 */
class INET_API SimplePowerConsumer : public cSimpleModule, public IPowerConsumer
{
  protected:
    bool isSleeping;
    int powerConsumerId;
    IPowerSource *powerSource;
    W powerConsumption;
    cMessage *timer;

  protected:
    virtual void initialize(int stage);

    virtual void handleMessage(cMessage *message);

    virtual void updatePowerConsumption();

    virtual void scheduleIntervalTimer();

  public:
    SimplePowerConsumer();
    virtual ~SimplePowerConsumer();

    virtual W getPowerConsumption() { return powerConsumption; }
};

} // namespace power

} // namespace inet

#endif // ifndef __INET_SIMPLEPOWERCONSUMER_H


//
// Copyright (C) OpenSim Ltd.
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

#ifndef __INET_ALTERNATINGEPENERGYCONSUMER_H
#define __INET_ALTERNATINGEPENERGYCONSUMER_H

#include "inet/power/contract/IEpEnergyConsumer.h"
#include "inet/power/contract/IEpEnergySource.h"

namespace inet {

namespace power {

/**
 * This class implements a simple power based alternating energy consumer.
 *
 * See the corresponding NED file for more details.
 *
 * @author Levente Meszaros
 */
class INET_API AlternatingEpEnergyConsumer : public cSimpleModule, public IEpEnergyConsumer
{
  protected:
    // parameters
    IEpEnergySource *energySource = nullptr;
    cMessage *timer = nullptr;

    // state
    bool isSleeping = false;
    W powerConsumption = W(NaN);

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;

    virtual void updatePowerConsumption();
    virtual void scheduleIntervalTimer();

  public:
    virtual ~AlternatingEpEnergyConsumer();

    virtual IEnergySource *getEnergySource() const override { return energySource; }
    virtual W getPowerConsumption() const override { return powerConsumption; }
};

} // namespace power

} // namespace inet

#endif // ifndef __INET_ALTERNATINGEPENERGYCONSUMER_H


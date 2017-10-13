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

#ifndef __INET_ALTERNATINGEPENERGYGENERATOR_H
#define __INET_ALTERNATINGEPENERGYGENERATOR_H

#include "inet/power/contract/IEpEnergyGenerator.h"
#include "inet/power/contract/IEpEnergySink.h"

namespace inet {

namespace power {

/**
 * This class implements a simple power based alternating energy generator.
 *
 * See the corresponding NED file for more details.
 *
 * @author Levente Meszaros
 */
class INET_API AlternatingEpEnergyGenerator : public cSimpleModule, public IEpEnergyGenerator
{
  protected:
    // parameters
    IEpEnergySink *energySink = nullptr;
    cMessage *timer = nullptr;

    // state
    bool isSleeping = false;
    W powerGeneration = W(NaN);

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;

    virtual void updatePowerGeneration();
    virtual void scheduleIntervalTimer();

  public:
    virtual ~AlternatingEpEnergyGenerator();

    virtual IEnergySink *getEnergySink() const override { return energySink; }
    virtual W getPowerGeneration() const override { return powerGeneration; }
};

} // namespace power

} // namespace inet

#endif // ifndef __INET_ALTERNATINGEPENERGYGENERATOR_H


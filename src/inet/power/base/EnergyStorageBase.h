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

#ifndef __INET_ENERGYSTORAGEBASE_H
#define __INET_ENERGYSTORAGEBASE_H

#include "inet/power/contract/IEnergyStorage.h"
#include "inet/power/base/EnergySourceBase.h"
#include "inet/power/base/EnergySinkBase.h"

namespace inet {

namespace power {

/**
 * TODO
 *
 * @author Levente Meszaros
 */
class INET_API EnergyStorageBase : public cSimpleModule, public EnergySourceBase, public EnergySinkBase, public virtual IEnergyStorage
{
  protected:
    virtual void initialize(int stage) override;

  public:
    virtual int addEnergyConsumer(const IEnergyConsumer *energyConsumer) override;
    virtual void removeEnergyConsumer(int energyConsumerId) override;
    virtual void setPowerConsumption(int energyConsumerId, W consumedPower) override;

    virtual int addEnergyGenerator(const IEnergyGenerator *energyGenerator) override;
    virtual void removeEnergyGenerator(int energyGeneratorId) override;
    virtual void setPowerGeneration(int energyGeneratorId, W generatedPower) override;
};

} // namespace power

} // namespace inet

#endif // ifndef __INET_ENERGYSINKBASE_H


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

#ifndef __INET_EPENERGYSTORAGEBASE_H
#define __INET_EPENERGYSTORAGEBASE_H

#include "inet/power/base/EpEnergySinkBase.h"
#include "inet/power/base/EpEnergySourceBase.h"
#include "inet/power/contract/IEpEnergyStorage.h"

namespace inet {

namespace power {

class INET_API EpEnergyStorageBase : public cSimpleModule, public EpEnergySourceBase, public EpEnergySinkBase, public virtual IEpEnergyStorage
{
  protected:
    virtual void initialize(int stage) override;

    virtual void updateTotalPowerConsumption() override;
    virtual void updateTotalPowerGeneration() override;

  public:
    virtual void addEnergyConsumer(const IEnergyConsumer *energyConsumer) override;
    virtual void removeEnergyConsumer(const IEnergyConsumer *energyConsumer) override;

    virtual void addEnergyGenerator(const IEnergyGenerator *energyGenerator) override;
    virtual void removeEnergyGenerator(const IEnergyGenerator *energyGenerator) override;

    virtual void receiveSignal(cComponent *source, simsignal_t signal, double value, cObject *details) override;
};

} // namespace power

} // namespace inet

#endif // ifndef __INET_EPENERGYSTORAGEBASE_H


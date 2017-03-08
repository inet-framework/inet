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

#ifndef __INET_EPENERGYSOURCEBASE_H
#define __INET_EPENERGYSOURCEBASE_H

#include "inet/power/base/EnergySourceBase.h"
#include "inet/power/contract/IEpEnergyConsumer.h"
#include "inet/power/contract/IEpEnergySource.h"

namespace inet {

namespace power {

class INET_API EpEnergySourceBase : public EnergySourceBase, public virtual IEpEnergySource, public cListener
{
  protected:
    W totalPowerConsumption = W(NaN);

  protected:
    virtual W computeTotalPowerConsumption() const;
    virtual void updateTotalPowerConsumption();

  public:
    virtual W getTotalPowerConsumption() const override { return totalPowerConsumption; }

    virtual void addEnergyConsumer(const IEnergyConsumer *energyConsumer) override;
    virtual void removeEnergyConsumer(const IEnergyConsumer *energyConsumer) override;
};

} // namespace power

} // namespace inet

#endif // ifndef __INET_EPENERGYSOURCEBASE_H


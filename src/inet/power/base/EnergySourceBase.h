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

#ifndef __INET_ENERGYSOURCEBASE_H
#define __INET_ENERGYSOURCEBASE_H

#include "inet/power/contract/IEnergySource.h"

namespace inet {

namespace power {

class INET_API EnergySourceBase : public virtual IEnergySource
{
  protected:
    std::vector<const IEnergyConsumer *> energyConsumers;

  public:
    virtual int getNumEnergyConsumers() const override { return energyConsumers.size(); }
    virtual const IEnergyConsumer *getEnergyConsumer(int index) const override;
    virtual void addEnergyConsumer(const IEnergyConsumer *energyConsumer) override;
    virtual void removeEnergyConsumer(const IEnergyConsumer *energyConsumer) override;
};

} // namespace power

} // namespace inet

#endif // ifndef __INET_ENERGYSOURCEBASE_H


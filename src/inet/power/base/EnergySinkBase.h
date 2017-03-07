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

#ifndef __INET_ENERGYSINKBASE_H
#define __INET_ENERGYSINKBASE_H

#include "inet/power/contract/IEnergySink.h"

namespace inet {

namespace power {

class INET_API EnergySinkBase : public virtual IEnergySink
{
  protected:
    std::vector<const IEnergyGenerator *> energyGenerators;

  public:
    virtual int getNumEnergyGenerators() const override { return energyGenerators.size(); }
    virtual const IEnergyGenerator *getEnergyGenerator(int index) const override;
    virtual void addEnergyGenerator(const IEnergyGenerator *energyGenerator) override;
    virtual void removeEnergyGenerator(const IEnergyGenerator *energyGenerator) override;
};

} // namespace power

} // namespace inet

#endif // ifndef __INET_ENERGYSINKBASE_H


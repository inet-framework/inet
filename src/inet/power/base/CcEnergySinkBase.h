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

#ifndef __INET_CCENERGYSINKBASE_H
#define __INET_CCENERGYSINKBASE_H

#include "inet/power/base/EnergySinkBase.h"
#include "inet/power/contract/ICcEnergyGenerator.h"
#include "inet/power/contract/ICcEnergySink.h"

namespace inet {

namespace power {

class INET_API CcEnergySinkBase : public EnergySinkBase, public virtual ICcEnergySink, public cListener
{
  protected:
    A totalCurrentGeneration = A(NaN);

  protected:
    virtual A computeTotalCurrentGeneration() const;
    virtual void updateTotalCurrentGeneration();

  public:
    virtual A getTotalCurrentGeneration() const override { return totalCurrentGeneration; }

    virtual void addEnergyGenerator(const IEnergyGenerator *energyGenerator) override;
    virtual void removeEnergyGenerator(const IEnergyGenerator *energyGenerator) override;
};

} // namespace power

} // namespace inet

#endif // ifndef __INET_CCENERGYSINKBASE_H


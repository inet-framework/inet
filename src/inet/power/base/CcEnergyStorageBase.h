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

#ifndef __INET_CCENERGYSTORAGEBASE_H
#define __INET_CCENERGYSTORAGEBASE_H

#include "inet/power/base/CcEnergySinkBase.h"
#include "inet/power/base/CcEnergySourceBase.h"
#include "inet/power/contract/ICcEnergyStorage.h"

namespace inet {

namespace power {

class INET_API CcEnergyStorageBase : public cSimpleModule, public CcEnergySourceBase, public CcEnergySinkBase, public virtual ICcEnergyStorage
{
  protected:
    virtual void initialize(int stage) override;

    virtual void updateTotalCurrentConsumption() override;
    virtual void updateTotalCurrentGeneration() override;

  public:
    virtual void addEnergyConsumer(const IEnergyConsumer *energyConsumer) override;
    virtual void removeEnergyConsumer(const IEnergyConsumer *energyConsumer) override;

    virtual void addEnergyGenerator(const IEnergyGenerator *energyGenerator) override;
    virtual void removeEnergyGenerator(const IEnergyGenerator *energyGenerator) override;

    virtual void receiveSignal(cComponent *source, simsignal_t signal, double value, cObject *details) override;
};

} // namespace power

} // namespace inet

#endif // ifndef __INET_CCENERGYSTORAGEBASE_H


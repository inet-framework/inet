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

#ifndef __INET_STATEBASEDCCENERGYCONSUMER_H
#define __INET_STATEBASEDCCENERGYCONSUMER_H

#include "inet/physicallayer/contract/packetlevel/IRadio.h"
#include "inet/power/contract/ICcEnergyConsumer.h"
#include "inet/power/contract/ICcEnergySource.h"

namespace inet {

namespace physicallayer {

/**
 * This is a simple radio energy consumer model. The current consumption is
 * determined by the radio mode, the transmitter state and the receiver state
 * using constant parameters.
 *
 * @author Levente Meszaros
 */
class INET_API StateBasedCcEnergyConsumer : public cSimpleModule, public virtual power::ICcEnergyConsumer, public cListener
{
  protected:
    // parameters
    V minSupplyVoltage = V(NaN);
    V maxSupplyVoltage = V(NaN);
    A offCurrentConsumption = A(NaN);
    A sleepCurrentConsumption = A(NaN);
    A switchingCurrentConsumption = A(NaN);
    A receiverIdleCurrentConsumption = A(NaN);
    A receiverBusyCurrentConsumption = A(NaN);
    A receiverReceivingCurrentConsumption = A(NaN);
    A receiverReceivingPreambleCurrentConsumption = A(NaN);
    A receiverReceivingHeaderCurrentConsumption = A(NaN);
    A receiverReceivingDataCurrentConsumption = A(NaN);
    A transmitterIdleCurrentConsumption = A(NaN);
    A transmitterTransmittingCurrentConsumption = A(NaN);
    A transmitterTransmittingPreambleCurrentConsumption = A(NaN);
    A transmitterTransmittingHeaderCurrentConsumption = A(NaN);
    A transmitterTransmittingDataCurrentConsumption = A(NaN);

    // environment
    IRadio *radio = nullptr;
    power::ICcEnergySource *energySource = nullptr;

    // state
    A currentConsumption = A(NaN);

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual A computeCurrentConsumption() const;

  public:
    virtual power::IEnergySource *getEnergySource() const override { return energySource; }
    virtual A getCurrentConsumption() const override { return currentConsumption; }

    virtual void receiveSignal(cComponent *source, simsignal_t signal, intval_t value, cObject *details) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signal, double value, cObject *details) override;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_STATEBASEDCCENERGYCONSUMER_H


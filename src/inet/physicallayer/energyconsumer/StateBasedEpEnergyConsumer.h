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

#ifndef __INET_STATEBASEDEPENERGYCONSUMER_H
#define __INET_STATEBASEDEPENERGYCONSUMER_H

#include "inet/physicallayer/contract/packetlevel/IRadio.h"
#include "inet/power/contract/IEpEnergyConsumer.h"
#include "inet/power/contract/IEpEnergySource.h"

namespace inet {

namespace physicallayer {

/**
 * This is a simple radio power consumer model. The power consumption is
 * determined by the radio mode, the transmitter state and the receiver state
 * using constant parameters.
 *
 * @author Levente Meszaros
 */
class INET_API StateBasedEpEnergyConsumer : public cSimpleModule, public power::IEpEnergyConsumer, public cListener
{
  protected:
    // parameters
    W offPowerConsumption = W(NaN);
    W sleepPowerConsumption = W(NaN);
    W switchingPowerConsumption = W(NaN);
    W receiverIdlePowerConsumption = W(NaN);
    W receiverBusyPowerConsumption = W(NaN);
    W receiverReceivingPowerConsumption = W(NaN);
    W receiverReceivingPreamblePowerConsumption = W(NaN);
    W receiverReceivingHeaderPowerConsumption = W(NaN);
    W receiverReceivingDataPowerConsumption = W(NaN);
    W transmitterIdlePowerConsumption = W(NaN);
    W transmitterTransmittingPowerConsumption = W(NaN);
    W transmitterTransmittingPreamblePowerConsumption = W(NaN);
    W transmitterTransmittingHeaderPowerConsumption = W(NaN);
    W transmitterTransmittingDataPowerConsumption = W(NaN);

    // environment
    IRadio *radio = nullptr;
    power::IEpEnergySource *energySource = nullptr;

    // state
    W powerConsumption = W(NaN);

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual W computePowerConsumption() const;

  public:
    virtual power::IEnergySource *getEnergySource() const override { return energySource; }
    virtual W getPowerConsumption() const override { return powerConsumption; }

    virtual void receiveSignal(cComponent *source, simsignal_t signal, intval_t value, cObject *details) override;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_STATEBASEDEPENERGYCONSUMER_H


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

#ifndef __INET_STATEBASEDENERGYCONSUMER_H
#define __INET_STATEBASEDENERGYCONSUMER_H

#include "inet/power/contract/IEnergySource.h"
#include "inet/physicallayer/contract/packetlevel/IRadio.h"

namespace inet {

namespace physicallayer {

using namespace inet::power;

/**
 * This is a simple radio power consumer model. The power consumption is
 * determined by the radio mode, the transmitter state and the receiver state
 * using constant parameters.
 *
 * @author Levente Meszaros
 */
class INET_API StateBasedEnergyConsumer : public cSimpleModule, public IEnergyConsumer, public cListener
{
  protected:
    // parameters
    W offPowerConsumption;
    W sleepPowerConsumption;
    W switchingPowerConsumption;
    W receiverIdlePowerConsumption;
    W receiverBusyPowerConsumption;
    W receiverReceivingPowerConsumption;
    W receiverReceivingPreamblePowerConsumption;
    W receiverReceivingHeaderPowerConsumption;
    W receiverReceivingDataPowerConsumption;
    W transmitterIdlePowerConsumption;
    W transmitterTransmittingPowerConsumption;
    W transmitterTransmittingPreamblePowerConsumption;
    W transmitterTransmittingHeaderPowerConsumption;
    W transmitterTransmittingDataPowerConsumption;

    // environment
    IRadio *radio;
    IEnergySource *energySource;

    // internal state
    int energyConsumerId;

  public:
    StateBasedEnergyConsumer();

    virtual W getPowerConsumption() const override;

    virtual void receiveSignal(cComponent *source, simsignal_t signalID, long value DETAILS_ARG) override;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }

    virtual void initialize(int stage) override;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_STATEBASEDENERGYCONSUMER_H


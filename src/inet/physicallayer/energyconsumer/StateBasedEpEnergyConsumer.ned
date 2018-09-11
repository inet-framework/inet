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

package inet.physicallayer.energyconsumer;

import inet.power.base.EpEnergyConsumerBase;

//
// This module provides a radio power consumer model. The power consumption is
// determined by the radio mode, the transmitter state and the receiver state
// using constant parameters.
//
simple StateBasedEpEnergyConsumer extends EpEnergyConsumerBase
{
    parameters:
        string energySourceModule;
        double offPowerConsumption @unit(W) = default(0mW);
        double sleepPowerConsumption @unit(W) = default(1mW);
        double switchingPowerConsumption @unit(W) = default(1mW);
        double receiverIdlePowerConsumption @unit(W) = default(2mW);
        double receiverBusyPowerConsumption @unit(W) = default(5mW);
        double receiverReceivingPowerConsumption @unit(W) = default(10mW);
        double receiverReceivingPreamblePowerConsumption @unit(W) = default(receiverReceivingPowerConsumption);
        double receiverReceivingHeaderPowerConsumption @unit(W) = default(receiverReceivingPowerConsumption);
        double receiverReceivingDataPowerConsumption @unit(W) = default(receiverReceivingPowerConsumption);
        double transmitterIdlePowerConsumption @unit(W) = default(2mW);
        double transmitterTransmittingPowerConsumption @unit(W) = default(100mW);
        double transmitterTransmittingPreamblePowerConsumption @unit(W) = default(transmitterTransmittingPowerConsumption);
        double transmitterTransmittingHeaderPowerConsumption @unit(W) = default(transmitterTransmittingPowerConsumption);
        double transmitterTransmittingDataPowerConsumption @unit(W) = default(transmitterTransmittingPowerConsumption);
        @display("i=block/cogwheel");
        @class(StateBasedEpEnergyConsumer);
}


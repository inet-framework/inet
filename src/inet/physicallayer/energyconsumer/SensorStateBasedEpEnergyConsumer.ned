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

//
// This module provides a radio power consumer model. The power consumption is
// determined by the radio mode, the transmitter state and the receiver state
// using constant parameters.
//
// This module provides default values 
// for low power wireless sensor nodes. Roughly based on 
// the data sheet for the CC2500 RF transceiver.
//

simple SensorStateBasedEpEnergyConsumer extends StateBasedEpEnergyConsumer
{
    parameters:
        offPowerConsumption = default(0mW);
        sleepPowerConsumption = default(0.001mW);
        switchingPowerConsumption = default(25mW);
        receiverIdlePowerConsumption = default(0.005mW);
        receiverBusyPowerConsumption = default(0.1mW);
        receiverReceivingPowerConsumption = default(50mW);
        transmitterIdlePowerConsumption = default(5mW);
        transmitterTransmittingPowerConsumption = default(75mW);
}


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
// This module provides a radio energy consumer model. The current consumption is
// determined by the radio mode, the transmitter state and the receiver state
// using constant parameters.
//
// This module extends StateBasedCcEnergyConsumer, and provides default values 
// suitable for using in low power wireless sensor nodes. Roughly based on 
// the data sheet for the CC2500 RF transceiver.
//

simple SensorStateBasedCcEnergyConsumer extends StateBasedCcEnergyConsumer
{
    parameters:
        offCurrentConsumption @unit(A) = default(0mA);
        sleepCurrentConsumption @unit(A) = default(0.001mA);
        switchingCurrentConsumption @unit(A) = default(5mA);
        receiverIdleCurrentConsumption @unit(A) = default(0.002mA);
        receiverBusyCurrentConsumption @unit(A) = default(0.025mA);
        receiverReceivingCurrentConsumption @unit(A) = default(15mA);
        transmitterIdleCurrentConsumption @unit(A) = default(2mA);
        transmitterTransmittingCurrentConsumption @unit(A) = default(25mA);
}


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

import inet.power.base.CcEnergyConsumerBase;

//
// This module provides a radio energy consumer model. The current consumption is
// determined by the radio mode, the transmitter state and the receiver state
// using constant parameters.
//
simple StateBasedCcEnergyConsumer extends CcEnergyConsumerBase
{
    parameters:
        string energySourceModule;
        double minSupplyVoltage @unit(V) = default(0V);
        double maxSupplyVoltage @unit(V) = default(inf V);
        double offCurrentConsumption @unit(A) = default(0mA);
        double sleepCurrentConsumption @unit(A) = default(1mA);
        double switchingCurrentConsumption @unit(A) = default(1mA);
        double receiverIdleCurrentConsumption @unit(A) = default(2mA);
        double receiverBusyCurrentConsumption @unit(A) = default(5mA);
        double receiverReceivingCurrentConsumption @unit(A) = default(10mA);
        double receiverReceivingPreambleCurrentConsumption @unit(A) = default(receiverReceivingCurrentConsumption);
        double receiverReceivingHeaderCurrentConsumption @unit(A) = default(receiverReceivingCurrentConsumption);
        double receiverReceivingDataCurrentConsumption @unit(A) = default(receiverReceivingCurrentConsumption);
        double transmitterIdleCurrentConsumption @unit(A) = default(2mA);
        double transmitterTransmittingCurrentConsumption @unit(A) = default(100mA);
        double transmitterTransmittingPreambleCurrentConsumption @unit(A) = default(transmitterTransmittingCurrentConsumption);
        double transmitterTransmittingHeaderCurrentConsumption @unit(A) = default(transmitterTransmittingCurrentConsumption);
        double transmitterTransmittingDataCurrentConsumption @unit(A) = default(transmitterTransmittingCurrentConsumption);
        @display("i=block/cogwheel");
        @class(StateBasedCcEnergyConsumer);
}


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
// This module provides a radio power consumer model for IEEE 802.11 radios.
// Default values are roughly based on CC3220 transceiver. The power consumption is
// determined by the radio mode, the transmitter state and the receiver state
// using constant parameters.
//
simple Ieee80211StateBasedCcEnergyConsumer extends StateBasedCcEnergyConsumer
{
    parameters:
        offCurrentConsumption = default(0mA);
        sleepCurrentConsumption = default(0.01mA);
        switchingCurrentConsumption = default(100mA);		// tx/rx calibration mode
        receiverIdleCurrentConsumption = default(100mA);
        receiverBusyCurrentConsumption = default(100mA);	// no info yet
        receiverReceivingCurrentConsumption = default(100mA);
        receiverReceivingPreambleCurrentConsumption = default(receiverReceivingCurrentConsumption);
        receiverReceivingHeaderCurrentConsumption = default(receiverReceivingCurrentConsumption);
        receiverReceivingDataCurrentConsumption = default(receiverReceivingCurrentConsumption);
        transmitterIdleCurrentConsumption = default(100mA);	// no info yet
        transmitterTransmittingCurrentConsumption = default(250mA);
        transmitterTransmittingPreambleCurrentConsumption = default(transmitterTransmittingCurrentConsumption);
        transmitterTransmittingHeaderCurrentConsumption = default(transmitterTransmittingCurrentConsumption);
        transmitterTransmittingDataCurrentConsumption = default(transmitterTransmittingCurrentConsumption);
}


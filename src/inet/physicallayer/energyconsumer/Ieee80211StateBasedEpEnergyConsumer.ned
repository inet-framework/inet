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
// This module provides a radio power consumer model for IEEE 802.11 radios.
// Default values are roughly based on CC3220 transceiver. The power consumption is
// determined by the radio mode, the transmitter state and the receiver state
// using constant parameters.
//
simple Ieee80211StateBasedEpEnergyConsumer extends StateBasedEpEnergyConsumer
{
    parameters:
        offPowerConsumption = default(0mW);
        sleepPowerConsumption = default(0.05mW);
        switchingPowerConsumption = default(100mW);		// tx/rx calibration mode
        receiverIdlePowerConsumption = default(0.5W);
        receiverBusyPowerConsumption = default(0.5W);	// no info yet
        receiverReceivingPowerConsumption = default(1W);
        receiverReceivingPreamblePowerConsumption = default(receiverReceivingPowerConsumption);
        receiverReceivingHeaderPowerConsumption = default(receiverReceivingPowerConsumption);
        receiverReceivingDataPowerConsumption = default(receiverReceivingPowerConsumption);
        transmitterIdlePowerConsumption = default(1W);	// no info yet
        transmitterTransmittingPowerConsumption = default(2.5W);
        transmitterTransmittingPreamblePowerConsumption = default(transmitterTransmittingPowerConsumption);
        transmitterTransmittingHeaderPowerConsumption = default(transmitterTransmittingPowerConsumption);
        transmitterTransmittingDataPowerConsumption = default(transmitterTransmittingPowerConsumption);
}


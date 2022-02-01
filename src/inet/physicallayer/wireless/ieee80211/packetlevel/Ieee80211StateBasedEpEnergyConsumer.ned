//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.ieee80211.packetlevel;

import inet.physicallayer.wireless.common.energyconsumer.StateBasedEpEnergyConsumer;

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
        switchingPowerConsumption = default(100mW);     // tx/rx calibration mode
        receiverIdlePowerConsumption = default(0.5W);
        receiverBusyPowerConsumption = default(0.5W);   // no info yet
        receiverReceivingPowerConsumption = default(1W);
        receiverReceivingPreamblePowerConsumption = default(receiverReceivingPowerConsumption);
        receiverReceivingHeaderPowerConsumption = default(receiverReceivingPowerConsumption);
        receiverReceivingDataPowerConsumption = default(receiverReceivingPowerConsumption);
        transmitterIdlePowerConsumption = default(1W);  // no info yet
        transmitterTransmittingPowerConsumption = default(2.5W);
        transmitterTransmittingPreamblePowerConsumption = default(transmitterTransmittingPowerConsumption);
        transmitterTransmittingHeaderPowerConsumption = default(transmitterTransmittingPowerConsumption);
        transmitterTransmittingDataPowerConsumption = default(transmitterTransmittingPowerConsumption);
}


//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.ieee80211.packetlevel;

import inet.physicallayer.wireless.common.energyconsumer.StateBasedCcEnergyConsumer;

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
        switchingCurrentConsumption = default(100mA);       // tx/rx calibration mode
        receiverIdleCurrentConsumption = default(100mA);
        receiverBusyCurrentConsumption = default(100mA);    // no info yet
        receiverReceivingCurrentConsumption = default(100mA);
        receiverReceivingPreambleCurrentConsumption = default(receiverReceivingCurrentConsumption);
        receiverReceivingHeaderCurrentConsumption = default(receiverReceivingCurrentConsumption);
        receiverReceivingDataCurrentConsumption = default(receiverReceivingCurrentConsumption);
        transmitterIdleCurrentConsumption = default(100mA);    // no info yet
        transmitterTransmittingCurrentConsumption = default(250mA);
        transmitterTransmittingPreambleCurrentConsumption = default(transmitterTransmittingCurrentConsumption);
        transmitterTransmittingHeaderCurrentConsumption = default(transmitterTransmittingCurrentConsumption);
        transmitterTransmittingDataCurrentConsumption = default(transmitterTransmittingCurrentConsumption);
}


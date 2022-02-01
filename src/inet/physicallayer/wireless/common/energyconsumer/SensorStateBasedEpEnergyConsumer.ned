//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.energyconsumer;

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


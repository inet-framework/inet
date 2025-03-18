//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.energyconsumer;

//
// Provides a radio energy consumer model. The current consumption is
// determined by the radio mode, the transmitter state, and the receiver state
// using constant parameters.
//
// This module extends `StateBasedCcEnergyConsumer` and provides default values
// suitable for use in low-power wireless sensor nodes. Approximately based on
// the datasheet for the CC2500 RF transceiver.
//

//
// Current consumption model for low-power wireless sensor nodes based on radio
// state. Provides realistic current consumption values for different operational
// states (sleep, idle, receiving, transmitting) based on the CC2500 RF
// transceiver specifications. Used for accurate battery life simulation in
// wireless sensor networks.
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

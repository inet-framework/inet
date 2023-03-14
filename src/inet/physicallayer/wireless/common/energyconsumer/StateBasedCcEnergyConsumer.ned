//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.energyconsumer;

import inet.power.contract.ICcEnergyConsumer;

//
// This module provides a radio energy consumer model. The current consumption is
// determined by the radio mode, the transmitter state and the receiver state
// using constant parameters.
//
simple StateBasedCcEnergyConsumer like ICcEnergyConsumer
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
        @signal[currentConsumptionChanged](type=double);
        @statistic[currentConsumption](title="Current consumption"; source=currentConsumptionChanged; record=vector; interpolationmode=sample-hold; unit=A);
}


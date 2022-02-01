//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.power.contract;

//
// This interface extends the corresponding energy model interface. It requires
// implementations to describe energy consumption and energy generation with
// current [A], and storage capacity with charge [C] and output voltage [V].
// The Cc is an abbreviation that is used for charge and current based interfaces.
//
// @see ~ICcEnergyConsumer, ~ICcEnergyGenerator, ~ICcEnergySink, ~ICcEnergyStorage, ~ICcEnergyManagement
//
moduleinterface ICcEnergySource extends IEnergySource
{
    parameters:
        @display("i=block/plug");
        @signal[currentConsumptionChanged](type=double);
}


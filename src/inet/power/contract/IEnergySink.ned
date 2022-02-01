//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.power.contract;

//
// The energy sink models absorb energy from multiple energy generators.
//
// @see ~IEnergyConsumer, ~IEnergySource, ~IEnergyGenerator, ~IEnergyStorage, ~IEnergyManagement
//
moduleinterface IEnergySink
{
    parameters:
        @display("i=block/plug");
}


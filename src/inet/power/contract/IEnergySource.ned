//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.power.contract;

//
// The energy source models provide energy for multiple energy consumers.
//
// @see ~IEnergyConsumer, ~IEnergyGenerator, ~IEnergySink, ~IEnergyStorage, ~IEnergyManagement
//
moduleinterface IEnergySource
{
    parameters:
        @display("i=block/plug");
}


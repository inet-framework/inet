//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.power.contract;

//
// The energy generator models describe the energy generation process of devices
// over time. A solar panel, for example, produces energy based on time, the
// panel's position on the globe, its orientation towards the sun and the actual
// weather conditions.
//
// @see ~IEnergyConsumer, ~IEnergySource, ~IEnergySink, ~IEnergyStorage, ~IEnergyManagement
//
moduleinterface IEnergyGenerator
{
    parameters:
        @display("i=block/plug");
}


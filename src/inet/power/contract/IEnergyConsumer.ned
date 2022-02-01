//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.power.contract;

//
// The energy consumer models describe the energy consumption process of devices
// over time. For example, a radio consumes energy when it transmits or receives
// signals, or a CPU consumes energy when the network layer processes packets,
// or a display consumes energy when it's turned on, etc.
//
// @see ~IEnergySource, ~IEnergyGenerator, ~IEnergySink, ~IEnergyStorage, ~IEnergyManagement
//
moduleinterface IEnergyConsumer
{
    parameters:
        @display("i=block/plug");
}


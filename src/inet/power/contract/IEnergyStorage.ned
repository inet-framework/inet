//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.power.contract;

//
// The energy storage models describe devices that absorb energy produced by
// generators, and provide energy for consumers. For example, an electrochemical
// battery in a mobile phone provides energy for its display, its CPU, and its
// wireless communication device. It can also absorb energy produced by a solar
// panel installed on its display, or by a portable charger plugged into a wall
// socket.
//
// @see ~IEnergyConsumer, ~IEnergySource, ~IEnergyGenerator, ~IEnergySink, ~IEnergyManagement
//
moduleinterface IEnergyStorage extends IEnergySource, IEnergySink
{
    parameters:
        @display("i=block/plug");
}


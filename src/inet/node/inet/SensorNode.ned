//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.node.inet;

import inet.node.base.ApplicationLayerNodeBase;

//
// A wireless sensor network (WSN) node with low-power communication
// capabilities. Provides a specialized node type for simulating resource-constrained
// devices commonly used in environmental monitoring, industrial automation, and IoT
// applications.
//
// The SensorNode is configured with an IEEE 802.15.4 narrowband wireless interface
// by default, which is designed for low-rate wireless personal area networks (LR-WPANs)
// with an emphasis on low cost and low power consumption. It also includes an energy
// storage module to model battery life and power constraints.
//
module SensorNode extends ApplicationLayerNodeBase
{
    parameters:
        @display("i=misc/sensor2");
        @figure[submodules];
        numWlanInterfaces = default(1);
        energyStorage.typename = default("IdealEpEnergyStorage");
        wlan[*].typename = default("Ieee802154NarrowbandInterface");
        wlan[*].radio.energyConsumer.typename = default("SensorStateBasedEpEnergyConsumer"); //FIXME does this consumer exist???
}

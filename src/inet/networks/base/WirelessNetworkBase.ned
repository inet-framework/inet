//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networks.base;

import inet.environment.contract.IPhysicalEnvironment;
import inet.physicallayer.wireless.common.contract.packetlevel.IRadioMedium;

network WirelessNetworkBase extends NetworkBase
{
    submodules:
        radioMedium: <default("RadioMedium")> like IRadioMedium if typename != "" {
            parameters:
                @display("p=100,400;is=s");
        }
        physicalEnvironment: <default("PhysicalEnvironment")> like IPhysicalEnvironment if typename != "" {
            parameters:
                @display("p=100,500;is=s");
        }
}

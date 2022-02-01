//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.node.wireless;

import inet.mobility.contract.IMobility;
import inet.node.contract.INetworkNode;
import inet.physicallayer.wireless.common.contract.packetlevel.IAntenna;

module Probe like INetworkNode
{
    parameters:
        @networkNode;
        @display("i=device/antennatower");
    submodules:
        mobility: <default("StationaryMobility")> like IMobility {
            parameters:
                subjectModule = default("^");
                @display("p=100,100");
        }
        antenna: <default("IsotropicAntenna")> like IAntenna {
            parameters:
                mobilityModule = default("^.mobility");
                @display("p=100,200");
        }
}


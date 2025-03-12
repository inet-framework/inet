//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.node.wireless;

import inet.mobility.contract.IMobility;
import inet.node.contract.INetworkNode;
import inet.physicallayer.wireless.common.contract.packetlevel.IAntenna;

//
// A minimal wireless network node that consists only of an antenna
// and mobility model. Designed for monitoring wireless signals without actively
// participating in network communication.
//
// Unlike full-featured wireless nodes, the Probe does not have protocol stacks,
// applications, or network interfaces. It simply provides the basic infrastructure
// needed to detect and measure wireless signals in the simulation environment.
//
// The Probe can be used for various measurement and analysis purposes in wireless
// simulations, such as signal strength mapping, interference analysis, and coverage
// testing. When the simulation runs in Qtenv, the module can be dragged (shift + mouse)
// around. Visualizers should be configured to monitor the transmission medium.
//
// @see ~AccessPoint, ~WirelessHost, ~MediumCanvasVisualizer
//
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


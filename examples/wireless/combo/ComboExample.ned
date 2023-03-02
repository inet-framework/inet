//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.examples.wireless.combo;

import inet.environment.common.PhysicalEnvironment;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.WirelessHost;
import inet.physicallayer.wireless.common.medium.RadioMedium;
import inet.visualizer.contract.IIntegratedVisualizer;

network ComboExample
{
    parameters:
        int numHosts;
        @display("bgb=1000,400");
    submodules:
        host[numHosts]: WirelessHost {
            parameters:
                @display("r=,,#707070;p=300,200");
        }
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=100,50;is=s");
        }
        radioMedium: RadioMedium {
            parameters:
                @display("p=150,50;is=s");
        }
        physicalEnvironment: PhysicalEnvironment {
            parameters:
                @display("p=200,50;is=s");
        }
        visualizer: <default(firstAvailableOrEmpty("IntegratedCanvasVisualizer"))> like IIntegratedVisualizer if typename != "" {
            parameters:
                @display("p=250,50;is=s");
        }
}


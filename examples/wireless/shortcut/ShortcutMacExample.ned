//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.examples.wireless.shortcut;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.WirelessHost;

network ShortcutMacExample
{
    submodules:
        client: WirelessHost {
            parameters:
                @display("p=250,100");
        }
        server: WirelessHost {
            parameters:
                @display("p=400,100");
        }
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=100,100;is=s");
        }
}

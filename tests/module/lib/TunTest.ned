//
// Copyright (C) 2014 Irene Ruengeler
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
//

package inet.test.moduletest.lib;

import inet.node.inet.StandardHost;

network TunTest
{
    parameters:

    submodules:
        testHost: StandardHost {
            parameters:
                ipv4.configurator.networkConfiguratorModule = "";
                @display("p=544,194;i=device/server_l");
        }
    connections:
}


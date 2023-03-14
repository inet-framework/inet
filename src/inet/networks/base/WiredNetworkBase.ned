//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networks.base;

import inet.networklayer.configurator.contract.INetworkConfigurator;

network WiredNetworkBase extends NetworkBase
{
    parameters:
        bool hasGlobalArp = default(true);
        **.ipv4.arp.typename = default(hasGlobalArp ? "GlobalArp" : "Arp");
    submodules:
        macForwardingTableConfigurator: <default(hasGlobalArp ? "MacForwardingTableConfigurator" : "")> like INetworkConfigurator if typename != "" {
            @display("p=100,400;is=s");
        }
}

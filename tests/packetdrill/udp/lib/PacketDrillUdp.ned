//
// Copyright (C) 2014 Irene Ruengeler
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
//

import inet.node.packetdrill.PacketDrillHost;

network PacketDrillUdp
{
    parameters:

    submodules:
        pdhost: PacketDrillHost {
            parameters:
                ipv4.configurator.networkConfiguratorModule = "";
                @display("p=544,194;i=device/server_l");
        }
    connections:
}


//
// Copyright (C) 2024 Daniel Zeitler
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.node.mrp;

import inet.node.ethernet.EthernetSwitch;

//
// Model of an Ethernet switch with the MRP protocol.
//
// @see ~Mrp, ~MrpInterconnection
//
module MrpSwitch extends EthernetSwitch
{
    parameters:
        hasMrp = true;
}


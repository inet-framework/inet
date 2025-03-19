//
// Copyright (C) 2024 Daniel Zeitler
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.node.mrp;

import inet.node.ethernet.EthernetSwitch;

//
// An Ethernet switch with Media Redundancy Protocol (MRP) support.
// It extends the standard EthernetSwitch by enabling the MRP functionality.
//
// MRP is a data network protocol standardized by IEC 62439-2 that allows rings of
// Ethernet switches to overcome any single network failure with recovery time of
// less than 200ms. It operates at the MAC layer and works by blocking one link in
// the ring to prevent loops, while monitoring the ring integrity to quickly restore
// connectivity if a failure occurs.
//
module MrpSwitch extends EthernetSwitch
{
    parameters:
        hasMrp = true;
}


//
// Copyright (C) 2006 Andras Babos and Andras Varga
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.networklayer.contract.ipv4.Ipv4Address;
import inet.routing.ospfv2.router.Ospfv2Common;

cplusplus{{
#include "inet/routing/ospfv2/neighbor/Ospfv2Neighbor.h"
}}

namespace inet::ospfv2;

class Neighbor extends cObject
{
    @existingClass;
    @descriptor(readonly);
    
    Ipv4Address neighborID;
    string stateString;
    unsigned int priority;
    Ipv4Address address;
    DesignatedRouterId designatedRouter;
    DesignatedRouterId backupDesignatedRouter;
    
};
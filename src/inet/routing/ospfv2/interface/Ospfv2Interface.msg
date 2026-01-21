//
// Copyright (C) 2006 Andras Babos and Andras Varga
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.networklayer.contract.ipv4.Ipv4Address;
import inet.routing.ospfv2.neighbor.Ospfv2Neighbor;
import inet.routing.ospfv2.router.Ospfv2Common;

cplusplus{{
#include "inet/routing/ospfv2/interface/Ospfv2Interface.h"
}}

namespace inet::ospfv2;

class Ospfv2Interface extends cObject
{
    @existingClass;
    @descriptor(readonly);
    
    string interfaceName;
    int ifIndex;
    Ipv4AddressRange addressRange;
    string typeString;
    int mtu;
    string stateString;
    string modeString;
    Metric outputCost;
    Ipv4Address areaId;
    Ipv4Address transitAreaId;
    short helloInterval;
    short pollInterval;
    short routerDeadInterval;
    short retransmissionInterval;
    short acknowledgementDelay;
    short transmissionDelay;
    // neighboringRouters[];
    string neighbors;
    Neighbor *neighboringRouters[];
    int routerPriority;
    DesignatedRouterId designatedRouter;
    DesignatedRouterId backupDesignatedRouter;
};
//
// Copyright (C) 2006 Andras Babos and Andras Varga
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.networklayer.contract.ipv4.Ipv4Address;

cplusplus{{
#include "inet/routing/ospfv2/interface/Ospfv2Interface.h"
}}

namespace inet::ospfv2;

struct DesignatedRouterId
{
    @existingClass;
    @descriptor(readonly);
    
    Ipv4Address routerID;
    Ipv4Address ipInterfaceAddress;
};

class Ospfv2Interface extends cObject
{
    @existingClass;
    @descriptor(readonly);
    
    string interfaceName;
    int ifIndex;
    string typeString;
    int mtu;
    string stateString;
    string modeString;
    unsigned long outputCost;
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
    int routerPriority;
    DesignatedRouterId designatedRouter;
    DesignatedRouterId backupDesignatedRouter;
};
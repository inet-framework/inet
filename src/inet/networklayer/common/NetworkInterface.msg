//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.linklayer.common.MacAddress;
import inet.networklayer.contract.ipv4.Ipv4Address;

cplusplus {{
#include "inet/networklayer/common/NetworkInterface.h"
}}

namespace inet;

class NetworkInterface extends cSimpleModule
{
    @existingClass;
    @implements(cIListener);
    @descriptor(readonly);
    bool up @getter("isUp");
    bool broadcast @getter("isBroadcast");
    bool multicast @getter("isMulticast");
    bool pointToPoint @getter("isPointToPoint");
    bool loopback @getter("isLoopback");
    int mtu;
    MacAddress macAddress;
    Ipv4Address ipv4Address @byValue @toValue(.str()) @fromValue(Ipv4Address($));
    Ipv4Address ipv4Netmask @byValue @toValue(.str()) @fromValue(Ipv4Address($));
    cObject *protocolData[] @getter(getProtocolData) @sizeGetter(getNumProtocolData);
}

//
// Copyright (C) 2007
// Faqir Zarrar Yousaf
// Communication Networks Institute, Dortmund University of Technology (TU Dortmund), Germany.
// Christian Bauer
// Institute of Communications and Navigation, German Aerospace Center (DLR)
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//

package inet.networklayer.mipv6;

import inet.common.SimpleModule;

//
// Implements Mobile IPv6 (RFC 3775): the mobility signalling and binding
// management logic for a mobile node, home agent, or correspondent node (the
// role is selected by the `isMobileNode` and `isHomeAgent` parameters). It is
// a submodule of ~Ipv6NetworkLayer (enabled by its `hasMipv6` parameter) and
// works together with the ~BindingUpdateList and ~BindingCache modules.
//
simple Mipv6 extends SimpleModule
{
    parameters:
        @class(Mipv6);
        string interfaceTableModule;   // The path to the InterfaceTable module
        string ipv6NeighbourDiscoveryModule;
        string bindingUpdateListModule;
        string bindingCacheModule;
        bool isHomeAgent;
        bool isMobileNode;
        double maxHaBindingLifeTime @unit(s) = default(3600s); // max binding lifetime for HA registration
        double maxRrBindingLifeTime @unit(s) = default(420s);  // max binding lifetime for CN (route optimization)
        bool useRouteOptimization = default(true);  // whether a mobile node route-optimizes with correspondent nodes (RFC 3775); if false, their traffic always uses the bidirectional home-agent tunnel
        @display("i=block/classifier2");
        @signal[mipv6RoCompleted](type=cMessage?);
        @signal[packetDropped](type=cPacket);
    gates:
        input fromIPv6;
        output toIPv6;
}


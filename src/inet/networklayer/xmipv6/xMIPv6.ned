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

package inet.networklayer.xmipv6;

//
// Implements xMIPv6 (where x = F, H, F-H).
//
simple xMIPv6
{
    parameters:
        string interfaceTableModule;   // The path to the InterfaceTable module
        string ipv6TunnelingModule;
        string ipv6NeighbourDiscoveryModule;
        string bindingUpdateListModule;
        string bindingCacheModule;
        bool isHomeAgent;
        bool isMobileNode;
        @display("i=block/classifier2");
        @signal[mipv6RoCompleted](type=cMessage?);
        @signal[packetDropped](type=cPacket);
    gates:
        input fromIPv6;
        output toIPv6;
}


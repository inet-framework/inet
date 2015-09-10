//
// This library is free software, you can redistribute it
// and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//


package inet.routing.ospfv2;

import inet.routing.contract.IIPRouting;

//
// Implements the OSPFv4 routing protocol.
//
simple OSPFRouting like IIPRouting
{
    parameters:
        string interfaceTableModule;
        string routingTableModule;
        xml ospfConfig; // xml containing the full OSPF AS configuration

        // default values for attributes of interface xml entries:
        int helloInterval @unit(s) = default(10s);
        int pollInterval @unit(s) = default(120s);
        int routerDeadInterval @unit(s) = default(40s);
        int retransmissionInterval @unit(s) = default(5s);
        int interfaceOutputCost = default(1);
        int interfaceTransmissionDelay = default(1);
        int routerPriority = default(1);
        string authenticationType = default("NullType");    // SimplePasswordType|CrytographicType|NullType
        string authenticationKey = default("0x00");         // 0xnn..nn
        int linkCost = default(1);
        bool RFC1583Compatible = default(false);

        string areaID = default("");
        int externalInterfaceOutputCost = default(1);
        string externalInterfaceOutputType = default("");  // Type1|Type2

        @display("i=block/network2");
    gates:
        input ipIn @labels(IPv4ControlInfo/up);
        output ipOut @labels(IPv4ControlInfo/down);
}

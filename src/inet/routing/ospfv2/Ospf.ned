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

import inet.routing.contract.IOspf;

//
// Implements the OSPFv2 routing protocol.
//
// The ospfConfig parameter can specify an XML file describing the
// configuration of all OSPF routers and their interfaces in the network.
//
// The XML configuration must contain exactly one <OSPFASConfig> root element which
// describes an OSPF autonomous system with its areas and its routers. The
// Area IDs, Router IDs and area Address Ranges must be unique within the
// autonomous system. Various OSPF parameters can be specified for the network
// interfaces in the XML file. If those parameters are not specified in the XML file,
// the module parameters of the OSPF module are used instead.
//
// - <Area> - defines an area in the network.
//   - @id - Mandatory attribute containing the area ID in dotted decimal notation.
//     The backbone is '0.0.0.0'. (NOTE: This is not an IP address though it looks like one.)
//   - <AddressRange> - Describes an address range of an OSPF area.
//     - @address - A string which is resolvable by IPvXAddressResolver class. (e.g. Area1.R1>Area1.N1)
//     - @mask - A string which is resolvable by IPvXAddressResolver class. (e.g. Area1.R1>Area1.N1)
//     - @status - Decides whether an address range will be advertised in Summary LSAs. (Advertise or DoNotAdvertise)
//   - <Stub> - Any OSPF area (except the backbone) can be configured as a stub area.
//     - @defaultCost - A required attribute that defines the default route cost (>=0 and <1000) advertised for the area.
//
// - <Router> - Describes an OSPF router with its interfaces. Interface 'ifName' and 'toward' values
//   must be unique within the same router and only one of them must be specified. A router is
//   identified by its Router ID, which is given as an IPv4 address (but isn't necessarily one).
//   It may contain various child elements.
//   - @name - A required string containing a node path specifying the router to which this configuration belongs to.
//   - @RFC1583Compatible - optional true or false value
//   - <PointToPointInterface> - Describes the parameters of a point-to-point interface. It is identified
//     in the router by its ifName value, or name of toward module.
//     - @ifName - A unique name of the interface (e.g. eth0) or ...
//     - @toward - a string containing a node path, for example "Area1.Router2"
//     - @areaID - An optional attribute containing the area ID in dotted decimal notation
//     - @interfaceOutputCost - An optional attribute specifying the associated cost (>=0 and <1000)
//     - @retransmissionInterval - An optional integer
//     - @interfaceTransmissionDelay - An optional integer
//     - @helloInterval - An optional integer that must be the same for all router interfaces attached to the same network
//     - @routerDeadInterval - An optional integer that must be the same for all router interfaces attached to the same network
//     - @authenticationType - Optional OSPF packet authentication protocol (SimplePasswordType | CrytographicType | NullType)
//     - @authenticationKey - Optional 8 byte hexadecimal value e.g. 0x5D8ABDAEDFCB8BEE
//
//   - <BroadcastInterface> - Describes the parameters of a broadcast interface. It is identified in
//     the router by its ifName value, or name of toward module. HelloInterval and RouterDeadInterval
//     must be the same for all router interfaces attached to the same network.
//     - @ifName - A unique name of the interface (e.g. eth0) or ...
//     - @toward - a string containing a node path, for example "Area1.Router2"
//     - @areaID - An optional attribute containing the area ID in dotted decimal notation
//     - @interfaceOutputCost - An optional attribute specifying the associated cost (>=0 and <1000)
//     - @retransmissionInterval - An optional integer
//     - @interfaceTransmissionDelay - An optional integer
//     - @helloInterval - An optional integer that must be the same for all router interfaces attached to the same network
//     - @routerDeadInterval - An optional integer that must be the same for all router interfaces attached to the same network
//     - @authenticationType - Optional OSPF packet authentication protocol (SimplePasswordType | CrytographicType | NullType)
//     - @authenticationKey - Optional 8 byte hexadecimal value e.g. 0x5D8ABDAEDFCB8BEE
//     - @routerPriority - Optional integer specifying the priority in DR/BDR election. Set to 0 to exclude the router from the election.
//
//   - <PointToMultiPointInterface> - Describes the parameters of a point-to-multipoint interface. It is
//     identified in the router by its ifName value, or name of toward module.
//     HelloInterval and RouterDeadInterval must be the same for all router
//     interfaces attached to the same network.
//     - @ifName - A unique name of the interface (e.g. eth0) or ...
//     - @toward - a string containing a node path, for example "Area1.Router2"
//     - @areaID - An optional attribute containing the area ID in dotted decimal notation
//     - @interfaceOutputCost - An optional attribute specifying the associated cost (>=0 and <1000)
//     - @retransmissionInterval - An optional integer
//     - @interfaceTransmissionDelay - An optional integer
//     - @helloInterval - An optional integer that must be the same for all router interfaces attached to the same network
//     - @routerDeadInterval - An optional integer that must be the same for all router interfaces attached to the same network
//     - @authenticationType - Optional OSPF packet authentication protocol (SimplePasswordType | CrytographicType | NullType)
//     - @authenticationKey - Optional 8 byte hexadecimal value e.g. 0x5D8ABDAEDFCB8BEE
//     - <PointToMultiPointNeighborList>
//       - <PointToMultiPointNeighbor> - the content of the element is a string resolvable to an IPv4 address
//
//   - <HostInterface>
//     - @ifName - A unique name of the interface (e.g. eth0) or ...
//     - @toward - a string containing a node path, for example "Area1.Router2"
//     - @areaID - An optional attribute containing the area ID in dotted decimal notation
//     - @attachedHost - IPv4 address of attached host
//     - @linkCost - An optional attribute specifying the associated cost (>=0 and <1000)
//
//   - <NBMAInterface>
//     - @ifName - A unique name of the interface (e.g. eth0) or ...
//     - @toward - a string containing a node path, for example "Area1.Router2"
//     - @areaID - An optional attribute containing the area ID in dotted decimal notation
//     - @interfaceOutputCost - An optional attribute specifying the associated cost (>=0 and <1000)
//     - @retransmissionInterval - An optional integer
//     - @interfaceTransmissionDelay - An optional integer
//     - @helloInterval - An optional integer that must be the same for all router interfaces attached to the same network
//     - @routerDeadInterval - An optional integer that must be the same for all router interfaces attached to the same network
//     - @authenticationType - Optional OSPF packet authentication protocol (SimplePasswordType | CrytographicType | NullType)
//     - @authenticationKey - Optional 8 byte hexadecimal value e.g. 0x5D8ABDAEDFCB8BEE
//     - @routerPriority - Optional integer specifying the priority in DR/BDR election. Set to 0 to exclude the router from the election.
//     - @pollInterval - Optional integer that specifies the length of time, in seconds, between
//                       OSPF packets that the router send before adjacency is established with a neighbor
//     - <NBMANeighborList>
//       - <NBMANeighbor>
//         - @networkInterfaceAddress - A string which is resolvable by IPvXAddressResolver class. (e.g. Area1.R1>Area1.N1)
//         - @neighborPriority - neighbor's priority in the DR/BDR election process.
//
//   - <VirtualLink>
//     - @endPointRouterID - the router ID of the endpoint of the virtual link (required)
//     - @transitAreaID - the ID of the area which is used for tansit
//     - @retransmissionInterval - An optional integer
//     - @interfaceTransmissionDelay - An optional integer
//     - @helloInterval - An optional integer that must be the same for all router interfaces attached to the same network
//     - @routerDeadInterval - An optional integer that must be the same for all router interfaces attached to the same network
//     - @authenticationType - Optional OSPF packet authentication protocol (SimplePasswordType | CrytographicType | NullType)
//     - @authenticationKey - Optional 8 byte hexadecimal value e.g. 0x5D8ABDAEDFCB8BEE
//
simple Ospf like IOspf
{
    parameters:
        string interfaceTableModule;
        string routingTableModule;
        xml ospfConfig; // xml containing the full OSPF AS configuration

        // default values for attributes of interface xml entries:
        int helloInterval @unit(s) = default(10s);  // specifies the time in seconds between hello packets that the other router sends on an interface
        int pollInterval @unit(s) = default(120s);  // specifies the length of time, in seconds, between OSPF packets that the router send before adjacency is established with a neighbor
        int routerDeadInterval @unit(s) = default(40s);  // the interval during which at least one hello packet must be received from a neighbor before the router declares that neighbor as down
        int retransmissionInterval @unit(s) = default(5s);  // The time between OSPF LSA retransmissions for adjacencies that belongs to the interface
        int interfaceOutputCost = default(1);  // cost of link on the interface (1-1000)
        int interfaceTransmissionDelay @unit(s) = default(1s);  // The number of seconds required to transmit a link state update packet. Valid values are 1 to 65535.
        int routerPriority = default(1);  // Priority in DR/BDR election process. Specify 0 to exclude the router from the election.
        string authenticationType @enum("SimplePasswordType","CrytographicType","NullType") = default("NullType");
        string authenticationKey = default("0x00");  // 0xnn..nn
        int linkCost = default(1);  // cost of link (1-1000)
        bool RFC1583Compatible = default(false);

        string areaID = default("");
        int externalInterfaceOutputCost = default(1);  // cost of link (1-1000)
        string externalInterfaceOutputType = default("");  // Type1|Type2

        @display("i=block/network2");
    gates:
        input ipIn @labels(Ipv4ControlInfo/up);
        output ipOut @labels(Ipv4ControlInfo/down);
}

//
// (C) 2005 Vojtech Janota
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


package inet.networklayer.rsvp_te;

//
// Implements RSVP-TE, a signalling protocol for MPLS. The module
// processes RSVP-TE messages, installs labels and does the reservation
// along LSP paths.
//
// Paths can be specified statically in an XML config file ("traffic"
// parameter), or can be set up and torn down dynamically with ~ScenarioManager
// commands.
//
// An example traffic.xml file:
//
//# FIXME verify against the code, and document formally (DTD)!
//# <setup_pri>, <holding_pri>, <sender>, <owner>, <max_delay> elements
//# are implemented but missing from this doc!
//
// <pre>
// <?xml version="1.0"?>
// <sessions>
//     <session>
//         <endpoint>host3</endpoint>
//         <tunnel_id>1</tunnel_id>
//         <paths>
//             <path>
//                 <lspid>100</lspid>
//                 <bandwidth>100000</bandwidth>
//                 <route>
//                     <node>10.1.1.1</node>
//                     <lnode>10.1.2.1</lnode>
//                     <node>10.1.4.1</node>
//                     <node>10.1.5.1</node>
//                 </route>
//                 <permanent>true</permanent>
//                 <color>100</color>
//             </path>
//         </paths>
//     </session>
// </sessions>
// </pre>
//
// In the route, <node> stands for strict hop, and <lnode> for loose hop.
//
// ~RSVP understands the "add-session" and "del-session" ~ScenarioManager
// commands. The contents of the <add-session> element can be the same
// as the <session> element for the traffic.xml above.
// The <del-command> element syntax is also similar, but only
// <endpoint>, <tunnel_id> and <lspid> needs to be specified.
//
// The following is an example scenario.xml file:
//
// <pre>
// <?xml version="1.0"?>
// <scenario>
//     <at t="2">
//         <add-session module="LSR1.rsvp">
//             <endpoint>10.2.1.1</endpoint>
//             <tunnel_id>1</tunnel_id>
//             <paths>
//                 ...
//             </paths>
//         </add-session>
//     </at>
//     <at t="2.4">
//         <del-session module="LSR1.rsvp">
//             <endpoint>10.2.1.1</endpoint>
//             <tunnel_id>1</tunnel_id>
//             <paths>
//                 <path>
//                     <lspid>100</lspid>
//                 </path>
//             </paths>
//         </del-session>
//     </at>
// </scenario>
// </pre>
//
// RSVP messages are subclassed from ~RSVPMessage, and include ~RSVPPathMsg,
// ~RSVPPathTear, ~RSVPPathError, ~RSVPResvMsg and ~RSVPHelloMsg.
//
// RSVP-TE communicates with the following components in the system:
// ~TED, ~MPLS, and may receive commands from ~ScenarioManager.
//
simple RSVP
{
    parameters:
        string interfaceTableModule;   // The path to the InterfaceTable module
        string routingTableModule;
        string libTableModule;
        string tedModule;
        string classifierModule; // The path to the module which implements the IClassifier C++ interface
        xml traffic = default(xml("<sessions/>")); // specifies paths to set up
        string peers; // names of the interfaces towards RSVP peers
        double helloInterval @unit(s);
        double helloTimeout @unit(s);
        @display("i=block/control");
        @signal[NF_TED_CHANGED](type=TEDChangeInfo);
    gates:
        input ipIn @labels(IPv4ControlInfo/up);
        output ipOut @labels(IPv4ControlInfo/down);
        input mplsSwitchIn;
        input appIn;
}


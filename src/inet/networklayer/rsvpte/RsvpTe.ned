//
// Copyright (C) 2005 Vojtech Janota
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.networklayer.rsvpte;

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
// ~RsvpTe understands the "add-session" and "del-session" ~ScenarioManager
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
// RsvpTe messages are subclassed from ~RsvpMessage, and include ~RsvpPathMsg,
// ~RsvpPathTear, ~RsvpPathError, ~RsvpResvMsg and ~RsvpHelloMsg.
//
// RSVP-TE communicates with the following components in the system:
// ~Ted, ~Mpls, and may receive commands from ~ScenarioManager.
//
simple RsvpTe
{
    parameters:
        string interfaceTableModule;   // The path to the InterfaceTable module
        string routingTableModule;
        string libTableModule;
        string tedModule;
        string classifierModule; // The path to the module which implements the IIngressClassifier C++ interface
        xml traffic = default(xml("<sessions/>")); // specifies paths to set up
        string peers; // names of the interfaces towards RSVP peers
        double helloInterval @unit(s);
        double helloTimeout @unit(s);
        @display("i=block/control");
        @signal[tedChanged](type=inet::TedChangeInfo);
    gates:
        input ipIn @labels(Ipv4ControlInfo/up);
        output ipOut @labels(Ipv4ControlInfo/down);
        input mplsSwitchIn;
        input appIn;
}


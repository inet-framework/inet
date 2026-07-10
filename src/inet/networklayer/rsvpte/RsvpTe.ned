//
// Copyright (C) 2005 Vojtech Janota
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.networklayer.rsvpte;

import inet.common.SimpleModule;

//
// Implements RSVP-TE, a signalling protocol for MPLS. The module processes RSVP-TE messages, installs labels and does the reservation along LSP paths.
//
// Paths can be specified statically in an XML config file ("traffic" parameter), or can be set up and torn down dynamically with ~ScenarioManager commands.
//
// An example traffic.xml file:
//
//# FIXME: Verify against the code, and document formally (DTD)!
//# <setup_pri>, <holding_pri>, <sender>, <owner> elements
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
//             </path>
//         </paths>
//     </session>
// </sessions>
// </pre>
//
// In the route, <node> stands for a strict hop, and <lnode> stands for a loose hop.
//
// ~RsvpTe understands the "add-session" and "del-session" ~ScenarioManager commands. The contents of the <add-session> element can be the same as the <session> element for the traffic.xml above.
// The <del-command> element syntax is also similar, but only <endpoint>, <tunnel_id> and <lspid> need to be specified.
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
// RSVP-TE messages are subclassed from ~RsvpMessage and include ~RsvpPathMsg, ~RsvpPathTear, ~RsvpPathError, ~RsvpResvMsg, and ~RsvpHelloMsg.
//
// RSVP-TE communicates with the following components in the system: ~Ted, ~Mpls, and may receive commands from ~ScenarioManager.
//

simple RsvpTe extends SimpleModule
{
    parameters:
        @class(RsvpTe);
        string interfaceTableModule;   // The path to the InterfaceTable module
        string routingTableModule;
        string libTableModule;
        string tedModule;
        string classifierModule; // The path to the module which implements the IIngressClassifier C++ interface
        xml traffic = default(xml("<sessions/>")); // Specifies paths to set up
        string peers = default("auto"); // "auto" (default) derives the RSVP peers from the TED (every directly-connected RSVP-speaking neighbour); otherwise a space-separated list of the interface names towards the RSVP peers
        double helloInterval @unit(s);
        double helloTimeout @unit(s);
        double refreshInterval @unit(s) = default(30s); // RFC 2205's R: nominal PSB/RSB refresh period; each actual refresh is scheduled at a jittered uniform(0.5, 1.5) * refreshInterval per RFC 2205 Section 3.7
        int stateLifetimeFactor = default(3); // RFC 2205's K: PSB/RSB state expires (if not refreshed) after (K + 0.5) * 1.5 * refreshInterval
        double retryInterval @unit(s) = default(1s); // delay before retrying a permanent path whose ingress PSB couldn't be created (model-specific, not an RFC 2205 value)
        bool advertiseImplicitNull = default(true); // penultimate hop popping: advertise the implicit null label (RFC 3032) instead of allocating a local pop entry when acting as egress for a tunnel
        @display("i=block/control");
        @signal[tedChanged](type=inet::TedChangeInfo);
        @signal[lspEstablished](type=simtime_t); // LSP setup latency, emitted at the ingress when commitResv first installs the ingress label for a session
        @signal[psbCount](type=long); // current PSBList size, emitted whenever a PSB is created or removed
        @signal[rsbCount](type=long); // current RSBList size, emitted whenever an RSB is created or removed
        @statistic[lspEstablished](title="LSP setup latency"; unit=s; record=histogram,vector; interpolationmode=none);
        @statistic[psbCount](title="number of PSBs"; record=last,vector; interpolationmode=none);
        @statistic[rsbCount](title="number of RSBs"; record=last,vector; interpolationmode=none);
    gates:
        input ipIn @labels(Ipv4ControlInfo/up);
        output ipOut @labels(Ipv4ControlInfo/down);
}


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
// <pre>
// <?xml version="1.0"?>
// <sessions>
//     <session>
//         <endpoint>host3</endpoint>
//         <tunnel_id>1</tunnel_id>
//         <setup_pri>7</setup_pri>
//         <holding_pri>7</holding_pri>
//         <paths>
//             <path>
//                 <lspid>100</lspid>
//                 <sender>10.1.1.1</sender>
//                 <bandwidth>100000</bandwidth>
//                 <route>
//                     <node>10.1.1.1</node>
//                     <lnode>10.1.2.1</lnode>
//                     <node>10.1.4.1</node>
//                     <node>10.1.5.1</node>
//                 </route>
//                 <permanent>true</permanent>
//                 <owner>LSR1.app[0]</owner>
//             </path>
//         </paths>
//     </session>
// </sessions>
// </pre>
//
// <session> elements accept the following children:
// - <tunnel_id>: numeric tunnel id of the session (required).
// - <endpoint>: the session's destination address or resolvable module name (required).
// - <setup_pri>, <holding_pri>: the session's RSVP setup and holding priority
//   (0-7, 0 being the highest), used for admission control and preemption;
//   default 7 if omitted. Set once per session: adding further paths to an
//   existing session with a different value is a configuration error.
// - <paths>: one or more <path> elements (see below).
//
// <path> elements accept the following children:
// - <lspid>: numeric LSP id, unique within the session (required).
// - <sender>: the sender (ingress) address for the SENDER_TEMPLATE object;
//   defaults to this router's address.
// - <bandwidth>: requested bandwidth in bits/sec for admission control;
//   defaults to 0 (best-effort, always admitted). This is the SENDER_TSPEC/
//   FLOWSPEC token rate [r] (RFC 2210).
// - <burst>, <peak_rate>, <min_policed_unit>, <max_packet_size>: the remaining
//   RFC 2210 Int-Serv token-bucket Tspec parameters -- bucket depth [b], peak
//   rate [p], minimum policed unit [m], maximum packet size [M] -- all default
//   to 0 when omitted (Workstream C11). LIMITATION: these are carried on the
//   wire and propagated/recorded end to end (~IntServ's SenderTspecObj) exactly
//   like <bandwidth>, but admission control (doCACCheck()) and preemption only
//   ever consult <bandwidth> [r] -- there is no per-LSP policing/shaping in
//   this model (that would require wiring ~inet::queueing::TokenBucket into the
//   MPLS forwarding path, out of scope here), so a sender declaring e.g. a
//   burst size larger than what any downstream link could actually absorb is
//   never rejected or throttled for it.
// - <route>: an explicit route (ERO), see below; if omitted, the path is
//   forwarded hop-by-hop according to the routing table (no ERO), unless
//   "computeEro" is on (see below).
// - <permanent>: if true (the default), the module keeps retrying to set up
//   the path (after "retryInterval") if the initial attempt fails. If an
//   already-established permanent path later fails (a PathErr arrives, or its
//   next hop goes down), the module make-before-break reroutes it (RFC 3209
//   Section 4.6.4, Workstream C7): it signals a brand-new replacement LSP-id
//   FIRST, while the old path/PSB is left completely alone; only once the
//   replacement's ingress label is installed does it rebind the classifier's
//   FEC entry to the new LSP-id (the actual traffic cutover) and tear down the
//   old one. If the replacement itself fails before that point, it keeps
//   retrying (same "retryInterval") without disturbing the old path further.
// - <owner>: the module (dotted path) that receives this path's status
//   notifications (created / preempted / failed / unfeasible); defaults to
//   this ~RsvpTe module.
// - <include_any>, <exclude_any>: optional hex-encoded admin-group bitmasks
//   (e.g. "0x3") consulted by CSPF when "computeEro" is on (RFC 3209 Section
//   4.7.4-style affinity, Workstream D3); ignored otherwise.
//
// In the route, <node> stands for a strict hop, and <lnode> stands for a loose hop
// (both take an IPv4 address). Hand-written EROs are the primary way to specify an
// explicit route. Alternatively, if "computeEro" is on and a path's <route> is
// omitted (or contains only loose hops), the ingress computes one itself via CSPF
// (~Ted's revived Bellman-Ford, constrained by bandwidth at the session's setup
// priority and, if given, the <include_any>/<exclude_any> affinity above) and
// installs it as a full strict ERO before signaling.
//
// ~RsvpTe understands the "add-session" and "del-session" ~ScenarioManager commands. The contents of the <add-session> element can be the same as the <session> element for the traffic.xml above.
// The <del-session> element syntax is similar, but only <tunnel_id>, <endpoint>, an optional <extended_tunnel_id> (defaults to this router's address), and (optionally) a <paths> list of <path> elements carrying just <lspid> need to be specified; if <paths> is omitted, every path of the session is torn down.
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
// RSVP-TE messages are subclassed from ~RsvpMessage and include ~RsvpPathMsg, ~RsvpPathTear, ~RsvpPathError, ~RsvpResvMsg, ~RsvpHelloMsg, and (refresh reduction, "refreshReduction") ~RsvpSrefreshMsg.
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
        bool computeEro = default(false); // C6: at the ingress, compute a full strict ERO via CSPF whenever a path's configured route is empty or contains only loose hops (see "computationMode" for WHERE that computation runs); default off keeps every shipped example/showcase (which all hand-write EROs) fingerprint-identical -- CSPF is a purely local ingress computation with no RFC-mandated wire behavior, so defaulting it off is a scope choice, not a compliance gap. Bandwidth-feasible (Ted's UnResvBandwidth pruning at the session's setup priority) and, if configured on a path via <include_any>/<exclude_any>, affinity-constrained (RFC 3209 Section 4.7.4-style admin-group match, Workstream D3).
        string computationMode = default("local"); // Workstream F4 Phase 2: only consulted when "computeEro" is true -- selects WHERE the automatic ERO computation ("computeEro"'s job: WHETHER to compute one at all) actually runs. "local" (default): today's behavior, unchanged -- Ted::calculateShortestPath() called directly and synchronously in this same ingress router. "pce": delegate to a co-located ~Pce via a PCReq/PCRep exchange (RFC 5440 Section 6.5/6.6) over the ~Pcc submodule named by "pccModule" instead -- see ~Pcc::requestPathComputation() for how that asynchronous exchange is bridged onto this synchronous-looking call (in short: a PCReq is sent and this attempt fails exactly like "CSPF found no path yet" -- reusing the SAME permanent-path retry machinery C6 already built, see createIngressPSB() -- until a later retry finds the PCRep has arrived).
        string pccModule = default(""); // module-path reference to a co-located ~Pcc submodule (e.g. "^.app[0]"); required (and only consulted) when "computationMode" is "pce"
        bool delegate = default(false); // Workstream F4 Phase 3 (RFC 8231 stateful delegation): only meaningful when "computationMode" is "pce". When on, every ingress path whose route is delegated that way (Phase 2) is ALSO reported to the PCE (PCRpt) once its label is installed -- initially, and again after any later route change, whether from an internal make-before-break re-route/reoptimization or a PCE-initiated one -- and this RsvpTe accepts and applies a PCE-initiated PCUpd for it (re-routed via make-before-break using the PCE-supplied ERO directly, no fresh CSPF/PCReq round-trip). Controller-death fallback: if the ~Pcc's PCEP session is lost while paths are still delegated and does not come back within its own "stateTimeout" (RFC 8231 Section 5.2), each such path falls back to LOCAL path computation (Ted::calculateShortestPath(), the plain "local" computationMode behavior) from that point on -- it is never delegated/reported to the PCE again, even if the session later recovers (no RFC 8231 Section 5.6 State Synchronization is implemented; a documented scope simplification). Default off keeps every Phase 1/2 deployment (and every shipped example/showcase) completely unaffected.
        double reoptimizeInterval @unit(s) = default(0s); // C7: RFC 4736-lite reoptimization. 0 (default) disables it entirely -- no timer is ever scheduled, regardless of "computeEro" -- keeping every shipped example/showcase fingerprint-identical. When nonzero, every reoptimizeInterval this router re-runs CSPF for each of its own established ingress paths; if a strictly cheaper route (lower total TE metric) is found than the one currently in use, it make-before-break reroutes onto it (same choreography "pathProblem" uses to react to a failed path: signal the new LSP-id first, cut the classifier over once its ingress label is installed, only then tear down the old one). Paths whose current route is not a full strict hop-by-hop ERO (empty, or containing a loose hop) are left alone -- this model only scores routes it can walk hop by hop against the TED.
        bool refreshReduction = default(false); // C8 (RFC 2961 Section 5.4): refresh reduction. Default off keeps every shipped example/showcase fingerprint-identical -- like a real deployment, this is a capability negotiated/enabled per link, not a wire-mandatory behavior. When on, every Path/Resv this router sends carries a MESSAGE_ID object identifying its content; once a PSB/RSB's content has been sent in full at least once, subsequent periodic refreshes send a compact Srefresh (~RsvpSrefreshMsg) naming that id instead of resending the full object set, and the receiver treats a recognized id exactly like a full refresh (resets the state's timeout). Receivers acknowledge received MESSAGE_IDs (and negatively acknowledge an unrecognized Srefresh id) by piggybacking a MESSAGE_ID_ACK/_NACK object on the next message already travelling back toward the originator -- there is no standalone Ack message in this model (a documented scope simplification); a NACK makes the originator fall back to a full resend of that one state.
        double stopOperationExtraTime @unit(s) = default(2s); // extra time to hold up the node's later shutdown stages after a graceful shutdown sent a forced PathTear for an owned session (see handleStopOperation); this module itself goes NOT_OPERATING immediately (further inbound RSVP packets are dropped, not processed), but the local network/link layers are held up until this elapses so the just-enqueued PathTear can actually leave the node instead of being dropped by an already-stopped Ipv4/interface
        @display("i=block/control");
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


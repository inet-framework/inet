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



//
// @page standards.html, MPLS/RSVP/LDP Model - Implemented Standards
//
// The implementation follows those RFCs below:
// - RFC 2702: Requirements for Traffic Engineering Over MPLS
// - RFC 2205: Resource ReSerVation Protocol
// - RFC 3031: Multiprotocol Label Switching Architecture
// - RFC 3036: LDP Specification
// - RFC 3209: RSVP-TE Extension to RSVP for LSP tunnels
// - RFC 2205: RSVP Version 1 - Functional Specification
// - RFC 2209: RSVP Message processing Version 1
//
//#------------------------------------------------------------------------
// @page mpls-pseudocode.html, MPLS Operation
//
// The following algorithm is carried out by the ~MPLS module:
//
// <pre>
// Step 1: - Check which layer the packet is coming from
// Alternative 1: From layer 3
//     Step 1: Find and check the next hop of this packet
//     Alternative 1: Next hop belongs to this MPLS cloud
//         Step 1: Encapsulate the packet in an MPLS packet with
//         IP_NATIVE_LABEL label
//         Step 2: Send to the next hop
//         Step 3: Return
//     Alternative 2: Next hop does not belong to this MPLS cloud
//         Step 1: Send the packet to the next hop
// Alternative 2: From layer 2
//     Step 1: Record the packet incoming interface
//     Step 2: - Check if the packet is for this LSR
//     Alternative 1: Yes
//         Step 1: Check if the packet has label
//         Alternative 1: Yes
//             Step 1: Strip off all labels and send the packet to L3
//             Step 2: Return
//         Alternative 2: No
//             Step 1: Send the packet to L3
//             Step 2: Return
//     Alternative 2: No
//         Step 1: Continue to the next step
//     Step 3: Check the packet type
//     Alternative 1: The packet is native IP
//         Step 1: Check the LSR type
//         Alternative 1: The LSR is an Ingress Router
//             Step 1: Look up LIB for outgoing label
//             Alternative 1: Label cannot be found
//                 Step 1: Check if the label for this FEC is being requested
//                 Alternative 1: Yes
//                     Step 1: Return
//                 Alternative 2: No
//                     Step 1: Store the packet with FEC id
//                     Step 2: Do request the signalling component
//                     Step 3: Return
//             Alternative 2: Label found
//                 Step 1: Carry out the label operation on the packet
//                 Step 2: Forward the packet to the outgoing interface found
//                 Step 3: Return
//         Alternative 2: The LSR is not an Ingress Router
//             Step 1: Print out the error
//             Step 2: Delete the packet and return
//     Alternative 2: The packet is MPLS
//         Step 1: Check the LSR type
//         Alternative 1: The LSR is an Egress Router
//             Step 1: POP the top label
//             Step 2: Forward the packet to the outgoing interface found
//             Step 3: Return
//         Alternative 2: The LSR is not an Egress Router
//             Step 1: Look up LIB for outgoing label
//             Alternative 1: Label cannot be found
//                 Step 1: Check if the label for this FEC is being requested
//                 Alternative 1: Yes
//                     Step 1: Return
//                 Alternative 2: No
//                     Step 1: Store the packet with FEC id
//                     Step 2: Do request the signalling component
//                     Step 3: Return
//             Alternative 2: Label found
//                 Step 1: Carry out the label operation on the packet
//                 Step 2: Forward the packet to the outgoing interface found
//                 Step 3: Return
// Step 2: Return
// </pre>
//
//
//#------------------------------------------------------------------------
// @page ldp-processing.html, LDP Message Processing
//
// The simulation follows message processing rules specified in RFC3036
// (LDP Specification). A summary of the algorithm used in the RFC is
// presented below.
//
// <b>Label Request Message processing</b>
//
// An LSR may transmit a Request message under any of the conditions below:
// - The LSR recognizes a new FEC via the forwarding tale, and the next hop
//   is its LDP peer. The LIB of this LSR does not have a mapping from the
//   next hop for the given FEC.
// - Network topology changes, the next hop to the FEC is no longer valid
//   and new mapping is not available.
// - The LSR receives a Label Request for a FEC from an upstream LDP and it
//   does not have label binding information for this FEC. The FEC next hop
//   is an LDP peer.
//
// Upon receiving a Label Request message, the following procedures will be
// performed:
//
// <pre>
// Step 1: Extract the FEC from the message and locate the incoming interface
//         of the message.
// Step 2: Check whether the FEC is an outstanding FEC.
//     Alternative 1: This FEC is outstanding
//         Step 1: Return
//     Alternative 2: This FEC is not outstanding
//         Step 1: Continue
// Step 3: Check if there is an exact match of the FEC in the routing table.
//     Alternative 1: There is an exact match
//         Step 1: Continue
//     Alternative 2: There is no match
//         Step 1: Construct a Notification message of No route and
//                 send this message back to the sender.
// Step 4: Make query to local LIB to find out the corresponding label.
//     Alternative 1: The label found
//         Step 1: Construct a Label Mapping message and send over
//                 the incoming interface.
//     Alternative 2: The label cannot be found for this FEC
//         Step 1: Construct a new Label Request message and send
//                 the message out using L3 routing.
//         Step 2: Construct a Notification message indicating that the
//                 label cannot be found.
// </pre>
//
// <b>Label Mapping Message processing</b>
//
// Upon receiving a Label Mapping message, the following procedures will be
// performed:
//
// <pre>
// Step 1: Extract the FEC and the label from the message.
// Step 2: Check whether this is an outstanding FEC
//     Alternative 1: This FEC is outstanding
//         Step 1: Continue
//     Alternative 2: This FEC is not outstanding
//         Step 1: Send back the server an Notification of Error message.
// Step 3: Install the new label to the local LIB using the extracted label,
//         FEC and the message incoming interface.
// </pre>
//
//
//#------------------------------------------------------------------------
// @page lib-table-file.html, LIB Table File Format
//
// The format of a LIB table file is:
//
// The beginning of the file should begin with comments. Lines begin with # are treated
// as comments. An empty line is required after the comments. The "LIB TABLE"
// syntax must come next with an empty line. The column headers follow. This header
// must be strictly "In-lbl In-intf Out-lbl Out-intf". Column
// values are after that with space or tab for field separation.
// The following is a sample of lib table file.
//
// <pre>
// #lib table for MPLS network simulation test
// #lib1.table for LSR1 - this is an edge router
// #no incoming label for traffic from in-intf 0 &1 - LSR1 is ingress router for those traffic
// #no outgoing label for traffic from in_intf 2 &3 - LSR 1 is egress router for those traffic
//
// LIB TABLE:
//
// In-lbl  In-intf         Out-lbl     Out-intf
// 1       193.233.7.90    1           193.231.7.21
// 2       193.243.2.1     0           193.243.2.3
// </pre>
//
//#------------------------------------------------------------------------
// @page cspf-algorithm.html, The CSPF Algorithm
//
// CSPF stands for Constraint Shortest Path First.
// This constraint-based routing is executed online by Ingress Router.
// The CSPF calculates an optimum explicit route (ER), based on
// specific constraints. CSPF relies on a Traffic Engineering Database (TED)
// to do those calculations. The resulting route is then used by RSVP-TE.
//
// The CSPF in particular and any constraint based routing process requires following
// inputs:
// - Attributes of the traffic trunks, e.g., bandwidth, link affinities
// - Attributes of the links of the network, e.g. bandwidth, delay
// - Attributes of the LSRs, e.g. types of signaling protocols supported
// - Other topology state information.
//
// There has been no standard for CSPF so far. The implementation of CSPF in
// the simulation is based on the concept of "induced graph" introduced in RFC
// 2702. An induced graph is analogous to a virtual topology in an overlay
// model. It is logically mapped onto the physical network through the
// selection of LSPs for traffic trunks. CSPF is similar to a normal SPF,
// except during link examination, it rejects links without capacity or links
// that do not match color constraints or configured policy. The CSPF
// algorithm used in the simulation has two phases. In the first phase, all
// the links that do not satisfy the constraints of bandwidth are excluded
// from the network topology. The link affinity is also examined in this
// phase. In the second phase, Dijkstra algorithm is performed.
//
// Dijkstra Algorithm:
//
// <pre>
// Dijkstra(G, w, s):
//    Initialize-single-source(G,s);
//    S = empty set;
//    Q = V[G];
//    While Q is not empty {
//        u = Extract-Min(Q);
//        S = S union {u};
//        for each vertex v in Adj[u] {
//            relax(u, v, w);
//        }
//    }
// </pre>
//
// In which:
// - G: the graph, represented in some way (e.g. adjacency list)
// - w: the distance (weight) for each edge (u,v)
// - s (small s): the starting vertex (source)
// - S (big S): a set of vertices whose final shortest path from s have already been determined
// - Q: set of remaining vertices, Q union S = V
//
//
//#------------------------------------------------------------------------
// @page traffic-xml-file.html, The traffic.xml file
//
// The traffic.xml file is read by the RSVP-TE module (RSVP).
// The file must be in the same folder as the executable
// network simulation file.
//
// The XML elements used in the "traffic.xml" file:
// - <Traffic></Traffic> is the root element. It may contain one or more <Conn>
//   elements.
// - <Conn></Conn> specifies an RSVP session. It may contain the following elements.
// - <src></src> specifies sender IP address
// - <dest></dest> specifies receiver IP address
// - <setupPri></setupPri> specifies LSP setup priority
// - <holdingPri></holdingPri> specifies LSP holding priority
// - <bandwidth></bandwidth> specifies the requested BW.
// - <delay></delay> specifies the requested delay.
// - <route></route> specifies the explicit route. This is a comma-separated
//   list of IP address, hop-type pairs (also separated by comma).
//   A hop type has a value of 1 if the hop is a loose hop and 0 otherwise.
//
// The following presents an example file:
//
// <pre>
// <?xml version="1.0"?>
// <!-- Example of traffic control file -->
// <traffic>
//    <conn>
//        <src>10.0.0.1</src>
//        <dest>10.0.1.2</dest>
//        <setupPri>7</setupPri>
//        <holdingPri>7</holdingPri>
//        <bandwidth>400</bandwidth>
//        <delay>5</delay>
//    </conn>
//    <conn>
//        <src>11.0.0.1</src>
//        <dest>11.0.1.2</dest>
//        <setupPri>7</setupPri>
//        <holdingPri>7</holdingPri>
//        <bandwidth>100</bandwidth>
//        <delay>5</delay>
//    </conn>
// </traffic>
// </pre>
//
// An example of using RSVP-TE as signaling protocol can be found in
// ExplicitRouting folder distributed with the simulation. In this
// example, a network similar to the network in LDP-MPLS example is
// setup. Instead of using LDP, "signaling" parameter is set to 2 (value
// of RSVP-TE handler). The following xml file is used for traffic
// control. Note the explicit routes specified in the second connection.
// It indicates that the route is a strict one since the values of every
// hop types are 0. The route defined is 10.0.0.1 -> 1.0.0.1 ->
// 10.0.0.3 -> 1.0.0.4 -> 10.0.0.5 -> 10.0.1.2.
//
// <pre>
// <?xml version="1.0"?>
// <!-- Example of traffic control file -->
// <traffic>
//     <conn>
//         <src>10.0.0.1</src>
//         <dest>10.0.1.2</dest>
//         <setupPri>7</setupPri>
//         <holdingPri>7</holdingPri>
//         <bandwidth>0</bandwidth>
//         <delay>0</delay>
//         <ER>false</ER>
//     </conn>
//     <conn>
//         <src>11.0.0.1</src>
//         <dest>11.0.1.2</dest>
//         <setupPri>7</setupPri>
//         <holdingPri>7</holdingPri>
//         <bandwidth>0</bandwidth>
//         <delay>0</delay>
//         <ER>true</ER>
//         <route>1.0.0.1,0,1.0.0.3,0,1.0.0.4,0,1.0.0.5,0,10.0.1.2,0</route>
//     </conn>
// </traffic>
// </pre>
//

package inet;

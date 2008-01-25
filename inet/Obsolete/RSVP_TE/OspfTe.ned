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
// OSPF-TE module's functionality is to provide a routing mechanism with Traffic
// Engineering supported. The LSPs are setup based on paths selected by OSPF-TE
// protocol.
//
// To address the problem of constrained routing in \IP networks, several
// solutions have been proposed by researchers. Their studies indicate that
// constraint should be applied to flows and not on a per packet basis.
// Bandwidth and delay are two constrained keys, in which delay jitter is less
// important and difficult to efficiently support. In addition, path selection
// should be performed by the source. Intermediate nodes only perform
// connection admission control and do not perform any constrained routing
// decision. Path selection does not need to be standardized. There are
// different types of constrained routing protocols: OSPF-TE, ISIS-TE, PNNI
// (ATM), etc. OSPF-TE is an extension to OSPF. It is designed to aid in the
// establishment of LSPs for traffic engineering.
//
// Operations
//
// OSPF-TE distributes new information about each link. This information includes:
// - Link type and link id
// - Local and remote \IP addresses
// - Traffic engineering metric
// - Maximum bandwidth
// - Maximum reservable bandwidth
// - Unreserved bandwidth
// - Resource class/color
//
// Essentially, OSPF-TE has two parts: Network resource and topology
// distribution and optimal route calculation. The network resource and
// topology distribution is about messages exchanged to get a convergence of
// link states. The route calculation is to find an optimal path based on
// input of the network topology and link states. A complete simulation of
// OSPF-TE would require a great amount of time and effort. Because of the
// time constraint, this simulation builds only the second part of the
// protocol-optimal route calculation. A judgment for this decision can be
// found in the "\RSVP-TE The simulation architecture" section of the report
// acompanying the simulation model.
//
// As mentioned in the previous parts of this report, only the computation
// part of OSPF-TE is built. The algorithm used is CSPF (Constraint Shortest
// Path First), see <a href="cspf-algorithm.html">CSPF Algorithm</a>.
//
// Collaborations
//
// OSPF-TE interacts with TED in order to find the optimal paths for \RSVP-TE
// modules.
//
// Usages
//
// OPSF-TE requires a global (at network scope) TED module to operate correctly.
//
simple OspfTe
{
}

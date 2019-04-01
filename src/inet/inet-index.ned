//
// Copyright (C) 2010 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

//
// @titlepage
//
// <center><h1>INET Framework</h1></center>
// <center>release ##BUILDID##</center>
//
// INET Framework is an open-source model library for the OMNeT++ simulation
// environment. It provides protocols, agents and other models for researchers
// and students working with communication networks. INET is especially useful
// when designing and validating new protocols, or exploring new or exotic
// scenarios.
//
// INET supports a wide class of communication networks, including wired,
// wireless, mobile, ad hoc and sensor networks. It contains models for the
// Internet stack (TCP, UDP, IPv4, IPv6, OSPF, BGP, etc.), link layer
// protocols (Ethernet, PPP, IEEE 802.11, various sensor MAC protocols, etc),
// refined support for the wireless physical layer, MANET routing protocols,
// DiffServ, MPLS with LDP and RSVP-TE signalling, several application models,
// and many other protocols and components. It also provides support for node
// mobility, advanced visualization, network emulation and more.
//
// Several other simulation frameworks take INET as a base, and extend it into
// specific directions, such as vehicular networks, overlay/peer-to-peer
// networks, or LTE.
//
// See the <tt><a href="whatsnew.html">WHATSNEW</a></tt> file in the distribution
// for the list of recent changes.
//
// Related documentation:
//   - <a href="https://inet.omnetpp.org/docs/users-guide/index.html" target="_top">INET User's Guide</a>
//   - <a href="https://inet.omnetpp.org/docs/developers-guide/index.html" target="_top">INET Developer's Guide</a>
//   - <a href="https://inet.omnetpp.org/docs/showcases/index.html" target="_top">INET Showcases</a> and
//     <a href="https://inet.omnetpp.org/docs/tutorials/index.html" target="_top">INET Tutorials</a>
//   - <a href="https://omnetpp.org/doc/omnetpp/manual" target="_top">OMNeT++ Simulation Manual</a>
//   - <a href="https://omnetpp.org/documentation" target="_top">Further OMNeT++ documentation</a>
//
// <a href="history.html">History and contributors</a>.
//
// @page history.html, History
//
// The predecessor of the INET framework was written by Klaus
// Wehrle, Jochen Reber, Dirk Holzhausen, Volker Boehm, Verena Kahmann,
// Ulrich Kaage and others at the University of Karlsruhe during 2000-2001,
// under the name IPSuite.
//
// The MPLS, LDP and RSVP-TE models were built as an add-on to IPSuite
// during 2003 by Xuan Thang Nguyen (Xuan.T.Nguyen@uts.edu.au) and other
// students at the University of Technology, Sydney under supervision of
// Dr Robin Brown. The package consisted of around 10,000 LOCs, and was
// published at http://charlie.it.uts.edu.au/~~tkaphan/xtn/capstone (now
// unavailable).
//
// After a period of IPSuite being unmaintained, Andras Varga took over
// the development in July 2003. Through a series of snapshot releases in
// 2003-2004, modules got completely reorganized, documented, and many of them
// rewritten from scratch. The MPLS models (including RSVP-TE, LDP, etc)
// also got refactored and merged into the codebase. The complete change log
// is available
// <a href="http://ctieware.eng.monash.edu.au/twiki/bin/view/Simulation/IPSuiteLogOfChanges" target="_top">here</a>.
//
// During 2004, Andras added a new, modular and extensible TCP implementation,
// application models, Ethernet implementation and an all-in-one IPv4 model
// to replace the earlier, modularized one.
//
// The package was renamed INET Framework in October 2004.
//
// Support for wireless and mobile networks got added during summer 2005
// by using code from the Mobility Framework.
//
// The MPLS models (including LDP and RSVP-TE) got revised and mostly
// rewritten from scratch by Vojta Janota in the first half of 2005
// for his diploma thesis. After further refinements by Vojta, the new code
// got merged into the INET CVS in fall 2005, and got eventually released
// in the March 2006 INET snapshot.
//
// The OSPFv2 model was created by Andras Babos during 2004 for his diploma
// thesis which was submitted early 2005. After several refinements and fixes,
// the code got merged into the INET Framework in 2005, and became part of the
// March 2006 INET snapshot.
//
// The Quagga routing daemon was ported into the INET Framework also by Vojta
// Janota. During fall 2005 and the months after, ripd and ospfd got ported,
// and the methodology of porting got refined. It is planned to port further
// Quagga daemons after March 2006.
//
// Based on experience from the IPv6Suite (from Ahmet Sekercioglu's group at
// CTIE, Monash University, Melbourne) and IPv6SuiteWithINET (Andras's effort
// to refactor IPv6Suite and merge it with INET early 2005), Wei Yang Ng
// (Monash Uni) implemented a new IPv6 model from scratch for the
// INET Framework in 2005 for his diploma thesis, under guidance from Andras
// who was visiting Monash between February and June 2005. This IPv6 model
// got first included in the July 2005 INET snapshot, and gradually refined
// afterwards.
//
// The SCTP implementation was contributed by Michael Tuexen, Irene Ruengeler
// and Thomas Dreibholz
//
// Support for Sam Jensen's Network Simulation Cradle,
// which makes real-world TCP stacks available in simulations was added
// by Zoltan Bojthe in 2010.
//
// TCP SACK and New Reno implementation was contributed by Thomas Reschka.
//
// Several other people have contributed to the INET Framework by providing
// feedback, reporting bugs, suggesting features and contributing patches;
// I'd like to acknowledge their help here as well.
//
// @page whatsnew.html, What's New
//
// <pre>
// <nohtml>
// @include ../../WHATSNEW
// </nohtml>
// </pre>

package inet;

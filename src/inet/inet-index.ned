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
// <center><h1>INET Framework for OMNeT++/OMNEST</h1></center>
// <center>release ##BUILDID##</center>
//
// INET Framework contains IPv4, IPv6, TCP, SCTP, UDP protocol implementations,
// and several application models. The framework also includes an MPLS model
// with RSVP-TE and LDP signalling. Link-layer models are PPP, Ethernet and 802.11.
// Static routing can be set up using network autoconfigurators, or one can use
// routing protocol implementations.
//
// The INET Framework supports wireless and mobile simulations as well.
//
// See the <tt><a href="whatsnew.html">WHATSNEW</a></tt> file in the distribution
// for the list of recent changes.
//
// Introductory material:
//  - <a href="inet-architecture.html">INET Framework architecture</a>
//  - <a href="../inet-manual-draft.pdf">INET Framework Manual (draft) (pdf)</a>
//
// A few useful starting points for browsing the documentation:
//  - hosts, routers, switches and other network level modules:
//    ~StandardHost, ~Router, ~EtherSwitch, ~EtherHub, ~InternetCloud etc.
//  - network interfaces: ~EthernetInterface, ~PPPInterface, ~Ieee80211Nic
//  - <a href="ieee80211.html">IEEE 802.11 models</a>, both ad-hoc and infrastructure
//    mode are supported
//  - IPv4 layer: ~IPv4, ~ARP and ~ICMP; ~IPv4RoutingTable
//  - IPv6 support: ~IPv6, ~IPv6NeighbourDiscovery, ~ICMPv6, ~IPv6RoutingTable;
//    see <a href="ipv6overview.html">IPv6 model overview</a>
//  - transport layer: ~TCP, ~UDP, ~SCTP
//  - ~InterfaceTable and ~IPv4RoutingTable can be found in every
//    host and router model
//  - queues in router network interfaces: ~DropTailQueue, ~DiffservQueue.
//  - ~IPv4NetworkConfigurator automatically assigns IPv4 addresses and
//    sets up static routes;
//  - ~ScenarioManager lets you change things in the model in the middle
//    of the simulation to study transient behaviour
//  - message classes: ~IPv4Datagram, ~TCPSegment, ~UDPPacket, ~EthernetIIFrame,
//    ~ARPPacket, ~PPPFrame, ~GenericAppMsg, ~EtherAppReq, etc.
//  - control info classes (used in communication between protocol layers):
//    ~IPv4ControlInfo, ~TCPCommand, ~UDPControlInfo, ~Ieee802Ctrl, etc.
//  - application models: ~UDPVideoStreamSvr, ~UDPVideoStreamCli, ~UDPBasicApp, ~UDPEchoApp,
//    ~TCPSessionApp, ~TCPBasicClientApp, ~TCPGenericSrvApp, ~TCPSinkApp,
//    ~TCPSrvHostApp, ~EtherAppCli, ~EtherAppSrv, ~PingApp, ~SimpleVoIPSender,
//    ~SimpleVoIPReceiver etc.
//  - wireless support: ~IRadioMedium, ~Ieee80211Nic
//  - Mobility models: ~TurtleMobility,
//    ~BonnMotionMobility, ~ANSimMobility, ~RandomWPMobility, ~MassMobility
//  - MPLS models: the core modules are ~MPLS, ~LIBTable, ~LDP, ~RSVP, ~TED,
//    ~LinkStateRouting; MPLS-enabled routers are ~LDP_LSR, ~RSVP_LSR;
//    see also the list of <a href="standards.html">implemented standards</a>
//  - dynamic routing is provided by the Daemon module (the Quagga routing
//    daemon ported into the INET Framework, currently in a different project)
//    used inside QuaggaRouter; alternatively one can run ~OSPFRouter which relies
//    on the ~OSPFRouting module;
//
// Related documentation:
//   - <a href="../doxy/index.html" target="_top">C++ source documentation (Doxygen)</a>
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

package inet;

//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2003 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


//
//# @page inet-index.html, About the INET Framework
// @titlepage
//
// <center><h1>INET Framework for OMNeT++/OMNEST</h1></center>
// <center>release 2005-07-20</center>
//
// <nohtml>INET Framework contains IPv4, TCP, UDP protocol implementations,
// and several application models. The framework also includes an MPLS model
// with \RSVP-TE and LDP signalling. RSVP-TE uses the CSPF, the routing
// algorithm in OSPF-TE, with bandwidth as the major resource constraint.
// Link-layer models are PPP and Ethernet.</nohtml>
//
// Recent addition is mobility and wireless support (currently 802.11b
// ad-hoc mode), using the lower layers of the
// <a href="http://mobility-fw.sourceforge.net/" target="_top">Mobility Framework</a>
// Also, several mobility models have been added (TurtleMobility,
// BonnMotionMobility, RandomWPMobility, etc).
//
// <a href="history.html">History and contributors</a>.
//
// For up-to-date info, visit the current
// <a href="http://ctieware.eng.monash.edu.au/twiki/bin/view/Simulation/INETFramework" target="_top">INET Framework home page</a>.
//
// Introductory material:
//  - <a href="../walkthrough/tutorial.html" target="_top">Guided Tour</a>
//  - <a href="../tcp-tutorial/index.html" target="_top">Getting started with TCP</a>
//
// A few useful starting points for browsing the documentation:
//  - the <a href="full-usage-diagram.html">Usage Diagram</a>
//    includes all modules and networks
//  - example networks: NClients, BulkTransfer, MulticastNetwork,
//    ARPTest, MixedLAN, etc.
//  - hosts, routers, switches: StandardHost, Router, EtherSwitch, EtherHub,
//    etc.
//  - network interfaces: EthernetInterface, PPPInterface, Nic80211
//  - higher layer protocols: IP, IPv6, ARP, TCP, UDP
//  - RoutingTable, InterfaceTable and NotificationBoard are there in every
//    host and router model
//  - queues in router network interfaces: DropTailQueue, REDQueue,
//    DropTailQoSQueue.
//  - FlatNetworkConfigurator to automatically assign \IP addresses and
//    set up static routes;
//  - ScenarioManager lets you change things in the model in the middle
//    of the simulation, and study the transient
//  - message classes: IPDatagram, TCPSegment, UDPPacket, EthernetIIFrame,
//    ARPPacket, PPPFrame, GenericAppMsg, EtherAppReq, etc.
//  - control info classes (used in communication between protocol layers):
//    IPControlInfo, TCPCommand, UDPControlInfo, Ieee802Ctrl, etc.
//  - application models: UDPVideoStreamSvr, UDPVideoStreamCli, UDPApp,
//    TCPSessionApp, TCPBasicClientApp, TCPGenericSrvApp, TCPSinkApp,
//    TCPSrvHostApp, EtherAppCli, EtherAppSrv, PingApp, etc.
//  - wireless support: ChannelControl, Nic80211
//  - mobility: MobileHost; mobility models such as TurtleMobility,
//    BonnMotionMobility, ANSimMobility, RandomWPMobility, MassMobility
//
// Starting points for the MPLS models:
//  - the core modules: MPLSModule, LIBTable, LDP, RSVP, TED
//  - MPLS-enabled routers: LDP_LSR, RSVP_LSR
//  - MPLSNet network model
//  - <a href="standards.html">list of implemented standards</a>
//
//
// Related documentation:
//   - <a href="../doxy/index.html" target="_top">C++ source documentation (Doxygen)</a>
//
// @page history.html, History
//
// The predecessor of the INET framework was written by written by Klaus
// Wehrle, Jochen Reber, Dirk Holzhausen, Volker Boehm, Verena Kahmann,
// Ulrich Kaage and others at the University of Karlsruhe during 2000-2001,
// under the name IPSuite.
//
// After a period of IPSuite being unmaintained, Andras Varga took over
// the development in July 2003. Through a series of snapshot releases in
// 2003-2004, modules got completely reorganized, documented, and many of them
// rewritten from scratch. The MPLS models (see below) was also merged
// into the codebase. The complete change log is available
// <a href="http://ctieware.eng.monash.edu.au/twiki/bin/view/Simulation/IPSuiteLogOfChanges" target="_top">here</a>.
// Most notable additions are a new, modular and extensible \TCP implementation,
// application models, Ethernet implementation and an all-in-one IP model
// to replace the earlier, modularized one.
//
// The package was renamed INET Framework in October 2004.
//
// The MPLS, LDP and \RSVP-TE models were built as an add-on to IPSuite.
// The package consists of around 10,000 LOCs, and it was written by Xuan
// Thang Nguyen (Xuan.T.Nguyen@uts.edu.au) and other students at the University
// of Technology, Sydney under supervision of Dr Robin Brown.
// See <a href="http://charlie.it.uts.edu.au/~tkaphan/xtn/capstone/" target="_top">project page</a>
// (no longer updated).
//
// The MPLS models are now also maintained by Andras Varga, as part of the
// INET framework.
//

//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
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
// @titlepage
//
// <center><h1>IPSuite with MPLS</h1></center>
// <center><h3>2004-09-24</h3></center>
//
// <nohtml>IPSuite contains IPv4, TCP, UDP models and support for QoS.
// It also includes an MPLS model with \RSVP-TE and LDP signalling.
// Link-layer models are PPP and Ethernet.</nohtml>
//
// The current codebase is a result of a complete rewrite of the original
// IPSuite by Andras Varga, started in 2003. The original IPSuite was written by 
// Klaus Wehrle, Jochen Reber, Dirk Holzhausen, Volker Boehm, Verena Kahmann, 
// Ulrich Kaage and others at the University of Karlsruhe during 2000-2001.
//
// The MPLS model builds on top of the IPSuite, and implements the MPLS, LDP,
// and \RSVP-TE protocols. It also implements the routing
// algorithm used in OSPF-TE. Its uses bandwidth as the major resource
// constraint. The software has been built around 10,000 LOCs.
// The model has been written by Xuan Thang Nguyen (Xuan.T.Nguyen@uts.edu.au)
// and other students at the University of Technology, Sydney
// under supervision of Dr Robin Brown.
// See <a href="http://charlie.it.uts.edu.au/~tkaphan/xtn/capstone/" target="_top">project page</a> (obsolete).
//
// For up-to-date info, visit the current
// <a href="http://ctieware.eng.monash.edu.au/twiki/bin/view/Simulation/IPv4Suite" target="_top">IPSuite home page</a>.
//
// A few useful starting points for browsing IPSuite:
//  - the <a href="full-usage-diagram.html">Usage Diagram</a>
//    includes all modules and networks
//  - example networks: NClients, ClientServerNetwork, McNetwork2, 
//    ARPTest, MixedLAN, etc.
//  - hosts, routers, switches: StandardHost2, Router, EtherSwitch, EtherHub, 
//    etc.
//  - protocol implementations: IP, IPProcessing, TCPMain, UDPProcessing, 
//    PPPInterface, ARP, EtherMAC, etc.
//  - a central module which holds the interface table and the route table: 
//    RoutingTable
//  - if you don't want to manually set up \IP addresses and routing: 
//    FlatNetworkConfigurator
//  - message classes: IPDatagram, TCPSegment, UDPPacket, EthernetIIFrame, 
//    ARPPacket, PPPFrame, GenericAppMsg, EtherAppReq, etc.
//  - control info classes (used in communication between protocol layers):
//    IPControlInfo, TCPCommand, UDPControlInfo, EtherCtrl, etc.
//  - application models: UDPApp, TCPSessionApp, TCPBasicClientApp, 
//    TCPGenericSrvApp, TCPSinkApp, TCPSrvHostApp, EtherAppCli, EtherAppSrv, 
//    etc.
//
// Starting points for the MPLS models:
//  - the core modules: MPLSModule, LIBTable, LDPproc, RSVP, TED, OspfTe
//  - MPLSNet network model
//  - <a href="standards.html">list of implemented standards</a>
//
// Related documentation:
//   - <a href="../doxy/index.html" target="_top">C++ source documentation (Doxygen)</a>
//

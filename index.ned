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
// <center><h3>2003-09-30</h3></center>
//
// IPSuite is an OMNeT++ model suite for the simulation of 
// <nohtml>TCP/IP</nohtml> protocols and networks. It has been implemented 
// at the Institut fuer Telematik, University Karlsruhe
// by Ulrich Kaage, Klaus Wehrle, Jochen Reber, Verena Kahmann, Volker Boehm
// and others; see the CREDITS file for a complete list.
//
// The MPLS model builds on top of the IPSuite, and implements the MPLS, LDP, 
// and <nohtml>RSVP-TE</nohtml> protocols. It also implements the routing 
// algorithm used in OSPF-TE. Its uses bandwidth as the major resource 
// constraint. The software has been built around 10,000 LOCs.
// The model has been written by Xuan Thang Nguyen (Xuan.T.Nguyen@uts.edu.au) 
// and other students at the University of Technology, Sydney 
// under supervision of Dr Robin Brown.
// Project home page: 
// <a href="http://charlie.it.uts.edu.au/~tkaphan/xtn/capstone/" target="_top">
// http://charlie.it.uts.edu.au/~tkaphan/xtn/capstone/</a>
//
// A few useful starting points for browsing IPSuite:
//  - the <a href="full-usage-diagram.html">Usage Diagram</a> 
//    includes all modules and networks
//  - example models such as TCPTester, ClientServerNetwork, McNetwork2 or 
//    Peer2Network
//  - protocol implementations: IPProcessing, TcpModule, ...
//
// Starting points for the MPLS models:
//  - the core modules: MPLSModule, LIBTable, LDPproc, RSVP, TED, OspfTe
//  - MPLSNet network model
//  - <a href="standards.html">list of implemented standards</a>
// 
// Related documentation:
//   - <a href="../doxy/index.html" target="_top">C++ source documentation (Doxygen)</a>
//

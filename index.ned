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
// <center><h3>2004-08-12</h3></center>
//
// <nohtml>IPSuite contains IPv4, TCP, UDP models and support for QoS.
// It also includes an MPLS model with RSVP-TE and LDP signalling.
// Link-layer models are PPP and Ethernet.</nohtml>
//
// The original IPSuite was originally written by Klaus Wehrle, Jochen Reber,
// Dirk Holzhausen, Volker Boehm, Verena Kahmann, Ulrich Kaage and others at
// the University of Karlsruhe during 2000-2001. The current codebase is a
// rewrite by Andras Varga, started in 2003 and still in progress.
//
// The MPLS model builds on top of the IPSuite, and implements the MPLS, LDP,
// and <nohtml>RSVP-TE</nohtml> protocols. It also implements the routing
// algorithm used in OSPF-TE. Its uses bandwidth as the major resource
// constraint. The software has been built around 10,000 LOCs.
// The model has been written by Xuan Thang Nguyen (Xuan.T.Nguyen@uts.edu.au)
// and other students at the University of Technology, Sydney
// under supervision of Dr Robin Brown.
// See <a href="http://charlie.it.uts.edu.au/~tkaphan/xtn/capstone/"
// target="_top">project page</a> (obsolete).
//
// For up-to-date info, visit the current
// <a href="http://ctieware.eng.monash.edu.au/twiki/bin/view/Simulation/IPv4Suite"
// target="_top">IPSuite home page</a>.
//
// A few useful starting points for browsing IPSuite:
//  - the <a href="full-usage-diagram.html">Usage Diagram</a>
//    includes all modules and networks
//  - example models such as ClientServerNetwork or McNetwork2
//  - hosts and routers: StandardHost2, Router
//  - protocol implementations: IP, IPProcessing, TCPMain, UDPProcessing, etc.
//
// Starting points for the MPLS models:
//  - the core modules: MPLSModule, LIBTable, LDPproc, RSVP, TED, OspfTe
//  - MPLSNet network model
//  - <a href="standards.html">list of implemented standards</a>
//
// Related documentation:
//   - <a href="../doxy/index.html" target="_top">C++ source documentation (Doxygen)</a>
//

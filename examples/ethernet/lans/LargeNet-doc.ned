//
// Copyright (C) 2003 CTIE, Monash University
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
// @page largenet.html, LargeNet model description
//
// The ~LargeNet model demonstrates how one can put together models of large
// LANs with little effort.
//
// ~LargeNet models a large Ethernet campus backbone. As configured in the
// default omnetpp.ini, it contains altogether about 8000 computers
// and 900 switches and hubs. This results in about 165MB process size
// on my (32-bit) linux box when I run the simulation.
// The model mixes all kinds of Ethernet technology: Gigabit Ethernet,
// 100Mb full-duplex, 100Mb half-duplex, 10Mb UTP, 10Mb bus ("thin Ethernet"),
// switched hubs, repeating hubs.
//
// The topology is in LargeNet.ned, and it looks like this: there's chain
// of n=15 large "backbone" switches (switchBB[]) as well as four more
// large switches (switchA, switchB, switchC, switchD) connected to
// somewhere the middle of the backbone (switchBB[4]). These 15+4 switches
// make up the backbone; the n=15 number is configurable in omnetpp.ini.
//
// Then there're several smaller LANs hanging off each backbone switch.
// There're three types of LANs: small, medium and large (represented by
// compound module types ~SmallLAN, ~MediumLAN, ~LargeLAN). A small LAN
// consists of a few computers on a hub (100Mb half-duplex); a medium
// LAN consists of a smaller switch with a hub on one of its port
// (and computers on both); the large one also has a switch and a hub,
// plus an Ethernet bus hanging of one port of the hub (there's still hubs
// around with one BNC connector besides the UTP ones).
// By default there're 5..15 LANs of each type hanging off each backbone
// switch. (These numbers are also omnetpp.ini parameters like the length
// of the backbone.)
//
// The application model which generates load on the simulated LAN is
// simple yet powerful. It can be used as a rough model for any
// request-response based protocol such as SMB/CIFS (the Windows file
// sharing protocol), HTTP, or a database client-server protocol.
//
// Every computer runs a client application (~EtherAppCli) which connects
// to one of the servers. There's one server attached to switches A, B,
// C and D each: serverA, serverB, serverC and serverD -- server selection
// is configured in omnetpp.ini). The servers run ~EtherAppSrv.
// Clients periodically send a request to the server, and the request
// packet contains how many bytes the client wants the server to send back
// (this can mean one or more Ethernet frames, depending on the byte count).
//  Currently the request and reply lengths are configured in omnetpp.ini
// as intuniform(50,1400) and truncnormal(5000,5000).
//
// The volume of the traffic can most easily be controlled with the
// time period between sending requests; this is currently
// set in omnetpp.ini to exponential(0.50) (that is, average 2
// requests per second). This already causes frames to be dropped
// in some of the backbone switches, so the network is a bit
// overloaded with the current settings.
//
// The model generates extensive statistics. All MACs (and most other
// modules too) write statistics into omnetpp.sca at the end
// of the simulation: number of frames sent, received, dropped, etc.
// These are only basic statistics, however it still makes the
// scalar file to be several ten megabytes in size. You can use
// the analysis tools provided with OMNeT++ to visualized the data
// in this file. (If the file size is too big, writing statistics
// can be disabled, by putting **.scalar-recording=false in the ini file.)
// The model can also record output vectors, but this is currently
// disabled in omnetpp.ini because the generated file can easily reach
// gigabyte sizes.
//

package inet.examples.ethernet.lans;


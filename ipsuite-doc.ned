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
// @page irt.html, The routing files
//
// An example routing file (copied here from one of the IPSuite example
// simulations: <tt>McNetwork2/mchost3.mrt</tt>):
//
// <pre>
// ifconfig:
//
// # ethernet card 0 to router
// name: eth0    encap: Ethernet   inet_addr: 172.0.0.3
// Groups: 225.0.0.1:225.0.1.2:225.0.2.1
// MTU: 1500    Metric: 1  BROADCAST MULTICAST
//
// # Point to Point link 1 to Host 1
// name: ppp0    encap: PPP        inet_addr: 172.0.0.4
// MTU: 200 Metric: 1
//
// ifconfigend.
//
// route:
// 172.0.0.2   *           255.255.255.255  H  0   ppp0
// 172.0.0.4   *           255.255.255.255  H  0   ppp0
// default:    10.0.0.13   0.0.0.0          G  0   eth0
//
// 225.0.0.1   *           255.255.255.255  H  0   ppp0
// 225.0.1.2   *           255.255.255.255  H  0   ppp0
// 225.0.2.1   *           255.255.255.255  H  0   ppp0
//
// 225.0.0.0   10.0.0.13   255.0.0.0        G  0   eth0
//
// routeend.
// </pre>
//
// The file consists of two parts: <tt>ifconfig...ifconfigend.</tt>
// which lists the interfaces, and <tt>route..routeend.</tt> which
// contains statically configured routes. The format of these sections
// roughly corresponds to the output of the <tt>ifconfig</tt> and
// <tt>netstat -rn</tt> Unix commands.
//
// Accepted interface fields are:
//    - <tt>name:</tt> - arbitrary interface name (e.g. eth0, ppp0)
//    - <tt>encap:</tt> - (ignored)
//    - <tt>HWaddr:</tt> - (ignored)
//    - <tt>inet_addr:</tt> - IP address
//    - <tt>Bcast:</tt>
//    - <tt>Mask:</tt>
//    - <tt>Groups:</tt> Multicast groups. 224.0.0.1 is added automatically,
//      and 224.0.0.2 also if the node is a router (IPForward==true).
//    - <tt>MTU:</tt> - MTU on the link (e.g. Ethernet: 1500)
//    - <tt>Metric:</tt> - integer route metric
//    - flags: <tt>BROADCAST</tt>, <tt>MULTICAST</tt>, <tt>POINTTOPOINT</tt>
//
// The route fields are:
//
// <pre>
// Destination  Gateway  Netmask  Flags  References Interface
// </pre>
//
// <i>Destination, Gateway</i> and <i>Netmask</i> have the usual meaning.
// The <i>Destination</i> field should either be an IP address or "default:"
// (to designate the default route). For <i>Gateway</i>, <tt>*</tt> is also
// accepted with the meaning <tt>0.0.0.0</tt>.
//
// <i>Flags</i> denotes route type:
//    - <i>G</i> direct route (directly attached to the router), and
//    - <i>H</i> remote route (reached through another router)
//
// The <i>References</i> field is currently ignored.
//
// <i>Interface</i> is the interface name, e.g. <tt>eth0</tt>.
//
//

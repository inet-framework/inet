//
// Copyright (C) 2003 Andras Varga
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

// @page whatsnew.html, What's New
//
// <pre>
// <nohtml>
// @include ../WHATSNEW
// </nohtml>
// </pre>
//
// @page inet-architecture.html, Architecture of the INET Framework
//
// The INET Framework builds upon OMNeT++, and uses the same concept:
// modules communicating by message passing.
//
// <h3>Modules and protocols</h3>
//
// Protocols are represented by simple modules. A simple module's external
// interface (gates [connectors] and parameters) is described in a NED file,
// and the implementation is contained in a C++ class with the same name.
// Some examples: ~TCP, ~IPv4.
//
// These modules can be freely combined to form hosts and other network devices
// with the NED language (no C++ code and no recompilation required).
// Various pre-assembled host, router, switch, access point, etc. models can
// be found in the <code>Nodes/</code> subdirectory (for example: ~StandardHost,
// ~Router), but you can also create your own ones for tailored to your particular
// simulation scenarios.
//
// Network interfaces (Ethernet, 802.11, etc) are usually compound modules (i.e.
// assembled from simple modules) themselves, and are being composed of a queue,
// a MAC, and possibly other simple modules. See ~EthernetInterface as an example.
//
// Not all modules implement protocols though. There are modules which hold
// data (for example ~RoutingTable), facilitate communication of modules
// (~NotificationBoard), perform autoconfiguration of a network
// (~FlatNetworkConfigurator), move a mobile node around (for example
// ~ConstSpeedMobility), and perform housekeeping associated with radio channels
// in wireless simulations (~ChannelControl).
//
// Protocol headers and packet formats are described in message definition
// files (msg files), which are translated into C++ classes by OMNeT++'s
// <i>opp_msgc</i> tool. The generated message classes subclass from OMNeT++'s
// <code>cMessage</code> class.
//
// <h3>About the documentation</h3>
//
// The INET Framework documentation itself is also comprised of two bodies of HTML pages:
// <i>neddoc</i> generated from NED and MSG files using OMNeT++'s <i>opp_neddoc</i> tool,
// and the documentation of the underlying C++ classes, generated from the source files
// using Doxygen.
// The C++ doc is generated in a way that it contains <b>the full C++ source code</b>
// as HTML pages. It is syntax highlighted, and variable and class names are hyperlinked
// and cross-referenced, which makes it convenient for exploring the code.
//
//
// <h3>Common modules in hosts and routers</h3>
//
// There are some common modules that appear in all (or many) host, router and device
// models.
//
// - ~InterfaceTable. This module contains the table of network interfaces
//   (eth0, wlan0, etc) in the host. This module does not send or receive messages:
//   it is accessed by other modules using standard C++ member function calls.
//   Other modules rely on the interface table submodule within the host to be called
//   <code>interfaceTable</code> to be able to find it. (They obtain a <code>cModule *</code>
//   pointer to it, then cast it to <code>InterfaceTable *</code> to be able to call its
//   functions). Network interfaces get dynamically registered (added to the table)
//   by simple modules implementing the network interface, for example ~EtherMAC.
//
// - ~RoutingTable. This module contains the IP (v4) routing table, and heavily relies
//   on ~InterfaceTable for its operation. This module is also accessed from other
//   modules (typically ~IPv4) by calling the public member functions of its C++ class.
//   There are member functions for querying, adding, deleting routes, and
//   finding the best matching route for a given destination IP address.
//   The routing table submodule within the host (router) must be called
//   <code>routingTable</code> for other modules to find it.
//
// - ~RoutingTable6. This is like ~RoutingTable, but for IPv6.
//
// - ~NotificationBoard. This module makes it possible for several modules to
//   communicate in a publish-subscribe manner. For example, the radio module
//   (~Ieee80211Radio) fires a <i>"radio state changed"</i> notification when
//   the state of the radio channel changes (from TRANSMIT to IDLE, for example),
//   and it will be delivered to other modules that have previously subscribed
//   to that notification category. The notification mechanism also works
//   my C++ functions calls, no message sending is involved.
//   The notification board submodule within the host (router) must be called
//   <code>notificationBoard</code> for other modules to find it.
//
//
// <h3>Common modules at network level</h3>
//
// Some modules have only one instance, at global network level:
//
// - ~FlatNetworkConfigurator assigns IP addresses to hosts and routers,
//   and sets up static routing.
//
// - ~ScenarioManager makes simulations scriptable. Modules can be made to support
//   scripting by implementing the <code>IScriptable</code> C++ interface.
//
// - ~ChannelControl is required for wireless simulations. It keeps track of which
//   nodes are within interference distance of other nodes.
//
// <h3>Communication between protocol layers</h3>
//
// In the INET Framework, when an upper-layer protocol wants to send a data
// packet over a lower-layer protocol, the upper-layer module just sends the
// message object representing the packet to the lower-layer module, which
// will in turn encapsulate it and send it. The reverse process takes place
// when a lower layer protocol receives a packet and sends it up after
// decapsulation.
//
// It is often necessary to convey extra information with the packet. For
// example, when an application-layer module wants to send data over TCP, some
// connection identifier needs to be specified for TCP. When TCP sends a
// segment over IP, IP will need a destination address and possibly other
// parameters like TTL. When IP sends a datagram to an Ethernet interface for
// transmission, a destination MAC address must be specified. This extra
// information is attached to the message object to as <i>control info</i>.
//
// Control info are small value objects, which are attached to packets
// (message objects) with its <code>setControlInfo()</code> member function.
// Control info only holds auxiliary information for the next protocol layer,
// and is not supposed to be sent over the network to other hosts and routers.
//
// @page ipaddresses.html, Specifying IP (IPv6) addresses in module parameters
//
// In INET, ~TCP, ~UDP and all application layer modules work with
// both IPv4 and IPv6. Internally they use the IPvXAddress C++ class, which
// can represent both IPv4 and IPv6 addresses.
//
// Most modules use the IPvXAddressResolver C++ class to resolve addresses
// specified in module parameters in omnetpp.ini.
// IPvXAddressResolver accepts the following syntax:
//
//    - literal IPv4 address: "186.54.66.2"
//    - literal IPv6 address: "3011:7cd6:750b:5fd6:aba3:c231:e9f9:6a43"
//    - module name: "server", "subnet.server[3]"
//    - interface of a host or router: "server/eth0", "subnet.server[3]/eth0"
//    - IPv4 or IPv6 address of a host or router: "server(ipv4)",
//      "subnet.server[3](ipv6)"
//    - IPv4 or IPv6 address of an interface of a host or router:
//      "server/eth0(ipv4)", "subnet.server[3]/eth0(ipv6)"
//
// @page irt.html, The IP routing files
//
// Routing files are files with <tt>.irt</tt> or <tt>.mrt</tt> extension,
// and their names are passed in the routingFileName parameter
// to ~RoutingTable modules. RoutingTables are present in all
// IP nodes (hosts and routers).
//
// Routing files may contain network interface configuration and static
// routes. Both are optional. Network interface entries in the file
// configure existing interfaces; static routes are added to the route table.
//
// Interfaces themselves are represented in the simulation by modules
// (such as the ~PPP module). Modules automatically register themselves
// with appropriate defaults in the ~RoutingTable, and entries in the
// routing file refine (overwrite) these settings.
// Interfaces are identified by names (e.g. ppp0, ppp1, eth0) which
// are normally derived from the module's name: a module called
// <tt>"ppp[2]"</tt> in the NED file registers itself as interface ppp2.
//
// An example routing file (copied here from one of the example simulations):
//
// <pre>
// ifconfig:
//
// # ethernet card 0 to router
// name: eth0   inet_addr: 172.0.0.3   MTU: 1500   Metric: 1  BROADCAST MULTICAST
// Groups: 225.0.0.1:225.0.1.2:225.0.2.1
//
// # Point to Point link 1 to Host 1
// name: ppp0   inet_addr: 172.0.0.4   MTU: 576   Metric: 1
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
// The <tt>ifconfig...ifconfigend.</tt> part configures interfaces,
// and <tt>route..routeend.</tt> part contains static routes.
// The format of these sections roughly corresponds to the output
// of the <tt>ifconfig</tt> and <tt>netstat -rn</tt> Unix commands.
//
// An interface entry begins with a <tt>name:</tt> field, and lasts until
// the next <tt>name:</tt> (or until <tt>ifconfigend.</tt>). It may
// be broken into several lines.
//
// Accepted interface fields are:
//    - <tt>name:</tt> - arbitrary interface name (e.g. eth0, ppp0)
//    - <tt>inet_addr:</tt> - IP address
//    - <tt>Mask:</tt> - netmask
//    - <tt>Groups:</tt> Multicast groups. 224.0.0.1 is added automatically,
//      and 224.0.0.2 also if the node is a router (IPForward==true).
//    - <tt>MTU:</tt> - MTU on the link (e.g. Ethernet: 1500)
//    - <tt>Metric:</tt> - integer route metric
//    - flags: <tt>BROADCAST</tt>, <tt>MULTICAST</tt>, <tt>POINTTOPOINT</tt>
//
// The following fields are parsed but ignored: <tt>Bcast</tt>,<tt>encap</tt>,
// <tt>HWaddr</tt>.
//
// Interface modules set a good default for MTU, Metric (as 2e9/bitrate) and
// flags, but leave inet_addr and Mask empty. inet_addr and mask should
// be set either from the routing file or by a dynamic network configuration
// module.
//
// The route fields are:
//
// <pre>
// Destination  Gateway  Netmask  Flags  Metric Interface
// </pre>
//
// <i>Destination, Gateway</i> and <i>Netmask</i> have the usual meaning.
// The <i>Destination</i> field should either be an IP address or "default:"
// (to designate the default route). For <i>Gateway</i>, <tt>*</tt> is also
// accepted with the meaning <tt>0.0.0.0</tt>.
//
// <i>Flags</i> denotes route type:
//    - <i>H</i> "host": direct route (directly attached to the router), and
//    - <i>G</i> "gateway": remote route (reached through another router)
//
// <i>Interface</i> is the interface name, e.g. <tt>eth0</tt>.
//
//
//
// @page ipv6overview.html, IPv6 model overview
//
// IPv6 support is implemented by several cooperating modules. The ~IPv6 module
// implements IPv6 datagram handling (sending, forwarding etc). It relies on
// ~RoutingTable6 to get access to the routes. ~RoutingTable6 also contains the
// neighbour discovery data structures (destination cache, neighbour cache,
// prefix list -- the latter effectively merged into the route table). Interface
// configuration (address, state, timeouts etc) is held in the ~InterfaceTable,
// in <tt>IPv6InterfaceData</tt> objects attached to <tt>InterfaceEntry</tt>
// as its <tt>ipv6()</tt> member.
//
// The ~IPv6NeighbourDiscovery module implements all tasks associated with
// neighbour discovery and stateless address autoconfiguration. The data
// structures themselves (destination cache, neighbour cache, prefix list)
// are kept in ~RoutingTable6, and are accessed via public C++ methods.
// Neighbour discovery packets are only sent and processed by this module --
// when ~IPv6 receives one, it forwards the packet to ~IPv6NeighbourDiscovery.
//
// The rest of ICMPv6 (ICMP errors, echo request/reply etc) is implemented in
// the ~ICMPv6 module, just like with IPv4. ICMP errors are sent to
// ~IPv6ErrorHandling, which the user can extend or replace to have errors
// handled in the way she likes.
//

package inet;

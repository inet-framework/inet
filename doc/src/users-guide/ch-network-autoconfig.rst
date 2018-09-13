.. _ug:cha:network-autoconfiguration:

Network Autoconfiguration
=========================

.. _ug:sec:autoconfig:overview:

Overview
--------

This chapter describes static autoconfiguration of networks.

.. _ug:sec:autoconfig:configuring-ipv4-networks:

Configuring IPv4 Networks
-------------------------

An IPv4 network is composed of several nodes like hosts, routers,
switches, hubs, Ethernet buses, or wireless access points. The nodes
having a IPv4 network layer (hosts and routers) should be configured at
the beginning of the simulation. The configuration assigns IP addresses
to the nodes, and fills their routing tables. If multicast forwarding is
simulated, then the multicast routing tables also must be filled in.

The configuration can be manual (each address and route is fully
specified by the user), or automatic (addresses and routes are generated
by a configurator module at startup).

Before version 1.99.4 INET offered :ned:`Ipv4FlatNetworkConfigurator`
for automatic and routing files for manual configuration. Both had
serious limitations, so a new configurator has been added in version
1.99.4: :ned:`Ipv4NetworkConfigurator`. This configurator supports both
fully manual and fully automatic configuration. It can also be used with
partially specified manual configurations, the configurator fills in the
gaps automatically.

The next section describes the usage of :ned:`Ipv4NetworkConfigurator`.
The legacy solutions :ned:`Ipv4FlatNetworkConfigurator` and routing
files are described in subsequent sections.

.. _ug:sec:autoconfig:ipv4networkconfigurator:

Ipv4NetworkConfigurator
~~~~~~~~~~~~~~~~~~~~~~~

The :ned:`Ipv4NetworkConfigurator` assigns IP addresses and sets up
static routing for an IPv4 network.

It assigns per-interface IP addresses, strives to take subnets into
account, and can also optimize the generated routing tables by merging
routing entries.

Hierarchical routing can be set up by using only a fraction of
configuration entries compared to the number of nodes. The configurator
also does routing table optimization that significantly decreases the
size of routing tables in large networks.

The configuration is performed in stage 2 of the initialization. At this
point interface modules (e.g. PPP) has already registered their
interface in the interface table. If an interface is named
``ppp[0]``, then the corresponding interface entry is named
``ppp0``. This name can be used in the config file to refer to the
interface.

The configurator goes through the following steps:

#. Builds a graph representing the network topology. The graph will have
   a vertex for every module that has a ``@node`` property (this
   includes hosts, routers, and L2 devices like switches, access points,
   Ethernet hubs, etc.) It also assigns weights to vertices and edges
   that will be used by the shortest path algorithm when setting up
   routes. Weights will be infinite for IP nodes that have IP forwarding
   disabled (to prevent routes from transiting them), and zero for all
   other nodes (routers and and L2 devices). Edge weights are chosen to
   be inversely proportional to the bitrate of the link, so that the
   configurator prefers connections with higher bandwidth. For internal
   purposes, the configurator also builds a table of all "links" (the
   link data structure consists of the set of network interfaces that
   are on the same point-to-point link or LAN)

#. Assigns IP addresses to all interfaces of all nodes. The assignment
   process takes into consideration the addresses and netmasks already
   present on the interfaces (possibly set in earlier initialize
   stages), and the configuration provided in the XML format (described
   below). The configuration can specify "templates" for the address and
   netmask, with parts that are fixed and parts that can be chosen by
   the configurator (e.g. "10.0.x.x"). In the most general case, the
   configurator is allowed to choose any address and netmask for all
   interfaces (which results in automatic address assignment). In the
   most constrained case, the configurator is forced to use the
   requested addresses and netmasks for all interfaces (which translates
   to manual address assignment). There are many possible configuration
   options between these two extremums. The configurator assigns
   addresses in a way that maximizes the number of nodes per subnet.
   Once it figures out the nodes that belong to a single subnet it, will
   optimize for allocating the longest possible netmask. The
   configurator might fail to assign netmasks and addresses according to
   the given configuration parameters; if that happens, the assignment
   process stops and an error is signalled.

#. Adds the manual routes that are specified in the configuration.

#. Adds static routes to all routing tables in the network. The
   configurator uses Dijkstra’s weighted shortest path algorithm to find
   the desired routes between all possible node pairs. The resulting
   routing tables will have one entry for all destination interfaces in
   the network. The configurator can be safely instructed to add default
   routes where applicable, significantly reducing the size of the host
   routing tables. It can also add subnet routes instead of interface
   routes further reducing the size of routing tables. Turning on this
   option requires careful design to avoid having IP addresses from the
   same subnet on different links. CAVEAT: Using manual routes and
   static route generation together may have unwanted side effects,
   because route generation ignores manual routes.

#. Then it optimizes the routing tables for size. This optimization
   allows configuring larger networks with smaller memory footprint and
   makes the routing table lookup faster. The resulting routing table
   might be different in that it will route packets that the original
   routing table did not. Nevertheless the following invariant holds:
   any packet routed by the original routing table (has matching route)
   will still be routed the same way by the optimized routing table.

#. Finally it dumps the requested results of the configuration. It can
   dump network topology, assigned IP addresses, routing tables and its
   own configuration format.

The module can dump the result of the configuration in the XML format
which it can read. This is useful to save the result of a time consuming
configuration (large network with optimized routes), and use it as the
config file of subsequent runs.

Network topology graph
^^^^^^^^^^^^^^^^^^^^^^

The network topology graph is constructed from the nodes of the network.
The node is a module having a ``@node`` property (this includes
hosts, routers, and L2 devices like switches, access points, Ethernet
hubs, etc.). An IP node is a node that contains an :ned:`InterfaceTable`
and a :ned:`Ipv4RoutingTable`. A router is an IP node that has multiple
network interfaces, and IP forwarding is enabled in its routing table
module. In multicast routers the :par:`forwardMulticast` parameter is
also set to ``true``.

A link is a set of interfaces that can send datagrams to each other
without intervening routers. Each interface belongs to exactly one link.
For example two interface connected by a point-to-point connection forms
a link. Ethernet interfaces connected via buses, hubs or switches. The
configurator identifies links by discovering the connections between the
IP nodes, buses, hubs, and switches.

Wireless links are identified by the :par:`ssid` or
:par:`accessPointAddress` parameter of the 802.11 management module.
Wireless interfaces whose node does not contain a management module are
supposed to be on the same wireless link. Wireless links can also be
configured in the configuration file of :ned:`Ipv4NetworkConfigurator`:



.. code-block:: xml

   <config>
     <wireless hosts="area1.*" interfaces="wlan*">
   </config>

puts wlan interfaces of the specified hosts into the same wireless link.

If a link contains only one router, it is marked as the gateway of the
link. Each datagram whose destination is outside the link must go
through the gateway.

Address assignment
^^^^^^^^^^^^^^^^^^

Addresses can be set up manually by giving the address and netmask for
each IP node. If some part of the address or netmask is unspecified,
then the configurator can fill them automatically. Unspecified fields
are given as an “x” character in the dotted notation of the address. For
example, if the address is specified as 192.168.1.1 and the netmask is
255.255.255.0, then the node address will be 192.168.1.1 and its subnet
is 192.168.1.0. If it is given as 192.168.x.x and 255.255.x.x, then the
configurator chooses a subnet address in the range of 192.168.0.0 -
192.168.255.252, and an IP address within the chosen subnet. (The
maximum subnet mask is 255.255.255.252 allows 2 nodes in the subnet.)

The following configuration generates network addresses below the
10.0.0.0 address for each link, and assign unique IP addresses to each
host:



.. code-block:: xml

   <config>
     <interface hosts="*" address="10.x.x.x" netmask="255.x.x.x"/>
   </config>

The configurator tries to put nodes on the same link into the same
subnet, so its enough to configure the address of only one node on each
link.

The following example configures a hierarchical network in a way that
keeps routing tables small.



.. code-block:: xml

   <config>
     <interface hosts="area11.lan1.*" address="10.11.1.x" netmask="255.255.255.x"/>
     <interface hosts="area11.lan2.*" address="10.11.2.x" netmask="255.255.255.x"/>
     <interface hosts="area12.lan1.*" address="10.12.1.x" netmask="255.255.255.x"/>
     <interface hosts="area12.lan2.*" address="10.12.2.x" netmask="255.255.255.x"/>
     <interface hosts="area*.router*" address="10.x.x.x" netmask="x.x.x.x"/>
     <interface hosts="*" address="10.x.x.x" netmask="255.x.x.0"/>
   </config>

The XML configuration must contain exactly one ``<config>`` element.
Under the root element there can be multiple of the following elements:

The interface element provides configuration parameters for one or more
interfaces in the network. The selector attributes limit the scope where
the interface element has effects. The parameter attributes limit the
range of assignable addresses and netmasks. The ``<interface>`` element
may contain the following attributes:

-  ``@hosts`` Optional selector attribute that specifies a list of
   host name patterns. Only interfaces in the specified hosts are
   affected. The pattern might be a full path starting from the network,
   or a module name anywhere in the hierarchy, and other patterns
   similar to ini file keys. The default value is "*" that matches all
   hosts. e.g. "subnet.client*" or "host\* router[0..3]" or
   "area*.*.host[0]"

-  ``@names`` Optional selector attribute that specifies a list of
   interface name patterns. Only interfaces with the specified names are
   affected. The default value is "*" that matches all interfaces. e.g.
   "eth\* ppp0" or "*"

-  ``@towards`` Optional selector attribute that specifies a list of
   host name patterns. Only interfaces connected towards the specified
   hosts are affected. The specified name will be matched against the
   names of hosts that are on the same LAN with the one that is being
   configured. This works even if there’s a switch between the
   configured host and the one specified here. For wired networks it
   might be easier to specify this parameter instead of specifying the
   interface names. The default value is "*". e.g. "ap" or "server" or
   "client*"

-  ``@among`` Optional selector attribute that specifies a list of
   host name patterns. Only interfaces in the specified hosts connected
   towards the specified hosts are affected. The ’among="X Y Z"’ is same
   as ’hosts="X Y Z" towards="X Y Z"’.

-  ``@address`` Optional parameter attribute that limits the range of
   assignable addresses. Wildcards are allowed with using ’x’ as part of
   the address in place of a byte. Unspecified parts will be filled
   automatically by the configurator. The default value "" means that
   the address will not be configured. Unconfigured interfaces still
   have allocated addresses in their subnets allowing them to become
   configured later very easily. e.g. "192.168.1.1" or "10.0.x.x"

-  ``@netmask`` Optional parameter attribute that limits the range of
   assignable netmasks. Wildcards are allowed with using ’x’ as part of
   the netmask in place of a byte. Unspecified parts will be filled
   automatically be the configurator. The default value "" means that
   any netmask can be configured. e.g. "255.255.255.0" or "255.255.x.x"
   or "255.255.x.0"

-  ``@mtu`` number Optional parameter attribute to set the MTU
   parameter in the interface. When unspecified the interface parameter
   is left unchanged.

-  ``@metric`` number Optional parameter attribute to set the Metric
   parameter in the interface. When unspecified the interface parameter
   is left unchanged.

Wireless interfaces can similarly be configured by adding ``<wireless>``
elements to the configuration. Each ``<wireless>`` element with a
different id defines a separate subnet.

-  ``@id`` (optional) identifies wireless network, unique value used
   if missed

-  ``@hosts`` Optional selector attribute that specifies a list of
   host name patterns. Only interfaces in the specified hosts are
   affected. The default value is "*" that matches all hosts.

-  ``@interfaces`` Optional selector attribute that specifies a list
   of interface name patterns. Only interfaces with the specified names
   are affected. The default value is "*" that matches all interfaces.

.. _ug:sec:autoconfig:multicast-groups:

Multicast groups
^^^^^^^^^^^^^^^^

Multicast groups can be configured by adding ``<multicast-group>``
elements to the configuration file. Interfaces belongs to a multicast
group will join to the group automatically.

For example,



.. code-block:: xml

   <config>
     <multicast-group hosts="router*" interfaces="eth*" address="224.0.0.5"/>
   </config>

adds all Ethernet interfaces of nodes whose name starts with “router” to
the 224.0.0.5 multicast group.

The ``<multicast-group>`` element has the following attributes:

-  ``@hosts`` Optional selector attribute that specifies a list of
   host name patterns. Only interfaces in the specified hosts are
   affected. The default value is "*" that matches all hosts.

-  ``@interfaces`` Optional selector attribute that specifies a list
   of interface name patterns. Only interfaces with the specified names
   are affected. The default value is "*" that matches all interfaces.

-  ``@towards`` Optional selector attribute that specifies a list of
   host name patterns. Only interfaces connected towards the specified
   hosts are affected. The default value is "*".

-  ``@among`` Optional selector attribute that specifies a list of
   host name patterns. Only interfaces in the specified hosts connected
   towards the specified hosts are affected. The ’among="X Y Z"’ is same
   as ’hosts="X Y Z" towards="X Y Z"’.

-  ``@address`` Mandatory parameter attribute that specifies a list
   of multicast group addresses to be assigned. Values must be selected
   from the valid range of multicast addresses. e.g. "224.0.0.1
   224.0.1.33"

Manual route configuration
^^^^^^^^^^^^^^^^^^^^^^^^^^

The :ned:`Ipv4NetworkConfigurator` module allows the user to fully
specify the routing tables of IP nodes at the beginning of the
simulation.

The ``<route>`` elements of the configuration add a route to the routing
tables of selected nodes. The element has the following attributes:

-  ``@hosts`` Optional selector attribute that specifies a list of
   host name patterns. Only routing tables in the specified hosts are
   affected. The default value "" means all hosts will be affected. e.g.
   "host\* router[0..3]"

-  ``@destination`` Optional parameter attribute that specifies the
   destination address in the route (L3AddressResolver syntax). The
   default value is "*". e.g. "192.168.1.1" or "subnet.client[3]" or
   "subnet.server(ipv4)" or "*"

-  ``@netmask`` Optional parameter attribute that specifies the
   netmask in the route. The default value is "*". e.g. "255.255.255.0"
   or "/29" or "*"

-  ``@gateway`` Optional parameter attribute that specifies the
   gateway (next-hop) address in the route (L3AddressResolver syntax).
   When unspecified the interface parameter must be specified. The
   default value is "*". e.g. "192.168.1.254" or "subnet.router" or "*"

-  ``@interface`` Optional parameter attribute that specifies the
   output interface name in the route. When unspecified the gateway
   parameter must be specified. This parameter has no default value.
   e.g. "eth0"

-  ``@metric`` Optional parameter attribute that specifies the metric
   in the route. The default value is 0.

Multicast routing tables can similarly be configured by adding
``<multicast-route>`` elements to the configuration.

-  ``@hosts`` Optional selector attribute that specifies a list of
   host name patterns. Only routing tables in the specified hosts are
   affected. e.g. "host\* router[0..3]"

-  ``@source`` Optional parameter attribute that specifies the
   address of the source network. The default value is "*" that matches
   all sources.

-  ``@netmask`` Optional parameter attribute that specifies the
   netmask of the source network. The default value is "*" that matches
   all sources.

-  ``@groups`` Optional List of IPv4 multicast addresses specifying
   the groups this entry applies to. The default value is "*" that
   matches all multicast groups. e.g. "225.0.0.1 225.0.1.2".

-  ``@metric`` Optional parameter attribute that specifies the metric
   in the route.

-  ``@parent`` Optional parameter attribute that specifies the name
   of the interface the multicast datagrams are expected to arrive. When
   a datagram arrives on the parent interface, it will be forwarded
   towards the child interfaces; otherwise it will be dropped. The
   default value is the interface on the shortest path towards the
   source of the datagram.

-  ``@children`` Mandatory parameter attribute that specifies a list
   of interface name patterns:

   -  a name pattern (e.g. "ppp*") matches the name of the interface

   -  a ’towards’ pattern (starting with ">", e.g. ">router*") matches
      the interface by naming one of the neighbour nodes on its link.

   Incoming multicast datagrams are forwarded to each child interface
   except the one they arrived in.

The following example adds an entry to the multicast routing table of
``router1``, that intsructs the routing algorithm to forward
multicast datagrams whose source is in the 10.0.1.0 network and whose
destinatation address is 225.0.0.1 to send on the ``eth1`` and
``eth2`` interfaces assuming it arrived on the ``eth0`` interface:



.. code-block:: xml

   <multicast-route hosts="router1" source="10.0.1.0" netmask="255.255.255.0"
                    groups="225.0.0.1" metric="10"
                    parent="eth0" children="eth1 eth2"/>

Automatic route configuration
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

If the :par:`addStaticRoutes` parameter is true, then the configurator
add static routes to all routing tables.

The configurator uses Dijkstra’s weighted shortest path algorithm to
find the desired routes between all possible node pairs. The resulting
routing tables will have one entry for all destination interfaces in the
network.

The configurator can be safely instructed to add default routes where
applicable, significantly reducing the size of the host routing tables.
It can also add subnet routes instead of interface routes further
reducing the size of routing tables. Turning on this option requires
careful design to avoid having IP addresses from the same subnet on
different links.



.. caution::

   Using manual routes and static route generation
   together may have unwanted side effects, because route generation ignores
   manual routes. Therefore if the configuration file contains
   manual routes, then the :par:`addStaticRoutes` parameter should be set
   to ``false``.

Route optimization
^^^^^^^^^^^^^^^^^^

If the :par:`optimizeRoutes` parameter is ``true`` then the
configurator tries to optimize the routing table for size. This
optimization allows configuring larger networks with smaller memory
footprint and makes the routing table lookup faster.

The optimization is performed by merging routes whose gateway and
outgoing interface is the same by finding a common prefix that matches
only those routes. The resulting routing table might be different in
that it will route packets that the original routing table did not.
Nevertheless the following invariant holds: any packet routed by the
original routing table (has matching route) will still be routed the
same way by the optimized routing table.

Parameters
^^^^^^^^^^

This list summarize the parameters of the :ned:`Ipv4NetworkConfigurator`:

-  :par:`config`: XML configuration parameters for IP address assignment
   and adding manual routes.

-  :par:`assignAddresses`: assign IP addresses to all interfaces in the
   network

-  :par:`assignDisjunctSubnetAddresses`: avoid using the same address
   prefix and netmask on different links when assigning IP addresses to
   interfaces

-  :par:`addStaticRoutes`: add static routes to the routing tables of
   all nodes to route to all destination interfaces (only where
   applicable; turn off when config file contains manual routes)

-  :par:`addDefaultRoutes`: add default routes if all routes from a
   source node go through the same gateway (used only if addStaticRoutes
   is true)

-  :par:`addSubnetRoutes`: add subnet routes instead of destination
   interface routes (only where applicable; used only if addStaticRoutes
   is true)

-  :par:`optimizeRoutes`: optimize routing tables by merging routes, the
   resulting routing table might route more packets than the original
   (used only if addStaticRoutes is true)

-  :par:`dumpTopology`: if true, then the module prints extracted
   network topology

-  :par:`dumpAddresses`: if true, then the module prints assigned IP
   addresses for all interfaces

-  :par:`dumpRoutes`: if true, then the module prints configured and
   optimized routing tables for all nodes to the module output

-  :par:`dumpConfig`: name of the file, write configuration into the
   given config file that can be fed back to speed up subsequent runs
   (network configurations)

.. _ug:sec:autoconfig:ipv4flatnetworkconfigurator:

Ipv4FlatNetworkConfigurator (Legacy)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The :ned:`Ipv4FlatNetworkConfigurator` module configures IP addresses
and routes of IP nodes of a network. All assigned addresses share a
common subnet prefix, the network topology will be ignored. Shortest
path routes are also generated from any node to any other node of the
network. The Gateway (next hop) field of the routes is not filled in by
these configurator, so it relies on proxy ARP if the network spans
several LANs. It does not perform routing table optimization (i.e.
merging similar routes into a single, more general route.)



.. warning::

   :ned:`Ipv4FlatNetworkConfigurator` is considered
   legacy, do not use it for new projects.

The :ned:`Ipv4FlatNetworkConfigurator` module configures the network
when it is initialized. The configuration is performed in stage 2, after
interface tables are filled in. Do not use a
:ned:`Ipv4FlatNetworkConfigurator` module together with static routing
files, because they can iterfere with the configurator.

The :ned:`Ipv4FlatNetworkConfigurator` searches each IP nodes of the
network. (IP nodes are those modules that have the @node NED property
and has a :ned:`Ipv4RoutingTable` submodule named “routingTable”). The
configurator then assigns IP addresses to the IP nodes, controlled by
the following module parameters:

-  :par:`netmask` common netmask of the addresses (default is
   255.255.0.0)

-  :par:`networkAddress` higher bits are the network part of the
   addresses, lower bits should be 0. (default is 192.168.0.0)

With the default parameters the assigned addresses are in the range
192.168.0.1 - 192.168.255.254, so there can be maximum 65534 nodes in
the network. The same IP address will be assigned to each interface of
the node, except the loopback interface which always has address
127.0.0.1 (with 255.0.0.0 mask).

After assigning the IP addresses, the configurator fills in the routing
tables. There are two kind of routes:

-  default routes: for nodes that has only one non-loopback interface a
   route is added that matches with any destination address (the entry
   has 0.0.0.0 ``host`` and ``netmask`` fields). These are remote
   routes, but the gateway address is left unspecified. The delivery of
   the datagrams rely on the proxy ARP feature of the routers.

-  direct routes following the shortest paths: for nodes that has more
   than one non-loopback interface a separate route is added to each IP
   node of the network. The outgoing interface is chosen by the shortest
   path to the target node. These routes are added as direct routes,
   even if there is no direct link with the destination. In this case
   proxy ARP is needed to deliver the datagrams.



.. note::

   This configurator does not try to optimize the routing tables.
   If the network contains $n$ nodes, the size of all routing tables
   will be proportional to $n^2$, and the time of the lookup of the
   best matching route will be proportional to $n$.

.. _ug:sec:autoconfig:routing-files:

Routing Files (Legacy)
~~~~~~~~~~~~~~~~~~~~~~

Routing files are files with ``.irt`` or ``.mrt`` extension, and
their names are passed in the :par:`routingFile` parameter to
:ned:`Ipv4RoutingTable` modules.

Routing files may contain network interface configuration and static
routes. Both are optional. Network interface entries in the file
configure existing interfaces; static routes are added to the route
table.



.. warning::

   *Routing files* are considered legacy, use do not use them for new
   projects. Their contents can be expressed in :ned:`Ipv4NetworkConfigurator`
   config files.

Interfaces themselves are represented in the simulation by modules (such
as the PPP module). Modules automatically register themselves with
appropriate defaults in the IPv4RoutingTable, and entries in the routing
file refine (overwrite) these settings. Interfaces are identified by
names (e.g. ppp0, ppp1, eth0) which are normally derived from the
module’s name: a module called ``"ppp[2]"`` in the NED file registers
itself as interface ppp2.

An example routing file (copied here from one of the example
simulations):



::

   ifconfig:

   # ethernet card 0 to router
   name: eth0   inet_addr: 172.0.0.3   MTU: 1500   Metric: 1  BROADCAST MULTICAST
   Groups: 225.0.0.1:225.0.1.2:225.0.2.1

   # Point to Point link 1 to Host 1
   name: ppp0   inet_addr: 172.0.0.4   MTU: 576   Metric: 1

   ifconfigend.

   route:
   172.0.0.2   *           255.255.255.255  H  0   ppp0
   172.0.0.4   *           255.255.255.255  H  0   ppp0
   default:    10.0.0.13   0.0.0.0          G  0   eth0

   225.0.0.1   *           255.255.255.255  H  0   ppp0
   225.0.1.2   *           255.255.255.255  H  0   ppp0
   225.0.2.1   *           255.255.255.255  H  0   ppp0

   225.0.0.0   10.0.0.13   255.0.0.0        G  0   eth0

   routeend.

The ``ifconfig...ifconfigend.`` part configures interfaces, and
``route..routeend.`` part contains static routes. The format of these
sections roughly corresponds to the output of the ``ifconfig`` and
``netstat -rn`` Unix commands.

An interface entry begins with a ``name:`` field, and lasts until the
next ``name:`` (or until ``ifconfigend.``). It may be broken into
several lines.

Accepted interface fields are:

-  ``name:`` - arbitrary interface name (e.g. eth0, ppp0)

-  ``inet_addr:`` - IP address

-  ``Mask:`` - netmask

-  ``Groups:`` Multicast groups. 224.0.0.1 is added automatically,
   and 224.0.0.2 also if the node is a router (IPForward==true).

-  ``MTU:`` - MTU on the link (e.g. Ethernet: 1500)

-  ``Metric:`` - integer route metric

-  flags: ``BROADCAST``, ``MULTICAST``, ``POINTTOPOINT``

The following fields are parsed but ignored: ``Bcast``, ``encap``,
``HWaddr``.

Interface modules set a good default for MTU, Metric (as
:math:`2*10^9`/bitrate) and flags, but leave :var:`inet_addr` and
:var:`Mask` empty. :var:`inet_addr` and :var:`mask` should be set either
from the routing file or by a dynamic network configuration module.

The route fields are:



::

   Destination  Gateway  Netmask  Flags  Metric Interface

:var:`Destination`, :var:`Gateway` and :var:`Netmask` have the usual
meaning. The :var:`Destination` field should either be an IP address or
“default” (to designate the default route). For :var:`Gateway`, ``*``
is also accepted with the meaning ``0.0.0.0``.

:var:`Flags` denotes route type:

-  *H* “host”: direct route (directly attached to the router), and

-  *G* “gateway”: remote route (reached through another router)

:var:`Interface` is the interface name, e.g. ``eth0``.



.. important::

   The meaning of the routes where the destination is a multicast address
   has been changed in version 1.99.4. Earlier these entries was used
   both to select the outgoing interfaces of multicast datagrams
   sent by the higher layer (if multicast interface was otherwise unspecified)
   and to select the outgoing interfaces of datagrams that are received from
   the network and forwarded by the node.

   From version 1.99.4 multicast routing applies reverse path forwarding.
   This requires a separate routing table, that can not be populated from
   the old routing table entries. Therefore simulations that use multicast
   forwarding can not use the old configuration files, they should be
   migrated to use an :ned:`Ipv4NetworkConfigurator` instead.

   Some change is needed in models that use link-local multicast too.
   Earlier if the IP module received a datagram from the higher layer
   and multiple routes was given for the multicast group,
   then IP sent a copy of the datagram on each interface of that routes.
   From version 1.99.4, only the first matching interface is used (considering
   longest match). If the application wants to send the multicast datagram
   on each interface, then it must explicitly loop and specify the multicast
   interface.

.. _ug:sec:autoconfig:configuring-layer-2:

Configuring Layer 2
-------------------

The :ned:`L2NetworkConfigurator` module allows configuring network scenarios at
layer 2. The STP/RTP-related parameters such as link cost, port priority
and the “is-edge” flag can be configured with XML files.

This module is similar to :ned:`Ipv4NetworkConfigurator`. It supports
the selector attributes ``@hosts``, ``@names``, ``@towards``,
``@among``, and they behave similarly to its
:ned:`Ipv4NetworkConfigurator` equivalent. The ``@ports`` selector is
also supported, for configuring per-port parameters.

The following example configures port 5 (if it exists) on all switches,
and sets cost=19 and priority=32768:



.. code-block:: xml

   <config>
     <interface hosts='**' ports='5' cost='19' priority='32768'/>
   </config>

For more information about the usage of the selector attributes see
:ned:`Ipv4NetworkConfigurator`.

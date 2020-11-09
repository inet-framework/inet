.. _ug:cha:networks:

Networks
========

.. _ug:sec:networks:overview:

Overview
--------

INET heavily builds upon the modular architecture of OMNeT++. It
provides numerous domain specific and highly parameterizable components
which can be combined in many ways. The primary means of building large
custom network simulations in INET is the composition of existing models
with custom models, starting from small components and gradually forming
ever larger ones up until the composition of the network. Users are not
required to have programming experience to create simulations unless
they also want to implement their own protocols, for example.

Assembling an INET simulation starts with defining a module representing
the network. Networks are compound modules which contain network nodes,
automatic network configurators, and sometimes additionally transmission
medium, physical environment, various visualizer, and other
infrastructure related modules. Networks also contain connections
between network nodes representing cables. Large hierarchical networks
may be further organized into compound modules to directly express the
hierarchy.

There are no predefined networks in INET, because it is very easy to
create one, and because of the vast possibilities. However, the OMNeT++
IDE provides several topology generator wizards for advanced scenarios.

As INET is an OMNeT++-based framework, users mainly use NED to describe
the model topology, and ini files to provide configuration. [1]_

.. _ug:sec:networks:built-in-network-nodes-and-other-top-level-modules:

Built-in Network Nodes and Other Top-Level Modules
--------------------------------------------------

INET provides several pre-assembled network nodes with carefully
selected components. They support customization via parameters and
parametric submodule types, but they are not meant to be universal.
Sometimes it may be necessary to create special network node models for
particular simulation scenarios. In any case, the following list gives a
taste of the built-in network nodes.

-  :ned:`StandardHost` contains the most common Internet protocols:
   :protocol:`UDP`, :protocol:`TCP`, :protocol:`IPv4`, :protocol:`IPv6`,
   :protocol:`Ethernet`, :protocol:`IEEE 802.11`. It also supports an
   optional mobility model, optional energy models, and any number of
   applications which are entirely configurable from INI files.

-  :ned:`EthernetSwitch` models an :protocol:`Ethernet` switch containing a
   relay unit and one MAC unit per port.

-  :ned:`Router` provides the most common routing protocols:
   :protocol:`OSPF`, :protocol:`BGP`, :protocol:`RIP`, :protocol:`PIM`.

-  :ned:`AccessPoint` models a Wifi access point with multiple
   :protocol:`IEEE 802.11` network interfaces and multiple
   :protocol:`Ethernet` ports.

-  :ned:`WirelessHost` provides a network node with one (default)
   :protocol:`IEEE 802.11` network interface in infrastructure mode,
   suitable for using with an :ned:`AccessPoint`.

-  :ned:`AdhocHost` is a :ned:`WirelessHost` with the network interface
   configured in ad-hoc mode and forwarding enabled.

Network nodes communicate at the network level by exchanging OMNeT++
messages which are the abstract representations of physical signals on
the transmission medium. Signals are either sent through OMNeT++
connections in the wired case, or sent directly to the gate of the
receiving network node in the wireless case. Signals encapsulate
INET-specific packets that represent the transmitted digital data.
Packets are further divided into chunks that provide alternative
representations for smaller pieces of data (e.g. protocol headers,
application data).

Additionally, there are components that occur on network level, but they
are not models of physical network nodes. They are necessary to model
other aspects. Some of them are:

-  A *radio medium* module such as :ned:`Ieee80211RadioMedium`,
   :ned:`ApskScalarRadioMedium` and :ned:`UnitDiskRadioMedium` (there
   are a few of them) are a required component of wireless networks.

-  :ned:`PhysicalEnvironment` models the effect of the physical
   environment (i.e. obstacles) on radio signal propagation. It is an
   optional component.

-  *Configurators* such as :ned:`Ipv4NetworkConfigurator`,
   :ned:`L2NetworkConfigurator` and :ned:`NextHopNetworkConfigurator`
   configure various aspects of the network. For example,
   :ned:`Ipv4NetworkConfigurator` assigns IP addresses to hosts and
   routers, and sets up static routing. It is used when modeling dynamic
   IP address assignment (e.g. via DHCP) or dynamic routing is not of
   importance. :ned:`L2NetworkConfigurator` allows one to configure
   802.1 LANs and provide STP/RSTP-related parameters such as link cost,
   port priority and the “is-edge” flag.

-  :ned:`ScenarioManager` allows scripted scenarios, such as timed
   failure and recovery of network nodes.

-  *Group coordinators* are needed for the operation of some group
   mobility mdels. For example, :ned:`MoBanCoordinator` is the
   coordinator module for the MoBAN mobility model.

-  *Visualizers* like :ned:`PacketDropOsgVisualizer` provide graphical
   rendering of some aspect of the simulation either in 2D (canvas) or
   3D (using OSG or osgEarth). The usual choice is
   :ned:`IntegratedVisualizer` which bundles together an instance of
   each specific visualizer type in a compound module.

.. _ug:sec:networks:typical-networks:

Typical Networks
----------------

.. _ug:sec:networks:wired-networks:

Wired Networks
~~~~~~~~~~~~~~

Wired network connections, for example :protocol:`Ethernet` cables, are
represented with standard OMNeT++ connections using the
:ned:`DatarateChannel` NED type. The channel’s :par:`datarate` and
:par:`delay` parameters must be provided for all wired connections.
The number of wired interfaces in a host (or router) usually does
not need to be manually configured, because it can be automatically
inferred from the actual number of links to neighbor nodes.

The following example shows how straightforward it is to create a model
for a simple wired network. This network contains a server connected to
a router using :protocol:`PPP`, which in turn is connected to a switch
using :protocol:`Ethernet`. The network also contains a parameterizable
number of clients, all connected to the switch forming a star topology.
The utilized network nodes are all predefined modules in INET. To avoid
the manual configuration of IP addresses and routing tables, an
automatic network configurator is also included.

.. literalinclude:: lib/Snippets.ned
   :language: ned
   :start-after: !WiredNetworkExample
   :end-before: !End
   :name: Wired network example

In order to run a simulation using the above network, an OMNeT++ INI
file must be created. The INI file selects the network, sets its number
of clients parameter, and configures a simple :protocol:`TCP`
application for each client. The server is configured to have a
:protocol:`TCP` application which echos back all data received from the
clients individually.

.. literalinclude:: lib/Snippets.ini
   :language: ini
   :start-after: !WiredNetworkConfigurationExample
   :end-before: !End
   :name: Wired network configuration example

When the above simulation is run, each client application connects to
the server using a :protocol:`TCP` socket. Then each one of them sends
1MB of data, which in turn is echoed back by the server, and the
simulation concludes. The default statistics are written to the
:file:`results` folder of the simulation for later analysis.

.. _ug:sec:networks:wireless-networks:

Wireless Networks
~~~~~~~~~~~~~~~~~

Wireless network connections are not modeled with OMNeT++ connections
due the dynamically changing nature of connectivity. For wireless
networks, an additional module, one that represents the transmission
medium, is required to maintain connectivity information.

.. literalinclude:: lib/Snippets.ned
   :language: ned
   :start-after: !WirelessNetworkExample
   :end-before: !End
   :name: Wireless network example

In the above network, positions in the display strings provide positions
for the transmission medium during the computation of signal propagation
and path loss.

In addition, ``host1`` is configured to periodically send
:protocol:`UDP` packets to ``host2`` over the AP.

.. literalinclude:: lib/Snippets.ini
   :language: ini
   :start-after: !WirelessNetworkConfigurationExample
   :end-before: !End
   :name: Wireless network configuration example

.. _ug:sec:networks:mobile-ad-hoc-networks:

Mobile Ad hoc Networks
~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: lib/Snippets.ned
   :language: ned
   :start-after: !MobileAdhocNetworkExample
   :end-before: !End
   :name: Mobile ad hoc network example

.. literalinclude:: lib/Snippets.ini
   :language: ini
   :start-after: !MobileAdhocNetworkConfigurationExample
   :end-before: !End
   :name: Mobile ad hoc network configuration example

.. _ug:sec:networks:frequent-tasks:

Frequent Tasks (How To...)
--------------------------

This section contains quick and somewhat superficial advice to many practical tasks.

.. _ug:sec:networks:automatic-wired-interfaces:

Automatic Wired Interfaces
~~~~~~~~~~~~~~~~~~~~~~~~~~

In many wired network simulations, the number of wired interfaces need
not be manually configured, because it can be automatically inferred
from the actual number of connections between network nodes.

.. literalinclude:: lib/Snippets.ned
   :language: ned
   :start-after: !AutomaticWiredInterfacesExample
   :end-before: !End
   :name: Automatic wired interfaces example
   :dedent: 8

.. _ug:sec:networks:multiple-wireless-interfaces:

Multiple Wireless Interfaces
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

All built-in wireless network nodes support multiple wireless
interfaces, but only one is enabled by default.

.. literalinclude:: lib/Snippets.ini
   :language: ini
   :start-after: !MultipleWirelessInterfacesExample
   :end-before: !End
   :name: Multiple wireless interfaces example

.. _ug:sec:networks:specifying-addresses:

Specifying Addresses
~~~~~~~~~~~~~~~~~~~~

Nearly all application layer modules, but several other components as
well, have parameters that specify network addresses. They typically
accept addresses given with any of the following syntax variations:

-  literal IPv4 address: ``"186.54.66.2"``

-  literal IPv6 address:
   ``"3011:7cd6:750b:5fd6:aba3:c231:e9f9:6a43"``

-  module name: ``"server"``, ``"subnet.server[3]"``

-  interface of a host or router: ``"server/eth0"``,
   ``"subnet.server[3]/eth0"``

-  IPv4 or IPv6 address of a host or router: ``"server(ipv4)"``,
   ``"subnet.server[3](ipv6)"``

-  IPv4 or IPv6 address of an interface of a host or router:
   ``"server/eth0(ipv4)"``, ``"subnet.server[3]/eth0(ipv6)"``

.. _ug:sec:networks:node-failure-and-recovery:

Node Failure and Recovery
~~~~~~~~~~~~~~~~~~~~~~~~~

.. _ug:sec:networks:enabling-dual-ip-stack:

Enabling Dual IP Stack
~~~~~~~~~~~~~~~~~~~~~~

All built-in network nodes support dual Internet protocol stacks, that
is both :protocol:`IPv4` and :protocol:`IPv6` are available. They are
also supported by transport layer protocols, link layer protocols, and
most applications. Only :protocol:`IPv4` is enabled by default, so in
order to use :protocol:`IPv6`, it must be enabled first, and an
application supporting :protocol:`IPv6` (e.g., :ned:`PingApp` must be
used). The following example shows how to configure two ping
applications in a single node where one is using an :protocol:`IPv4` and
the other is using an :protocol:`IPv6` destination address.

.. literalinclude:: lib/Snippets.ini
   :language: ini
   :start-after: !DualStackExample
   :end-before: !End
   :name: Dual stack example

.. _ug:sec:networks:enabling-packet-forwarding:

Enabling Packet Forwarding
~~~~~~~~~~~~~~~~~~~~~~~~~~

In general, network nodes don’t forward packets by default, only
:ned:`Router` and the like do. Nevertheless, it’s possible to enable
packet forwarding as simply as flipping a switch.

.. literalinclude:: lib/Snippets.ini
   :language: ini
   :start-after: !ForwardingExample
   :end-before: !End
   :name: Forwarding example

.. [1]
   Some components require additional configuration to be provided as
   separate files, e.g. in XML.

Interconnecting Ethernet Networks
=================================

Goals
-----

Ethernet is the most widely installed local area network (LAN)
technology. It could seem to take an enormous amount of time and effort
to create a network of interconnected Ethernet LANs. However, INET
features various models with different topologies and technologies for
simulating Ethernet networks, and a simple way to connect them. These
include Ethernet LAN models ranging from the simplest ones, like two
hosts directly connected to each other via twisted pair, to more complex
ones using mixed connection types and a higher number of hosts.

This showcase presents the :ned:`LargeNet` model, which demonstrates how
one can fit models of LANs together with little effort, and how it can
be used for network analysis.

INET version: ``4.0``
Source files location: `inet/showcases/general/ethernet <https://github.com/inet-framework/inet-showcases/tree/master/general/ethernet/lans>`__

About Ethernet
--------------

Ethernet is a family of computer networking technologies commonly used
in local area networks (LAN), metropolitan area networks (MAN) and wide
are networks (WAN). Systems communicating over Ethernet divide a stream
of data into shorte pieces called frames. Each frame contains source and
destination addresses, and error-checking data so that damaged frames
can be detected and discarded. As per the OSI model, Ethernet provides
services up to and including the data link layer.

About LAN
---------

A local area network (LAN) is a computer network that interconnects
computers within a limited area such as a residence, school, laboratory,
university campus or office building. By contrast, a wide area network
(WAN) not only covers a larger geographic distance, but also generally
involves leased telecommunication circuits. Ethernet and Wi-Fi are the
two most common technologies in use for local area networks.

A campus network is a proprietary local area network (LAN) or set of
interconnected LANs serving a corporation, government agency,
university, or similar organization. The end users in a campus network
may be dispersed more widely (in a geographical sense) than in a single
LAN, but they are usually not as scattered as they would be in a wide
area network (WAN).

About MAC sublayer
------------------

This showcase demonstrates how one can put together models of LANs with
little effort, making use of MAC auto-configuration.

The medium access control (MAC) sublayer and the logical link control
(LLC) sublayer together make up the data link layer. Within the data
link layer, the LLC provides flow control and multiplexing for the
logical link, while the MAC provides flow control and multiplexing for
the transmission medium.

When sending data to another device on the network, the MAC block
encapsulates higher-level frames into frames appropriate for
transmission medium, adds a frame check sequence to identify
transmission errors, and then forwards the data to the physical layer as
soon as the appropriate channel access method permits it. Controlling
when data is sent and when to wait is necessary to avoid congestion and
collisions, especially for topologies with collision domains (bus, ring,
mesh, point-to-multipoint topologies). Additionally, the MAC is also
responsible for compensating for congestion and collision by initiating
retransmission if a jam signal is detected, and/or negotiating slower
transmission rate if necessary. When receiving data from the physical
layer, the MAC block ensures data integrity by verifying the sender's
frame check sequence, and strips off the sender's preamble and padding
passing the data up to higher layers.

The MAC sublayer is especially important in LANs, many of which use a
multiaccess channel as the basis for communication.

The model
---------

The Ethernet model contains a MAC model (``EtherMAC``), LLC model
(``EtherLLC``) as well a bus (:ned:`EtherBus`, for modelling coaxial cable)
and a hub (:ned:`EtherHub`) model. A switch model (:ned:`EtherSwitch`) is also
provided. - :ned:`EtherHost` is a sample node with an Ethernet NIC. -
:ned:`EtherSwitch`, :ned:`EtherBus`, :ned:`EtherHub` model switching hub,
repeating hub and coaxial cable. - basic components of the model:
``EtherMAC``, ``EtherLLC``/:ned:`EtherEncap` module types,
``MACRelayUnit``, ``EtherFrame`` message type, ``MACAddress`` class.

**Note:** *Nowadays almost all Ethernet networks operate using
full-duplex point-to-point connections between hosts and switches. This
means that there are no collisions, and the behaviour of the MAC
component is much simpler than in classic Ethernet that used coaxial
cables and hubs. The INET framework contains two MAC modules for
Ethernet: the ``EtherMACFullDuplex`` is simpler to understand and easier
to extend, because it supports only full-duplex connections. The
``EtherMAC`` module implements the full MAC functionality including
CSMa/showcases/CD, and it can operate both half-duplex and full-duplex mode.*

The network
~~~~~~~~~~~

:ned:`LargeNet` model implements a large Ethernet campus backbone. The
model mixes all kinds of Ethernet technology: Gigabit Ethernet, 100Mb
full duplex, 100Mb half duplex, 10Mb UTP, 10Mb bus, switched hubs and
repeating hubs. There is a chain of "backbone" switches
(``switchBB[*]``) as well as large switches (``switchA``, ``switchB``,
``switchC``, ``switchD``). There is one server attached to switches A,
B, C and D each: ``serverA``, ``serverB``, ``serverC`` and ``serverD``.
Then there are several smaller LANs hanging off each backbone switch.
Three types of LANs are used in the showcase varying in topology,
technology and size: - ``SmallLAN:`` a small LAN consists of a few
computers on a hub (100Mb half duplex). (`SmallLAN
layout <smallLAN_layout.png>`__) - ``MediumLAN:`` a medium LAN consists
of a smaller switch with a hub on one of its port, and computers on both
devices. (`MediumLAN layout <mediumLAN_layout.png>`__) - ``LargeLAN:``
consists of a switch, a hub, and an Ethernet bus connected to one port
of the hub. (`LargeLAN layout <largeLAN_layout.png>`__)

The topology of the :ned:`LargeNet` model can be seen in the
``LargeNet.ned`` file:

.. figure:: LargeNet_layout.png
   :width: 100%
   :align: center

The application model which generates load on the simulated LAN is
simple yet powerful. It can be used as a rough model for any
request-response based protocol such as SMb/showcases/CIFS (the Windows file
sharing protocol), HTTP, or a database client-server protocol.

Every computer runs a client application (``EtherAppCli``) which
connects to one of the servers, while the servers run ``EtherAppSrv``.
Clients periodically send a request to the server, and the request
packet contains how many bytes the client wants the server to send back.

Configuration and behaviour
~~~~~~~~~~~~~~~~~~~~~~~~~~~

As stated above, this showcase demonstrates how easily different
Ethernet LANs can be connected making use of MAC auto-configuration.
After setting up the connections in the ``LargeNet.ned`` file, no
complex configuration is needed in the ``LargeNet.ini`` file:

.. literalinclude:: ../largeNet.ini
   :language: ini
   :start-at: # MAC settings
   :end-before: # queue

**Note:** *INET framework ethernet switches are built from
``IMACRelayUnit`` components. Each relay unit has N input and output
gates for sending/receiving Ethernet frames. The module handles the
mapping between ports and MAC addresses, and forward frames
(``EtherFrame``) to appropriate ports.*

The number of switches and LANs and the number of the hosts in each LAN
is configured by setting the input parameters of the :ned:`LargeNet`
network:

.. literalinclude:: ../largeNet.ini
   :language: ini
   :start-at: number of hosts
   :end-before: request-response

By default, no finite buffer is used in hosts, so MAC contains an
internal queue named ``txQueue`` to queue up packets waiting for
transmission. Conceptually, ``txQueue`` is of infinite size, but for
better diagnostics one can specify a hard limit in the ``txQueueuLimit``
parameter. If this limit is exceeded, the simulation stops with an
error. In this example :ned:`DropTailQueue` is used instead, in order to
observe the drop statistics (as well):

.. literalinclude:: ../largeNet.ini
   :language: ini
   :start-at: # queue
   :end-before: #####

The flow control is managed with ``PAUSE`` frames. These frames contain
a timer value that specifies how long the transmitter should remain
quiet. For allowing a transmitter to resume immediately (if the receiver
becomes uncongested), a ``PAUSE`` frame with a timer value of zero can
be sent.

CRC checks are modeled by the ``bitError`` flag of the packets. Packets
are dropped by the MAC in case of error.

The model (especially with the third configuration) generates extensive
statistics, so it is recommended to keep both scalar- and
vector-recording disabled, and cherry-pick the desired statistics:

.. literalinclude:: ../largeNet.ini
   :language: ini
   :start-at: scalar- and vector-recording disabled
   :end-before: number of hosts

Otherwise the generated file could easily reach gigabyte size.

In this showcase, two statistics are observed in order to demonstrate
how the traffic rate (and congestion rate) at critical parts of the
network changes with the different configurations. These two generated
statistics are the number of dropped packets (``dropPk``) at switches,
and the number of collisions (``collisions``):

.. literalinclude:: ../largeNet.ini
   :language: ini
   :start-at: # statistics
   :end-before: #####

**Note:** *The ``collision`` signal is an extra signal generated by the
``EtherMAC`` module. Collision occurs, when a frame is received, while
the transmission or reception of another signal is in progress, or when
transmission is started, while receiving a frame is in progress.*

For further examination of the model, let's assume that the goal is to
design a high performance Ethernet campus area network (CAN). A campus
network is a computer network made up of an interconnection of local
area networks (LANs) within a limited geographical area. In real life,
the requirements for performance, capacity and network ports are given
by the client. This example is only concerned about the number of hosts
connected to the network and the performance of the CAN. Three different
simulations are run:

-  ``LargeNet_notOverloaded``: Nearly 500 hosts are connected to the
   network, each performing low data rate. As a result, the network is
   not overloaded, and no packets are dropped.
-  ``LargeNet_overloadedBySendIntervall``: Nearly 500 hosts are
   connected to the network, each performing high data rate. As a
   result, the network is overloaded, and packets are dropped.
-  ``LargeNet_overloadedByNumberOfHosts``: Nearly 7500 hosts are
   connected to the network, each performing low data rate. As a result,
   the network is overloaded, and packets are dropped.

Results
-------

LargeNet_notOverloaded
~~~~~~~~~~~~~~~~~~~~~~

As expected, the 500 hosts, producing an average of 2 requests in a
second, can not consume the network's capacity. As the statistic shows,
no packets are dropped.

The number of collisions with this configuration is also insignificant.
However, we can see that the critical parts of the network from the
point of view of ``collisions`` are the ``largeLAN`` models. This
occurs, because these subnetworks inplement a topology containing bus
and hub, and these networks contain the highest number of hosts.

A network of this topology, technology and configuration (traffic rate,
number of hosts) would be a high performance Ethernet network with no
data-loss and low delay.

LargeNet_overloadedBySendIntervall
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The volume of the traffic can most easily be controlled by changing the
elapsed time between the sending of requests. With this configuration,
the ``sendIntervall`` parameter is set to be much smaller (to an average
of 25 requests per second) than it was in the ``LargeNet_notOverloaded``
configuration:

.. literalinclude:: ../largeNet.ini
   :language: ini
   :start-at: sendInterval -> average 25 requests per sec
   :end-before: #####

As a result, the generated traffic is more than 10 times higher than it
was with the previous configuration. The number of packets dropped by
the :ned:`DropTailQueue` of switches increased dramatically. It is not
surprising that the highest drop rates occur at the connections between
the switches ``switchBB[]``, ``switchB``, ``switchC`` and ``switchD``,
because the majority of the packets follow this route.

The number of collisions conspicuously increased. This phenomena occurs,
because although the number of hosts on the LANs is the same as it was
with the ``LargeNet_notOverloaded`` configuration, the data rate of the
hosts is much higher.

If the estimated traffic of the hosts would be as high as it is set in
this configuration, then this topology would not fit the requirements,
and another topology would need to be chosen.

LargeNet_overloadedByNumberOfHosts
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The other way to control the traffic is to increase or decrease the
number of hosts. With this configuration, there are nearly 7500 hosts
present on the :ned:`LargeNet` network:

.. literalinclude:: ../largeNet.ini
   :language: ini
   :start-at: LargeNet.n = 15
   :end-before: #####

Conceptually the same result is get with the this configuration, as with
the ``LargeNet_overloadedBySendIntervall`` one, meaning an overloaded
network. The highest drop rate occurs at ``switchB.eth[18]``, meaning
that the connection between the backbone switches and ``switchB`` is the
most congested. This is due to the fact that the number of switches on
the backbone (``n`` = 15) is relatively large. As a consequence of this,
a high traffic is present on that single connection, causing it to
become the most critical part of the network.

Although the overall number of hosts increased compared to the
``LargeNet_overloadedBySendIntervall`` configuration, the number of
hosts connected to each subnet LAN remained the same. The traffic rate
(``sendIntervall`` parameter) of the hosts is the same as it was with
the ``LargeNet_notOverloaded`` configuration, so nearly the same
collision rate was expected. If we take a look at the statistics, we can
see that the ``collisions`` is in the same order of magnitude as it was
with the first configuration.

Further Information
-------------------

Useful links:
-  `LAN <https://en.wikipedia.org/wiki/Local_area_network>`__
-  `Ethernet <https://en.wikipedia.org/wiki/Ethernet>`__ - `Ethernet
frame <https://en.wikipedia.org/wiki/Ethernet_frame>`__
-  `MAC <https://en.wikipedia.org/wiki/Medium_access_control>`__
-  `LLC <https://en.wikipedia.org/wiki/Logical_link_control>`__
-  `Backbone Network <https://en.wikipedia.org/wiki/Backbone_network>`__

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-showcases/issues/??>`__ in
the GitHub issue tracker for commenting on this showcase.

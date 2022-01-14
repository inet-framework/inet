.. _ug:cha:ethernet:

The Ethernet Model
==================

.. _ug:sec:ethernet:overview:

Overview
--------

Ethernet is the most popular wired LAN technology nowadays, and its use
is also growing in metropolitan area and wide area networks. Since its
introduction in 1980, Ethernet data transfer rates have increased from
the original 10Mb/s to the latest 400Gb/s. Originally, The technology
has changed from using coaxial cables and repeaters to using unshielded
twisted-pair cables with hubs and switches. Today, switched Ethernet is
prevalent, and most links operate in full duplex mode. The INET
Framework contains support for all major Ethernet technologies and
device types.

In Ethernet networks containing multiple switches, broadcast storms are
prevented by use of a spanning tree protocol (STP, RSTP) that disables
selected links to eliminate cycles from the topology. Ethernet switch
models in INET contain support for STP and RSTP.

.. _ug:sec:ethernet:nodes:

Nodes
-----

There are several node models that can be used in an Ethernet network:

-  Node models such as :ned:`StandardHost` and :ned:`Router` are
   Ethernet-capable

-  :ned:`EthernetSwitch` models an Ethernet switch, i.e. a multiport
   bridging device

-  :ned:`WireJunction` can models the coaxial cable (10BASE2 or 10BASE5 network
   segments) on legacy Ethernet networks, or an Ethernet hub/multiport repeater

-  :ned:`EthernetHost` is a sample node which can be used to generate “raw”
   Ethernet traffic

.. _ug:sec:ethernet:etherswitch:

Ethernet Switch
~~~~~~~~~~~~~~~

:ned:`EthernetSwitch` models an Ethernet switch. Ethernet switches play an
important role in modern Ethernet LANs. Unlike passive hubs and
repeaters that work in the physical layer, the switches operate in the
data link layer and relay frames between the connected subnets.

In modern Ethernet LANs, each node is connected to the switch directly
by full duplex lines, so no collisions are possible. In this case, the
CSMA/CD is not needed and the channel utilization can be high.

The :par:`duplexMode` parameters of the MACs must be set according to
the medium connected to the port; if collisions are possible (it’s a bus
or hub) it must be set to false, otherwise it can be set to true. By
default it uses half-duplex MAC with CSMA/CD.

.. _ug:sec:ethernet:etherhub:

Ethernet Hub
~~~~~~~~~~~~

The :ned:`WireJunction` can model an Ethernet hub. Ethernet hubs are a simple
broadcast devices. Messages arriving on a port are regenerated and
broadcast to every other port.

The connections connected to the hub must have the same data rate. Cable
lengths should be reflected in the delays of the connections.

.. _ug:sec:ethernet:etherbus:

Ethernet Bus
~~~~~~~~~~~~

The :ned:`WireJunction` component can also model a connection to a 
common coaxial cable found in early Ethernet LANs. Network nodes
are attached to the :ned:`WireJunction` via a :ned:`DatarateChannel`.
The :ned:`WireJunction` modules are connected to each other via 
:ned:`DatarateChannel` as well. When a node sends a signal, it will propagate
along the cable in both directions at the given propagation speed.

The speed of the connection can be set on the datarate channels; 
all connected channels must have the same speed.

.. _ug:sec:ethernet:the-physical-layer:

The Physical Layer
------------------

Stations on an Ethernet networks are connected by coaxial, twisted pair
or fibre cables. (Coaxial only has historical importance, but is
supported by INET anyway.) There are several cable types specified in
the standard.

In the INET framework, the cables are represented by connections. The
connections used in Ethernet LANs must be derived from
ned::``DatarateChannel`` and should have their :par:`delay` and
:par:`datarate` parameters set. The delay parameter can be used to model
the distance between the nodes. The datarate parameter can have four
values:

-  10Mbps (classic Ethernet)

-  100Mbps (Fast Ethernet)

-  1Gbps (Gigabit Ethernet, GbE)

-  10Gbps (10 Gigabit Ethernet, 10GbE)

-  40Gbps (40 Gigabit Ethernet, 40GbE)

-  100Gbps (100 Gigabit Ethernet, 100GbE)

There is currently no support for 200Gbps and 400Gbps Ethernet.

:ned:`Eth10M`, :ned:`Eth100M`, :ned:`Eth1G`, :ned:`Eth10G`,
:ned:`Eth40G`, :ned:`Eth100G`

.. _ug:sec:ethernet:ethernet-interface:

Ethernet Interface
------------------

The :ned:`EthernetInterface` compound module implements the
:ned:`IWiredInterface` interface. Complements :ned:`EthernetCsmaMac` and
:ned:`EthernetEncapsulation` with an output queue for QoS and RED support. It also
has configurable input/output filters as :ned:`IHook` components
similarly to the :ned:`PppInterface` module.

The Ethernet MAC (Media Access Control) layer transmits the Ethernet
frames on the physical media. This is a sublayer within the data link
layer. Because encapsulation/decapsulation is not always needed (e.g.
switches does not do encapsulation/decapsulation), it is implemented in
a separate modules (e.g. :ned:`EthernetEncapsulation`) that are part
of the LLC layer.

Nowadays almost all Ethernet networks operate using full-duplex
point-to-point connections between hosts and switches. This means that
there are no collisions, and the behaviour of the MAC component is much
simpler than in classic Ethernet that used coaxial cables and hubs. The
INET framework contains two MAC modules for Ethernet: the
:ned:`EthernetMac` is simpler to understand and easier to extend,
because it supports only full-duplex connections. The :ned:`EthernetCsmaMac`
module implements the full MAC functionality including CSMA/CD, it can
operate both half-duplex and full-duplex mode.

.. _ug:sec:ethernet:components:

Components
----------

The following components are present in the model:

-  :ned:`EthernetMac`

-  :ned:`EthernetCsmaMac`

-  :ned:`EthernetEncapsulation`

-  :ned:`MacRelayUnit`

-  :ned:`MacForwardingTable`

-  :ned:`Ieee8021dRelay`

.. _ug:sec:ethernet:ethermacfullduplex:

EthernetMac
~~~~~~~~~~~

From the two MAC implementation :ned:`EthernetMac` is the simpler
one, it operates only in full-duplex mode (its :par:`duplexEnabled`
parameter fixed to ``true`` in its NED definition). This module does
not need to implement CSMA/CD, so there is no collision detection,
retransmission with exponential backoff, carrier extension and frame
bursting.

.. _ug:sec:ethernet:ethermac:

EthernetCsmaMac
~~~~~~~~~~~~~~~

Ethernet MAC layer implementing CSMA/CD. It supports both half-duplex
and full-duplex operations; in full-duplex mode it behaves as
:ned:`EthernetMac`. In half-duplex mode it detects collisions,
sends jam messages and retransmit frames upon collisions using the
exponential backoff algorithm. In Gigabit Ethernet networks it supports
carrier extension and frame bursting. Carrier extension can be turned
off by setting the :par:`carrierExtension` parameter to ``false``.

.. _ug:sec:ethernet:etherencap:

EthernetEncapsulation
~~~~~~~~~~~~~~~~~~~~~

The :ned:`EthernetEncapsulation` module performs Ethernet II or Ethernet with SNAP
encapsulation/decapsulation.

.. _ug:sec:ethernet:macrelayunit:

MacRelayUnit
~~~~~~~~~~~~

INET framework ethernet switches are built from :ned:`IMacRelayUnit`
components. Each relay unit has N input and output gates for
sending/receiving Ethernet frames. They should be connected to
:ned:`EthernetInterface` modules.

The relay unit holds a table for the destination address -> output port
mapping in a :ned:`MacForwardingTable` module. When the relay unit receives
a data frame, it updates the table with the source address->input port.

If the destination address is not found in the table, the frame is
broadcast. The frame is not sent to the same port it was received from,
because then the target should already have received the original frame.

A simple scheme for sending PAUSE frames is built in (although users
will probably change it). When the buffer level goes above a high
watermark, PAUSE frames are sent on all ports. The watermark and the
pause time is configurable; use zero values to disable the PAUSE
feature.

.. _ug:sec:ethernet:macforwardingtable:

MacForwardingTable
~~~~~~~~~~~~~~~

The :ned:`MacForwardingTable` module stores the mapping between ports and
MAC addresses. Entries are deleted if their age exceeds a certain limit.

If needed, address tables can be pre-loaded from text files at the
beginning of the simulation; this controlled by the
:par:`forwardingTableFile` module parameter. In the file, each line
contains a literal 0 (reserved for VLAN id), a hexadecimal MAC address
and a decimal port number, separated by tabs. Comment lines beginning
with ’#’ are also allowed:

::

   0    01 ff ff ff ff    0
   0    00-ff-ff-ee-d1    1
   0    0A:AA:BC:DE:FF    2

Entries are deleted if their age exceeds the duration given as the
:par:`agingTime` parameter.

.. _ug:sec:ethernet:ieee8021drelay:

Ieee8021dRelay
~~~~~~~~~~~~~~

:ned:`Ieee8021dRelay` is a MAC relay unit that should be used instead of
:ned:`MacRelayUnit` that when STP or RSTP is needed.

.. _ug:sec:ethernet:stp:

Stp
~~~

The :ned:`Stp` module type implements Spanning Tree Protocol (STP). STP
is a network protocol that builds a loop-free logical topology for
Ethernet networks. The basic function of STP is to prevent bridge loops
and the broadcast radiation that results from them.

STP creates a spanning tree within a network of connected layer-2
bridges, and disables those links that are not part of the spanning
tree, leaving a single active path between any two network nodes.

.. _ug:sec:ethernet:rstp:

Rstp
~~~~

:ned:`Rstp` implements Rapid Spanning Tree Protocol (RSTP), an improved
version of STP. RSTP provides significantly faster recovery in response
to network changes or failures.

.. _ug:sec:ethernet:implemented-standards:

Implemented Standards
---------------------

The Ethernet model operates according to the following standards:

-  Ethernet: IEEE 802.3-1998

-  Fast Ethernet: IEEE 802.3u-1995

-  Full-Duplex Ethernet with Flow Control: IEEE 802.3x-1997

-  Gigabit Ethernet: IEEE 802.3z-1998

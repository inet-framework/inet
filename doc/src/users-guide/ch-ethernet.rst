.. _ug:cha:ethernet:

The Ethernet Model
==================

.. _ug:sec:ethernet:overview:

Overview
--------

Ethernet is the most popular wired LAN technology nowadays, and its use
is also growing in metropolitan area and wide area networks. Since its
introduction in 1980, Ethernet data transfer rates have increased from
the original 10Mb/s to 800Gb/s. The technology has also changed from
using coaxial cables and repeaters to using unshielded twisted-pair cables
with hubs and switches. Today, switched Ethernet is prevalent, and most
links operate in full-duplex mode.

In a network node, the Ethernet model consists of three parts:

-  the network interfaces (``eth[*]``), which transmit and receive the
   frames of the ports of the node

-  the Ethernet layer (``ethernet``), which inserts and removes the
   Ethernet MAC header and the FCS for all network interfaces

-  the relay (``bridging``), which relays frames between the ports of a
   switch

For each part, INET provides a *basic* implementation, which performs the
function in one or a few larger modules, and a *modular* implementation,
which builds it from small protocol elements that can be replaced or
omitted one by one:

-  network interface: :ned:`EthernetInterface` (basic) and
   :ned:`LayeredEthernetInterface` (modular)

-  Ethernet layer: :ned:`EthernetEncapsulation` (basic) and
   :ned:`EthernetLayer` (modular)

-  relay: :ned:`MacRelayUnit`, :ned:`Ieee8021dRelay` and :ned:`MrpRelay`
   (basic), and :ned:`BridgingLayer` (modular)

:ned:`StandardHost`, :ned:`Router` and :ned:`EthernetSwitch` use the basic
implementations by default, and :ned:`TsnDevice` and :ned:`TsnSwitch` use
the modular ones. The implementation of each part is selected with a
``typename`` parameter, independently of the other two parts. The only
rule is that :ned:`LayeredEthernetInterface` must be used together with
:ned:`EthernetLayer`, because it leaves the address filtering of the
received frames to the Ethernet layer.

The basic and the modular implementations of a part support different
features. For example, half-duplex operation needs
:ned:`EthernetInterface`, the spanning tree protocols need
:ned:`Ieee8021dRelay`, and VLAN port policies need :ned:`BridgingLayer`. The
:ref:`ug:sec:ethernet:comparison` section lists the differences for each
part.

This chapter first presents the network nodes and the links of an Ethernet
network, then the network interface, the Ethernet and IEEE 802.1Q layers,
the architecture of Ethernet switches (bridges), and finally VLANs.

.. _ug:sec:ethernet:nodes:

Network Nodes
-------------

Several node models can be used in an Ethernet network:

-  :ned:`StandardHost`, :ned:`Router`, and the other node models derived from
   :ned:`LinkLayerNodeBase` are Ethernet-capable. They support multiple
   Ethernet interfaces, determined by the size of the ``ethg`` gate vector.

-  :ned:`EthernetSwitch` models an Ethernet switch, which is a multiport
   bridging device. Its architecture is described in
   :ref:`ug:sec:ethernet:bridge-architecture`.

-  :ned:`TsnDevice` and :ned:`TsnSwitch` are a host and a switch with the
   modular implementations and the TSN features; see :doc:`ch-tsn`.

-  :ned:`EthernetHub` and :ned:`WireJunction` model an Ethernet hub
   (multiport repeater), and :ned:`WireJunction` can also model the coaxial
   cable of legacy Ethernet networks (10BASE2 or 10BASE5 network segments).

-  :ned:`EthernetHost` is a sample node that can be used to generate "raw"
   Ethernet traffic, without IP.

-  :ned:`EthernetPlcaHost` is a node for 10BASE-T1S multidrop links; see
   :doc:`ch-10base-t1s`.

.. _ug:sec:ethernet:the-physical-layer:

Links
-----

Stations on Ethernet networks are connected by coaxial, twisted pair
or fiber cables. (Coaxial only has historical importance, but is
supported by INET anyway.)

In INET, the cables are represented by connections. The connections used
in Ethernet networks must be derived from :ned:`DatarateChannel`. INET
provides the :ned:`EthernetLink` channel type, which computes the
propagation delay from its :par:`length` parameter, and its subtypes with
a fixed data rate: :ned:`Eth10M`, :ned:`Eth100M`, :ned:`Eth1G`,
:ned:`Eth10G`, :ned:`Eth40G`, :ned:`Eth100G`, :ned:`Eth200G` and
:ned:`Eth400G`.

The data rate can have the following values:

-  10Mbps (classic Ethernet)

-  100Mbps (Fast Ethernet)

-  1Gbps (Gigabit Ethernet), 2.5Gbps and 5Gbps

-  10Gbps, 25Gbps and 40Gbps

-  100Gbps, 200Gbps, 400Gbps and 800Gbps

:ned:`EthernetInterface` takes the data rate from its :par:`bitrate`
parameter if it is set, otherwise from the connected channel. Its MACs
accept other data rates only if their :par:`allowNonstandardBitrate`
parameter is set. :ned:`LayeredEthernetInterface` takes the data rate from
its :par:`bitrate` parameter, which must be set.

.. _ug:sec:ethernet:etherhub:

Hubs
~~~~

:ned:`EthernetHub` and :ned:`WireJunction` can model an Ethernet hub.
Ethernet hubs are simple broadcast devices. Signals arriving on a port are
regenerated and broadcast to every other port, so the network nodes
connected to a hub form a single collision domain.

The connections connected to the hub must have the same data rate. Cable
lengths should be reflected in the delays of the connections.

.. _ug:sec:ethernet:etherbus:

Buses
~~~~~

The :ned:`WireJunction` component can also model a connection to a
common coaxial cable found in early Ethernet LANs. Network nodes
are attached to the :ned:`WireJunction` via a :ned:`DatarateChannel`.
The :ned:`WireJunction` modules are connected to each other via
:ned:`DatarateChannel` as well. When a node sends a signal, it will propagate
along the cable in both directions at the given propagation speed.

The speed of the connection can be set on the datarate channels;
all connected channels must have the same speed.

Network interfaces connected to a hub or a bus must operate in half-duplex
mode; see :ref:`ug:sec:ethernet:ethermac`.

.. _ug:sec:ethernet:ethernet-interface:

The Ethernet Interface
----------------------

An Ethernet network interface transmits and receives the frames of one
port of a network node. It does not insert or remove the Ethernet MAC
header and the FCS; that is the task of the Ethernet layer of the node
(see :ref:`ug:sec:ethernet:protocol-layers`), which serves all Ethernet
interfaces of the node. The network interfaces implement the
:ned:`IEthernetInterface` module interface, and the ``eth[*]`` submodules
of the network nodes select one with their ``typename`` parameter.

INET provides the following Ethernet network interfaces:

-  :ned:`EthernetInterface`: the basic network interface

-  :ned:`LayeredEthernetInterface`: the modular network interface

-  :ned:`EthernetCutthroughInterface`: an extension of
   :ned:`LayeredEthernetInterface` that supports cut-through switching; see
   :doc:`ch-tsn`

-  :ned:`EthernetPlcaInterface`: a network interface for 10BASE-T1S
   multidrop links; see :doc:`ch-10base-t1s`

.. _ug:sec:ethernet:etherinterface:

EthernetInterface
~~~~~~~~~~~~~~~~~

:ned:`EthernetInterface` contains an output queue (``queue``), a combined
MAC and PHY module (``mac``), and optional traffic conditioners for the
outgoing and the incoming traffic (``egressTC`` and ``ingressTC``) that can
be used for QoS, for example for DiffServ (see :doc:`ch-diffserv`).

:ned:`EthernetInterface` has no separate physical layer module. Its ``mac``
submodule implements the Ethernet MAC, and it also does the work of the
physical layer: it inserts and removes the preamble and the start frame
delimiter, transmits each frame as a signal that lasts for the transmission
time, and keeps the interframe gap. The two modules that can fill this role
have "MacPhy" in their names for this reason:

-  :ned:`EthernetMacPhy` supports only full-duplex operation, so it has no
   CSMA/CD. It is simpler to understand and to extend.

-  :ned:`EthernetCsmaMacPhy` implements CSMA/CD, and it can operate in both
   half-duplex and full-duplex mode.

The :par:`duplexMode` parameter of the network interface selects
full-duplex (``true``, the default) or half-duplex (``false``) operation.
The :par:`csmacdSupport` parameter selects the module:
:ned:`EthernetCsmaMacPhy` if it is ``true``, :ned:`EthernetMacPhy`
otherwise. By default, it is the opposite of :par:`duplexMode`, so a
full-duplex network interface uses :ned:`EthernetMacPhy`, and a half-duplex
one uses :ned:`EthernetCsmaMacPhy`.

The MAC and the PHY are separate modules in
:ned:`LayeredEthernetInterface` (see
:ref:`ug:sec:ethernet:layeredinterface`), and in
:ned:`EthernetPlcaInterface`, which becomes a CSMA/CD network interface
with a separate MAC and PHY if its ``plca`` submodule is omitted.

Nowadays, almost all Ethernet networks operate using full-duplex
point-to-point connections between hosts and switches. This means that
there are no collisions, and the behavior of the MAC component is much
simpler than in classic Ethernet that used coaxial cables and hubs.

The MAC+PHY modules fill in the source MAC address of outgoing frames if it
is empty, and they check the FCS of incoming frames. They pass up only the
frames addressed to the network interface and the broadcast frames, unless
their :par:`promiscuous` parameter is set, in which case they pass up all
frames. Switches set this parameter.

.. _ug:sec:ethernet:ethermacfullduplex:

EthernetMacPhy
^^^^^^^^^^^^^^

Of the two MAC+PHY modules, :ned:`EthernetMacPhy` is the simpler one. It
operates only in full-duplex mode (its :par:`duplexMode` parameter must be
``true``). This module does not need to implement CSMA/CD, so there is no
collision detection, retransmission with exponential backoff, carrier
extension, and frame bursting.

.. _ug:sec:ethernet:ethermac:

EthernetCsmaMacPhy
^^^^^^^^^^^^^^^^^^

:ned:`EthernetCsmaMacPhy` implements the Ethernet MAC with CSMA/CD. It
supports both half-duplex and full-duplex operation; in full-duplex mode,
it behaves as :ned:`EthernetMacPhy`. In half-duplex mode, it detects
collisions, sends jam messages, and retransmits frames upon collisions using the
exponential backoff algorithm. In half-duplex Gigabit Ethernet networks,
it uses carrier extension, and it supports frame bursting. Frame bursting
can be turned off by setting the :par:`frameBursting` parameter to
``false``.

.. _ug:sec:ethernet:layeredinterface:

LayeredEthernetInterface
~~~~~~~~~~~~~~~~~~~~~~~~

:ned:`LayeredEthernetInterface` is a stack of protocol layers, each of which
can be replaced or omitted:

-  ``processingDelayLayer`` (empty by default) models the processing delay
   of the network interface

-  ``trafficConditionerLayer`` (empty by default) contains traffic
   conditioners for the outgoing and the incoming traffic

-  ``measurementLayer`` (empty by default) measures delays and other
   properties of the packets passing through

-  ``macLayer`` contains the transmission queue and the server that takes
   the frames from the queue for transmission. It is
   :ned:`EthernetMacLayer` by default, and the TSN features replace it, for
   example with :ned:`EthernetPreemptingMacLayer` for frame preemption.

-  ``phyLayer`` inserts and removes the preamble and the start frame
   delimiter, and transmits and receives the frames. It is
   :ned:`EthernetPhyLayer` by default, and the TSN features replace it with
   a streaming variant when the transmission must be observed or
   interrupted while it is in progress.

:ned:`LayeredEthernetInterface` operates in full-duplex mode only. It does
not filter the received frames by their destination address, unlike the MAC
of :ned:`EthernetInterface`. The address filtering is done by the
:ned:`EthernetLayer` above the network interfaces, whose
:par:`promiscuous` parameter has the same meaning, so
:ned:`LayeredEthernetInterface` must be used together with
:ned:`EthernetLayer`.

The queue of the MAC layer can be replaced with any queue of the queueing
library (see :doc:`ch-queueing`), for example with a priority queue that
uses the PCP of the frames (see :ref:`ug:sec:ethernet:vlan-priority`), or
with the traffic shapers of the TSN chapter.

.. _ug:sec:ethernet:protocol-layers:

The Ethernet and IEEE 802.1Q Layers
-----------------------------------

The Ethernet layer (``ethernet``), the IEEE 802.1Q layer (``ieee8021q``)
and the IEEE 802.1CB layer (``ieee8021r``) are submodules of the network
node, not of the network interfaces: :ned:`LinkLayerNodeBase` (and so
:ned:`StandardHost` and :ned:`Router`) and :ned:`EthernetSwitch` have them,
and one instance of each serves all Ethernet interfaces of the node. They
are connected to the network interfaces, to the ``bridging`` submodule and
to the higher layers through message dispatchers (see
:ref:`ug:sec:nodes:node-architecture`), so a received frame goes to the
module of its next header: a tagged frame goes from the Ethernet layer to
the 802.1Q layer, and an untagged frame goes from the Ethernet layer
directly to the bridging layer or to a higher layer protocol.

.. _ug:sec:ethernet:etherencap:

The Ethernet Layer
~~~~~~~~~~~~~~~~~~

The ``ethernet`` submodule inserts and removes the Ethernet MAC header,
the padding and the FCS. Two implementations are provided:

-  :ned:`EthernetEncapsulation`, the basic one, performs Ethernet II or
   Ethernet with SNAP encapsulation and decapsulation in one module. It
   is the default.

-  :ned:`EthernetLayer`, the modular one, consists of separate header,
   padding and FCS inserters and checkers. It also filters the received
   frames by their destination address. :ned:`EthernetInterface` does this
   in its MAC, but :ned:`LayeredEthernetInterface` does not, so
   :ned:`LayeredEthernetInterface` must be used together with
   :ned:`EthernetLayer`.

Both provide Ethernet sockets for applications that send and receive raw
Ethernet frames (:ned:`EthernetLayer` if its :par:`hasSocketSupport`
parameter is set). The :par:`fcsMode` parameter selects whether the FCS is
computed (``"computed"``) or only declared correct (``"declared"``).

.. _ug:sec:ethernet:ieee8021qlayer:

The IEEE 802.1Q Layer
~~~~~~~~~~~~~~~~~~~~~

The ``ieee8021q`` submodule inserts and removes the VLAN tags. It is absent
by default; it is needed in every node that sends or receives tagged
frames. Two implementations are provided:

-  :ned:`Ieee8021qProtocol` only inserts and removes the tags. This is the
   one to use together with the VLAN policies of the bridging layer.

-  :ned:`Ieee8021qLayer` combines an :ned:`Ieee8021qProtocol` with a VLAN
   policy above it. This policy only sees the frames that carry a tag
   when they are received, and the frames that already have a VLAN
   request when they are sent.

The :par:`vlanTagType` parameter selects the tag that the layer handles:
``"c"`` for the C-tag (TPID 0x8100) and ``"s"`` for the S-tag (TPID 0x88a8)
of IEEE 802.1ad. See :ref:`ug:sec:ethernet:vlans` for details. If the
:par:`hasSocketSupport` parameter is set, applications and virtual network
interfaces can send and receive frames of a VLAN through IEEE 802.1Q
sockets.

The ``ieee8021r`` submodule (:ned:`Ieee8021rProtocol`) inserts and removes
the R-tag of IEEE 802.1CB frame replication and elimination; see
:doc:`ch-tsn`.

.. _ug:sec:ethernet:bridge-architecture:

Bridge Architecture
-------------------

Ethernet switches are called *bridges* in the IEEE 802.1 standards. A
bridge relays frames between its ports based on their destination MAC
addresses, and it learns which MAC address can be reached through which
port from the source addresses of the frames it receives.

.. _ug:sec:ethernet:etherswitch:

EthernetSwitch
~~~~~~~~~~~~~~

:ned:`EthernetSwitch` is built from the same protocol layers as hosts (see
:ref:`ug:sec:ethernet:protocol-layers`), with a relay unit in its
``bridging`` submodule. Its network interfaces and its Ethernet layer are in
promiscuous mode, so that the relay unit receives every frame.

The ``bridging`` submodule can be one of the following:

-  :ned:`Ieee8021dRelay`: a basic relay unit, which also supports the
   spanning tree protocols. It is the default if the Ieee8021d feature of
   INET is enabled.

-  :ned:`MacRelayUnit`: a simpler basic relay unit without spanning tree
   and MRP support; the default if the Ieee8021d feature is disabled

-  :ned:`MrpRelay`: a basic relay unit with Media Redundancy Protocol
   support (the default if :par:`hasMrp` is set)

-  :ned:`BridgingLayer`: the modular relay, which is the default in
   :ned:`TsnSwitch`

The other submodules of the switch are:

-  ``macTable`` is the MAC address table (forwarding database) of the relay
   unit; see :ref:`ug:sec:ethernet:macforwardingtable`

-  ``interfaceTable`` holds the network interfaces (the ports) of the switch

-  ``stp`` is the spanning tree protocol (if :par:`hasStp` is set), ``mrp``
   is the Media Redundancy Protocol (if :par:`hasMrp` is set), and
   ``l2NodeConfigurator`` applies their per-port configuration; see
   :ref:`ug:sec:ethernet:stp`

-  ``gptp`` is the gPTP time synchronization protocol (if :par:`hasGptp`
   is set); see :doc:`ch-tsn`

-  ``llc`` is the :ned:`Ieee8022Llc` layer, needed by MRP

-  ``app[*]`` are applications that run on the switch itself

The parameters :par:`hasCutthroughSwitching` and :par:`numPcapRecorders` of
the switch select cut-through network interfaces and PCAP recording.

To use the modular implementations in a switch, set the ``typename`` of
the relay, the Ethernet layer and the network interfaces:

.. code-block:: ini

   *.switch*.bridging.typename = "BridgingLayer"
   *.switch*.ethernet.typename = "EthernetLayer"
   *.switch*.eth[*].typename = "LayeredEthernetInterface"
   *.switch*.eth[*].bitrate = 100Mbps

The three parts can also be chosen separately. For example, a switch that
runs a spanning tree protocol can have the modular network interfaces and
Ethernet layer with :ned:`Ieee8021dRelay`.

.. _ug:sec:ethernet:macrelayunit:
.. _ug:sec:ethernet:ieee8021drelay:

The Basic Relay Units
~~~~~~~~~~~~~~~~~~~~~

:ned:`MacRelayUnit` and :ned:`Ieee8021dRelay` are simple modules that
implement the relay function in one place. When a relay unit receives a
frame, it updates the MAC address table with the source address and the
incoming port. Then it looks up the destination address in the table: if
the address is found, it sends the frame to that port; otherwise, and for
broadcast frames, it sends the frame to all other ports. A multicast frame
goes to the ports listed in the table for the multicast address, or to all
other ports if there is none. The frame is never sent back to the port it
was received from, because then the target should already have received
the original frame.

The relay unit sends the frame out with the VLAN tags, the PCP and the drop
eligible indicator that it was received with. The :par:`vlanTagType`
parameter selects the tag whose VLAN id the relay unit uses for learning
and for the lookup; see :ref:`ug:sec:ethernet:vlan-bridges`.

:ned:`Ieee8021dRelay` also cooperates with the spanning tree protocols: it
passes the BPDUs to the ``stp`` submodule, and it learns and forwards only
on the ports whose port state allows it. Without a spanning tree protocol,
it behaves as :ned:`MacRelayUnit`. :ned:`MrpRelay` extends it with the
handling of the frames and the port states of MRP.

.. _ug:sec:ethernet:bridginglayer:

The BridgingLayer
~~~~~~~~~~~~~~~~~

:ned:`BridgingLayer` is the modular relay. It is a stack of
protocol layers, each of which can be replaced or omitted. From top to
bottom:

-  ``processingDelayLayer`` (empty by default) models the processing delay
   of the bridge

-  ``directionReverser`` (:ned:`DirectionReverserLayer`) decides whether a
   received frame is delivered to a protocol of the node itself, or
   forwarded. A forwarded frame is sent with the MAC addresses, VLAN tags
   and PCP it was received with (the :par:`forwardVlan` and
   :par:`forwardPcp` parameters of its ``reverser`` submodule control the
   last two).

-  ``streamIdentifier``, ``streamRelay``, ``streamFilter`` and
   ``streamCoder`` (all empty by default) implement the TSN stream
   identification, frame replication and elimination, per-stream filtering
   and policing, and the encoding of streams into VLAN tags; see
   :doc:`ch-tsn`

-  ``interfaceRelay`` (:ned:`InterfaceRelayLayer`) contains the relay
   function proper: the :ned:`RelayInterfaceLearner` updates the MAC
   address table from the incoming frames, and the
   :ned:`RelayInterfaceSelector` selects the outgoing ports of a frame from
   the table, in the same way as the basic relay units do

-  ``vlanPolicy`` (empty by default) applies the VLAN policy of the bridge;
   see :ref:`ug:sec:ethernet:vlan-bridges`

The VLAN policy is at the bottom of the bridging layer, so it sees every
frame between the network interfaces and the relay, whether it is tagged
or not. :ned:`BridgingLayer` can also be used in hosts with all submodules
but ``vlanPolicy`` omitted, to apply a VLAN policy to the traffic of the
host.

:ned:`BridgingLayer` does not take the port states of the spanning tree
protocols into account, so it cannot be used together with STP, RSTP or
MRP.

.. _ug:sec:ethernet:macforwardingtable:

MAC Address Table
~~~~~~~~~~~~~~~~~

The :ned:`MacForwardingTable` module stores the mapping between MAC
addresses and ports, for each VLAN separately. Both the basic relay units
and the :ned:`BridgingLayer` use it. Entries are deleted if their age
exceeds a certain limit.

If needed, address tables can be pre-loaded from text files at the
beginning of the simulation; this is controlled by the
:par:`forwardingTableFile` module parameter. In the file, each line
contains a VLAN id (0 if VLANs are not used), a MAC address, and the name
of a network interface of the switch, separated by spaces or tabs. Comment
lines beginning with "#" are also allowed:

::

   # VLAN  MAC address        interface
   0       0A-AA-BC-DE-FF-01  eth0
   0       0A-AA-BC-DE-FF-02  eth1

Entries can also be given in the :par:`forwardingTable` parameter. Here, the
address can also be the name of a network node:

.. code-block:: ini

   *.switch.macTable.forwardingTable = [{address: "host1", interface: "eth0"},
                                        {address: "0A-AA-BC-DE-FF-02", interface: "eth1", vlan: 0}]

Learned entries are deleted if their age exceeds the duration given as the
:par:`agingTime` parameter. Pre-loaded entries do not age.

:ned:`MrpMacForwardingTable` is a variant of the table for MRP, and it is
the default if the :par:`hasMrp` parameter of the switch is set.

.. _ug:sec:ethernet:stp:
.. _ug:sec:ethernet:rstp:

Loop Prevention: STP, RSTP and MRP
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In Ethernet networks containing multiple switches, broadcast storms are
prevented by protocols that disable selected links to eliminate cycles from
the topology.

The :ned:`Stp` module type implements the Spanning Tree Protocol (STP) of
IEEE 802.1D. STP creates a spanning tree within a network of connected
layer-2 bridges and disables those links that are not part of the spanning
tree, leaving a single active path between any two network nodes.
:ned:`Rstp` implements the Rapid Spanning Tree Protocol (RSTP), an improved
version of STP, which provides significantly faster recovery in response to
network changes or failures.

The :par:`hasStp` parameter of :ned:`EthernetSwitch` enables the spanning
tree protocol, and the :par:`spanningTreeProtocol` parameter selects
between ``"Stp"`` (the default) and ``"Rstp"``. The per-port configuration,
such as the port priorities and the link costs, can be given to the
:ned:`L2NetworkConfigurator` module of the network, and the
``l2NodeConfigurator`` submodule of each switch applies it.

:ned:`Mrp` implements the Media Redundancy Protocol (MRP) of IEC 62439-2,
which manages the redundancy of ring topologies found in industrial
networks. The :par:`hasMrp` parameter of :ned:`EthernetSwitch` enables it.

The spanning tree protocols and MRP require a relay unit that takes the
port states into account: :ned:`Ieee8021dRelay` for STP and RSTP, and
:ned:`MrpRelay` for MRP. :ned:`MacRelayUnit` and :ned:`BridgingLayer` do
not take the port states into account, so they cannot be used together
with STP, RSTP or MRP.

.. _ug:sec:ethernet:vlans:

VLANs
-----

A virtual LAN (VLAN) divides one physical Ethernet network into several
logical ones: the frames of a VLAN only reach the ports that are members of
that VLAN, as if each VLAN was a separate network. INET implements VLANs
as specified in IEEE 802.1Q, including the provider bridging (Q-in-Q) of
IEEE 802.1ad.

.. _ug:sec:ethernet:vlan-concepts:

Concepts
~~~~~~~~

A frame belongs to a VLAN either because it carries a VLAN tag, or because
the bridge that receives it classifies it into a VLAN. The VLAN tag is
four bytes long, and it is inserted after the source MAC address. It
consists of a Tag Protocol Identifier (TPID) in the place of the EtherType,
followed by the Priority Code Point (PCP, 3 bits), the Drop Eligible
Indicator (DEI, 1 bit) and the VLAN identifier (VID, 12 bits). VLAN ids 1
to 4094 identify VLANs; VLAN id 0 means a *priority-tagged* frame, which
carries only a PCP.

IEEE 802.1Q defines two kinds of tags, which only differ in their TPID:

-  the *C-tag* (customer tag, TPID 0x8100) is the ordinary VLAN tag

-  the *S-tag* (service tag, TPID 0x88a8) is the tag that a service
   provider uses to carry the traffic of its customers (IEEE 802.1ad). An
   S-tagged frame of a customer can carry the customer's own C-tag behind
   the S-tag.

A *VLAN-aware bridge* forwards a frame only within its VLAN. It learns
MAC addresses per VLAN, so the same MAC address can be reached through
different ports in different VLANs. For each port, the bridge has a
configuration that determines how the frames are classified into VLANs:

-  which frames it accepts: untagged, priority-tagged and tagged frames

-  the *port VLAN id* (PVID), the VLAN of the untagged and priority-tagged
   frames received on the port

-  the *member set*, the VLANs whose frames the port receives and sends

-  the *untagged set*, the VLANs whose frames the port sends without a tag

Common combinations have names: an *access port* is a member of a single
VLAN, and it sends and receives the frames of that VLAN untagged; a *trunk
port* is a member of several VLANs, and it sends and receives their frames
tagged.

.. _ug:sec:ethernet:vlan-model:

VLANs in INET
~~~~~~~~~~~~~

In INET, the VLAN tags are inserted and removed by the IEEE 802.1Q layer
of the nodes (see :ref:`ug:sec:ethernet:ieee8021qlayer`). The other modules
handle the VLAN of a frame through packet tags: a received frame has a
:cpp:`CVlanInd` (or :cpp:`SVlanInd` for an S-tag) with the VLAN id of its
tag, and a frame to be sent with a tag must have a :cpp:`CVlanReq` (or
:cpp:`SVlanReq`). The same holds for the PCP (:cpp:`CPcpInd`,
:cpp:`CPcpReq`, ...) and the drop eligible indicator.

The :par:`vlanTagType` parameter of the VLAN-related modules selects which
tag they work with: ``"c"`` (the default) for the C-tag, and ``"s"`` for the
S-tag. A C-VLAN bridge uses ``"c"`` everywhere, and an S-VLAN bridge
(the bridge of a service provider) uses ``"s"``: its 802.1Q layer inserts
and removes S-tags, and leaves the C-tags of the customers in the payload;
its relay learns and forwards by the S-VLAN; and its VLAN policy uses the
S-VLAN. The parameter can be set for all modules of a node at once:

.. code-block:: ini

   *.providerBridge.**.vlanTagType = "s"

.. _ug:sec:ethernet:vlan-bridges:

VLANs in Bridges
~~~~~~~~~~~~~~~~

A bridge becomes VLAN-aware if it has an IEEE 802.1Q layer: then its
relay unit learns and forwards per VLAN, and it sends a frame out with the
VLAN tag it was received with. Without a VLAN policy, however, the
bridge has no port configuration: every port is a member of every VLAN,
and untagged frames stay untagged. The VLAN policy, which classifies the
frames into VLANs and enforces the port configuration, is the
``vlanPolicy`` submodule of :ned:`BridgingLayer`. INET provides two VLAN
policies:

-  :ned:`Ieee8021adPolicy` configures each port with a port type, from a
   single parameter. This is the recommended choice for bridges.

-  :ned:`VlanPolicyLayer` consists of separate mappers and filters for
   the incoming and the outgoing frames. It is more flexible, and it can
   also be used in hosts.

A typical configuration of a VLAN-aware bridge with the modular
implementations:

.. code-block:: ini

   *.bridge.ethernet.typename = "EthernetLayer"
   *.bridge.eth[*].typename = "LayeredEthernetInterface"
   *.bridge.ieee8021q.typename = "Ieee8021qProtocol"
   *.bridge.bridging.typename = "BridgingLayer"
   *.bridge.bridging.vlanPolicy.typename = "Ieee8021adPolicy"

The basic relay units are VLAN-aware too, if the switch has an IEEE
802.1Q layer, but they have no place for a VLAN policy. With
:ned:`Ieee8021qLayer` as the 802.1Q layer, the tagged frames can be
filtered and mapped per port, but the untagged frames cannot be classified
into VLANs.

.. _ug:sec:ethernet:ieee8021adpolicy:

Ieee8021adPolicy
^^^^^^^^^^^^^^^^

The :par:`ports` parameter of :ned:`Ieee8021adPolicy` maps the name of each
network interface to the configuration of its port. The interface name
``*`` matches every interface that has no entry of its own. The port types
are:

-  ``access``: a port of a C-VLAN bridge in one C-VLAN (``vid``). It
   accepts untagged and priority-tagged frames and classifies them into
   ``vid``, drops C-tagged frames, and sends the frames of ``vid`` untagged.

-  ``trunk``: a port of a C-VLAN bridge in the C-VLANs ``vids``. It accepts
   and sends the C-tagged frames of these VLANs only.

-  ``cnp``: a Customer Network Port of an S-VLAN bridge with a port-based
   service interface. It is an ``access`` port in the S-VLAN ``vid``: it
   puts all frames of the customer into that S-VLAN, whatever C-tag they
   have, and sends the frames of the S-VLAN without an S-tag.

-  ``pnp``: a Provider Network Port of an S-VLAN bridge in the S-VLANs
   ``vids``. It is a ``trunk`` port with S-tags.

``access`` and ``trunk`` require ``vlanTagType = "c"``, ``cnp`` and ``pnp``
require ``vlanTagType = "s"``. For example, the following bridge has two
hosts in two VLANs, and a trunk to another bridge that carries both VLANs:

.. code-block:: ini

   *.bridge.bridging.vlanPolicy.ports = {"eth0": {"type": "access", "vid": 10},
                                         "eth1": {"type": "access", "vid": 20},
                                         "eth2": {"type": "trunk", "vids": [10, 20]}}

:ned:`Ieee8021adPolicy` does not support VLAN id translation, hybrid ports
(ports that send some VLANs tagged and others untagged), and the C-tagged
service interface of Provider Edge Bridges. For these, use
:ned:`VlanPolicyLayer`.

.. _ug:sec:ethernet:vlanpolicylayer:

VlanPolicyLayer
^^^^^^^^^^^^^^^

:ned:`VlanPolicyLayer` processes the received frames and the frames to be
sent with two submodules each, in the following order:

-  ``inboundFilter`` (:ned:`VlanIndFilter`) drops the received frames whose
   VLAN id is not accepted on the incoming interface

-  ``inboundMapper`` (empty by default; :ned:`VlanIndMapper`) changes the
   VLAN of the received frames, per incoming interface. Mapping the
   untagged frames (VLAN id -1) and the priority-tagged frames (VLAN id 0)
   to a VLAN implements the PVID.

-  ``outboundMapper`` (:ned:`VlanReqMapper`) changes the VLAN of the frames
   to be sent, per outgoing interface. Mapping a VLAN to -1 sends its
   frames untagged.

-  ``outboundFilter`` (:ned:`VlanReqFilter`) drops the frames to be sent
   whose VLAN id is not accepted on the outgoing interface

The parameters of the mappers and filters are maps from the interface name
to the accepted VLAN ids or to the VLAN id mapping, where -1 stands for
untagged frames. Note that the inbound filter sees the VLAN ids before the
inbound mapping, and the outbound filter sees them after the outbound
mapping. For example, the following configuration is similar to the
:ned:`Ieee8021adPolicy` example above, except that the access ports do not
accept priority-tagged frames:

.. code-block:: ini

   *.bridge.bridging.vlanPolicy.typename = "VlanPolicyLayer"
   *.bridge.bridging.vlanPolicy.inboundFilter.acceptedVlanIds = {"eth0": [-1], "eth1": [-1],
                                                                "eth2": [10, 20]}
   *.bridge.bridging.vlanPolicy.inboundMapper.typename = "VlanIndMapper"
   *.bridge.bridging.vlanPolicy.inboundMapper.mappedVlanIds = {"eth0": {"-1": 10},
                                                              "eth1": {"-1": 20}}
   *.bridge.bridging.vlanPolicy.outboundMapper.mappedVlanIds = {"eth0": {"10": -1},
                                                               "eth1": {"20": -1}}
   *.bridge.bridging.vlanPolicy.outboundFilter.acceptedVlanIds = {"eth0": [-1], "eth1": [-1],
                                                                 "eth2": [10, 20]}

.. _ug:sec:ethernet:vlan-hosts:

VLANs in Hosts
~~~~~~~~~~~~~~

A host that sends and receives tagged frames needs an IEEE 802.1Q layer.
There are two ways to put its traffic into a VLAN:

-  A *virtual network interface* on top of the Ethernet interface sends
   and receives the frames of one VLAN. The host gets virtual network
   interfaces with the :par:`numVirtInterfaces` parameter; the
   :par:`realInterface` and :par:`vlanId` parameters of the ``tunnel``
   submodule of :ned:`VirtualInterface` select the Ethernet interface and
   the VLAN. The virtual network interface uses an IEEE 802.1Q socket, so
   the :par:`hasSocketSupport` parameter of the 802.1Q layer must be set.
   The virtual network interface has its own addresses and routes, just like
   the real one.

   .. code-block:: ini

      *.host*.ieee8021q.typename = "Ieee8021qProtocol"
      *.host*.ieee8021q.hasSocketSupport = true
      *.host*.numVirtInterfaces = 1
      *.host*.virt[0].tunnel.realInterface = "eth0"
      *.host*.virt[0].tunnel.vlanId = 42

-  A :ned:`BridgingLayer` with only a VLAN policy in it tags all outgoing
   traffic of the host, and accepts the incoming frames of the VLAN:

   .. code-block:: ini

      *.host*.ieee8021q.typename = "Ieee8021qProtocol"
      *.host*.bridging.typename = "BridgingLayer"
      *.host*.bridging.directionReverser.typename = ""
      *.host*.bridging.interfaceRelay.typename = ""
      *.host*.bridging.vlanPolicy.typename = "VlanPolicyLayer"
      *.host*.bridging.vlanPolicy.outboundMapper.mappedVlanIds = {"eth0": {"-1": 42}}
      *.host*.bridging.vlanPolicy.inboundFilter.acceptedVlanIds = {"eth0": [42]}
      *.host*.bridging.vlanPolicy.outboundFilter.acceptedVlanIds = {"eth0": [42]}

In most networks, however, the hosts send and receive untagged frames, and
the access ports of the bridges classify them into VLANs.

.. _ug:sec:ethernet:provider-bridging:

Provider Bridging (IEEE 802.1ad)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In provider bridging, also known as Q-in-Q, a service provider carries the
traffic of its customers in S-VLANs, one or more for each customer. The
customers can use any C-VLAN ids, even the same ones, because the bridges
of the provider (S-VLAN bridges) forward the frames by the S-tag only. In
INET, an S-VLAN bridge is a VLAN-aware bridge with ``vlanTagType = "s"``
in all of its modules, and with ``cnp`` ports toward the customers and
``pnp`` ports toward the other bridges of the provider:

.. code-block:: ini

   *.providerEdge.**.vlanTagType = "s"
   *.providerEdge.bridging.vlanPolicy.ports = {"eth0": {"type": "cnp", "vid": 100},
                                               "eth1": {"type": "cnp", "vid": 200},
                                               "eth2": {"type": "pnp", "vids": [100, 200]}}

.. _ug:sec:ethernet:vlan-priority:

Priority
~~~~~~~~

The PCP field of the VLAN tag carries the priority of the frame. The
802.1Q layer sets the PCP of an outgoing frame from its PCP request
(:cpp:`CPcpReq` or :cpp:`SPcpReq`), or, if there is none, from the user
priority of the frame (:cpp:`UserPriorityReq`), which can be set, for
example, by a :ned:`PacketTagger`. On reception, it indicates the PCP as the
user priority of the frame (:cpp:`UserPriorityInd`). Bridges forward a
frame with the PCP it was received with.

The queues of the network interfaces can serve the frames according to
their PCP: :ned:`PcpClassifier` and :ned:`PcpTrafficClassClassifier`
classify the frames into the subqueues of a priority queue. See
:doc:`ch-queueing` and :doc:`ch-tsn` for the queues and the traffic shapers.

.. _ug:sec:ethernet:comparison:

Choosing the Implementations
----------------------------

The implementations of the network interfaces, the Ethernet layer and the
relay are chosen separately. The choice for each part depends on the
features that the simulation needs.

Network interface:

-  :ned:`EthernetInterface` is needed for half-duplex operation with
   CSMA/CD (hubs and buses), with its :ned:`EthernetCsmaMacPhy` MAC. The
   :ned:`EthernetPlcaInterface` of 10BASE-T1S is the other network
   interface for shared media.

-  :ned:`LayeredEthernetInterface` is needed for frame preemption,
   cut-through switching (with :ned:`EthernetCutthroughInterface`), and
   the modeling of the processing delay of the network interface.

Ethernet layer:

-  :ned:`EthernetLayer` is needed with :ned:`LayeredEthernetInterface`.

-  With :ned:`EthernetInterface`, either one can be used.

Relay:

-  :ned:`Ieee8021dRelay` is needed for STP and RSTP, and :ned:`MrpRelay`
   for MRP. :ned:`MacRelayUnit` supports neither.

-  :ned:`BridgingLayer` is needed for VLAN policies (classification of
   untagged frames, port types, VLAN membership), for the TSN stream
   features (stream identification, frame replication and elimination,
   per-stream filtering and policing), and for the modeling of the
   processing delay of the bridge.

-  Both learn MAC addresses and forward frames per VLAN, if the node has
   an IEEE 802.1Q layer.

.. _ug:sec:ethernet:implemented-standards:

Implemented Standards
---------------------

The Ethernet model operates according to the following standards:

-  Ethernet: IEEE 802.3, including Fast Ethernet (IEEE 802.3u),
   full-duplex operation with flow control (IEEE 802.3x), and Gigabit
   Ethernet (IEEE 802.3z)

-  MAC bridges and the Spanning Tree Protocol: IEEE 802.1D-1998

-  Rapid Spanning Tree Protocol: IEEE 802.1D-2004

-  VLANs and VLAN-aware bridges, including provider bridging (IEEE
   802.1ad): IEEE 802.1Q-2018

-  Media Redundancy Protocol: IEC 62439-2

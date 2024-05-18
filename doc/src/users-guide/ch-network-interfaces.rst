.. _ug:cha:network-interfaces:

Network Interfaces
==================

.. _ug:sec:interfaces:overview:

Overview
--------

In INET simulations, network interface modules are the primary means of
communication between network nodes. They represent the required
combination of software and hardware elements from an operating system
point-of-view.

Network interfaces are implemented with OMNeT++ compound modules that
conform to the :ned:`INetworkInterface` module interface. Network
interfaces can be further categorized as wired and wireless; they
conform to the :ned:`IWiredInterface` and :ned:`IWirelessInterface` NED
types, respectively, which are subtypes of :ned:`INetworkInterface`.

.. _ug:sec:interfaces:built-in-network-interfaces:

Built-in Network Interfaces
---------------------------

INET provides pre-assembled network interfaces for several standard
protocols, protocol tunneling, hardware emulation, etc. The most commonly used
network interfaces are as follows.

-  :ned:`EthernetInterface` represents an :protocol:`Ethernet` interface

-  :ned:`PppInterface` is for wired links using :protocol:`PPP`

-  :ned:`Ieee80211Interface` represents a Wi-Fi (:protocol:`IEEE 802.11`)
   interface

-  :ned:`Ieee802154NarrowbandInterface` and :ned:`Ieee802154UwbIrInterface`
   represent an :protocol:`IEEE 802.15.4` interface

-  :ned:`BMacInterface`, :ned:`LMacInterface`, :ned:`XMacInterface`
   provide low-power wireless sensor MAC protocols along with a simple
   hypothetical PHY protocol

-  :ned:`TunInterface` is a tunneling interface that can be directly
   used by applications

-  :ned:`LoopbackInterface` provides local loopback within the network
   node

-  :ned:`ExtLowerEthernetInterface` represents a real-world interface,
   suitable for hardware-in-the-loop simulations

.. _ug:sec:interfaces:anatomy-of-network-interfaces:

Anatomy of Network Interfaces
-----------------------------

Within the INET Framework, network interfaces are compound modules
containing many more components than just the corresponding layer 2
protocol implementation. By default, most of these components are absent but
can be added via configuration.

Typical ingredients include:

-  *Layer 2 protocol implementation*. For some interfaces such as
   :ned:`PppInterface`, this is a single module. For others like Ethernet
   and Wi-Fi, separate modules for MAC, LLC, and possibly other subcomponents
   exist.

-  *PHY model*. Some interfaces also contain separate module(s) that
   implement the physical layer. For example, :ned:`Ieee80211Interface`
   contains a radio module.

-  *Output queue*. This module allows experimentation with different
   queueing policies and implementation of QoS, RED, etc.

-  *Traffic conditioners* allow the addition of traffic shaping and policing
   elements to the interface. Examples include implementing a Diffserv router.

-  *Hooks* allow the insertion of extra modules in the incoming and outgoing
   paths of packets.

.. _ug:sec:interfaces:internal-vs-external-output-queue:

Internal vs External Output Queue
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Network interfaces usually have a queue module defined with a parametric
type like this:


.. code-block:: ned

   queue: <default("DropTailQueue")> like IPacketQueue;

When the :par:`typename` parameter of the queue submodule is unspecified
(this is the default), the queue module is a :ned:`DropTailQueue`. Conceptually,
the queue is of infinite size, but for better diagnostics, a hard limit for
queue length can often be specified via module parameters. If this limit is
exceeded, the simulation stops with an error.

If the :par:`typename` parameter of the queue module is not empty, it must
name an NED type implementing the :ned:`IPacketQueue` interface. The
queue module model enables the modeling of finite buffers or the implementation
of various queueing policies for QoS and/or RED.

The most frequently used module type for the queue module is
:ned:`DropTailQueue`, a finite-size FIFO that drops overflowing
packets). Other queue types implementing queueing policies can be
created by assembling compound modules from queueing model and DiffServ
components (see chapter :doc:`ch-diffserv`). One such example of a compound
module is :ned:`DiffservQueue`.

Below is an example ini file fragment that installs a priority queue on PPP interfaces:


.. code-block:: ini

   **.ppp[*].ppp.queue.typename = "PriorityQueue"
   **.ppp[*].ppp.queue.packetCapacity = 10
   **.ppp[*].ppp.queue.numQueues = 2
   **.ppp[*].ppp.queue.classifier.typename = "WrrClassifier"
   **.ppp[*].ppp.queue.classifier.weights = "1 1"

.. _ug:sec:interfaces:traffic-conditioners:

Traffic Conditioners
~~~~~~~~~~~~~~~~~~~~

Many network interfaces contain optional traffic conditioner submodules
defined with parametric types, like this:


.. code-block:: ned

   ingressTC: <default("")> like ITrafficConditioner if typename != "";
   egressTC: <default("")> like ITrafficConditioner if typename != "";

Traffic conditioners allow the implementation of the policing and shaping
actions of a Diffserv router. They can be added to the input or output packets
paths in the network interface. (On the output path, they are
added before the queue module.)

Traffic conditioners must implement the :ned:`ITrafficConditioner`
module interface. Traffic conditioners can be assembled from DiffServ
components (see chapter :doc:`ch-diffserv`). INET does not provide any
preassembled traffic conditioners, but some can be found in the
example simulations.

Example configuration with fictitious types:

.. code-block:: ini

   **.ppp[*].ingressTC.typename = "CustomIngressTC"
   **.ppp[*].egressTC.typename = "CustomEgressTC"

.. _ug:sec:interfaces:the-interface-table:

The Interface Table
-------------------

Network nodes typically contain an :ned:`InterfaceTable` module. The
interface table serves as a registry of all network interfaces in
the host. It does not send or receive messages; other modules access it via
C++ function calls. The interface table's contents can also be
inspected, for example, in Qtenv.

Network interfaces register themselves in the interface table at the
beginning of the simulation, typically through the MAC
(or equivalent) module.

.. _ug:sec:interfaces:wired-network-interfaces:

Wired Network Interfaces
------------------------

Wired interfaces have special-purpose OMNeT++ gates representing the capability
of having an external physical connection to another network node (e.g., an
Ethernet port). In order to enable wired communication, these gates must be
connected with special connections representing the physical cable between
the physical ports. The connections must use special OMNeT++ channels (e.g.,
:ned:`DatarateChannel`) which determine data rate and delay parameters.

Wired network interfaces are compound modules implementing the
:ned:`IWiredInterface` interface. The following wired network interfaces
are available in INET.

.. _ug:sec:interfaces:ppp:

PPP
~~~

Chapter :doc:`ch-ppp` describes network interfaces for point-to-point links
(namely :ned:`PppInterface`). They are typically used
in routers.

.. _ug:sec:interfaces:ethernet:

Ethernet
~~~~~~~~

Ethernet interfaces (:ned:`EthernetInterface`), together with models of
Ethernet devices such as switches and hubs, are described in chapter
:doc:`ch-ethernet`.

.. _ug:sec:interfaces:wireless-network-interfaces:

Wireless Network Interfaces
---------------------------

Wireless interfaces use direct sending [1]_ for communication instead of
links. As a result, their compound modules do not have output gates at the
physical layer; only an input gate dedicated to receiving is present. Another
difference from the wired case is that wireless interfaces require (and
collaborate with) a *transmission medium* module at the network level.
The medium module represents the shared transmission medium
(electromagnetic field or an acoustic medium) and is responsible for modeling
physical effects such as signal attenuation and maintaining connectivity
information. While wired interfaces can function without explicit
modeling of the physical layer, a PHY module is an indispensable part of
a wireless interface.

Wireless network interfaces are compound modules that implement the
:ned:`IWirelessInterface` interface. The following sections provide an overview
of the wireless interfaces available in INET.

.. _ug:sec:interfaces:generic-wireless-interface:

Generic Wireless Interface
~~~~~~~~~~~~~~~~~~~~~~~~~~

The :ned:`WirelessInterface` compound module is a generic implementation
of :ned:`IWirelessInterface`. The types of
the MAC protocol and PHY layer (the radio) are parameters in this network
interface:

.. code-block:: ned

   mac: <> like IMacProtocol;
   radio: <> like IRadio if typename != "";

There are specialized versions of :ned:`WirelessInterface` where the MAC
and the radio modules are fixed to a particular value. One example is
:ned:`BMacInterface`, which contains a :ned:`BMac` and an
:ned:`ApskRadio`.

.. _ug:sec:interfaces:ieee-80211:

IEEE 802.11
~~~~~~~~~~~

Chapter :doc:`ch-80211` covers IEEE 802.11 or Wi-Fi network interfaces
(:ned:`Ieee80211Interface`), as well as models of devices acting as access
points (AP).

.. _ug:sec:interfaces:ieee-802154:

IEEE 802.15.4
~~~~~~~~~~~~~

A separate chapter, :doc:`ch-802154`, covers :ned:`Ieee802154NarrowbandInterface`.

.. _ug:sec:interfaces:wireless-sensor-networks:

Wireless Sensor Networks
~~~~~~~~~~~~~~~~~~~~~~~~

Chapter :doc:`ch-sensor-macs` covers MAC protocols for wireless sensor networks
(WSNs) and the corresponding network interfaces.

.. _ug:sec:interfaces:csma/ca:

CSMA/CA
~~~~~~~

:ned:`CsmaCaMac` implements an imaginary CSMA/CA-based MAC protocol with
optional acknowledgements and a retry mechanism. With the appropriate
settings, it can approximate basic 802.11b ad-hoc mode operation.

:ned:`CsmaCaMac` provides a lot of room for experimentation:
acknowledgements can be turned on/off, and operation parameters like
inter-frame gap sizes, backoff behavior (slot time, minimum and maximum
number of slots), maximum retry count, header and ACK frame sizes, bit
rate, etc. can be configured via NED parameters.

:ned:`CsmaCaInterface` interface is a :ned:`WirelessInterface` with the
MAC type set to :ned:`CsmaCaMac`.

.. _ug:sec:interfaces:acking-mac:

Acking MAC
~~~~~~~~~~

Not every simulation requires a detailed simulation of the lower layers.
:ned:`AckingWirelessInterface` is a highly abstracted wireless interface that
offers simplicity in scenarios where Layer 1 and 2 effects can be
completely ignored (e.g., testing the basic functionality of a wireless ad-hoc
routing protocol).

:ned:`AckingWirelessInterface` is a :ned:`WirelessInterface` parameterized to
contain a unit disk radio (:ned:`GenericUnitDiskRadio`) and a
trivial MAC protocol (:ned:`AckingMac`).

The most important parameter :ned:`GenericUnitDiskRadio` accepts is the
transmission range. When a radio transmits a frame, all other radios
within transmission range are able to receive the frame correctly, and
radios that are out of range will not be affected at all. Interference
modeling (collisions) is optional, and it is recommended to turn it off
with :ned:`AckingMac`.

:ned:`AckingMac` implements a trivial MAC protocol that has packet
encapsulation and decapsulation, but no real medium access procedure.
Frames are simply transmitted on the wireless channel as soon as the
transmitter becomes idle. There is no carrier sense, collision
avoidance, or collision detection. :ned:`AckingMac` also provides an
optional out-of-band acknowledgement mechanism (using C++ function
calls, not actual wirelessly sent frames), which is turned on by
default. There is no retransmission: if the acknowledgement does not
arrive after the first transmission, the MAC gives up and counts the
packet as a failed transmission.

.. _ug:sec:interfaces:shortcut:

Shortcut
~~~~~~~~

:ned:`ShortcutMac` supports error-free "teleportation" of packets to
the peer MAC entity, with delay computed from transmission duration and propagation delay.
The physical layer is completely bypassed in this model. The corresponding network
interface type, :ned:`ShortcutInterface`, does not have a radio model.

:ned:`ShortcutInterface` is useful for modeling wireless networks where
full connectivity is assumed, and Layer 1 and Layer 2 effects can be
completely ignored.

.. _ug:sec:interfaces:special-purpose-network-interfaces:

Special-Purpose Network Interfaces
----------------------------------

.. _ug:sec:interfaces:tunnelling:

Tunnelling
~~~~~~~~~~

:ned:`TunInterface` is a virtual network interface that can be used for
creating tunnels. However, it is more powerful than that. It allows an
application-layer module to capture packets sent to the TUN interface and
perform any desired action on them, including sending them to a peer entity
in a UDP or plain IPv4 packet.

To set up a tunnel, add an instance of :ned:`TunnelApp` to the node, and
specify the protocol (IPv4 or UDP) and the remote endpoint of the tunnel
(address and port) in the parameters.

.. _ug:sec:interfaces:local-loopback:

Local Loopback
~~~~~~~~~~~~~~

:ned:`LoopbackInterface` provides local loopback within the network
node.

.. _ug:sec:interfaces:external-interface:

External Interfaces
~~~~~~~~~~~~~~~~~~~

:ned:`ExtLowerEthernetInterface` represents a real-world interface suitable for
hardware-in-the-loop simulations. External interfaces are explained in
Chapter :doc:`ch-emulation`.

.. _ug:sec:interfaces:custom-network-interfaces:

Custom Network Interfaces
-------------------------

It is also possible to build custom network interfaces. The following
example shows how to build a custom wireless interface.

.. literalinclude:: lib/Snippets.ned
   :language: ned
   :start-after: !WirelessInterfaceExample
   :end-before: !End
   :name: Wireless interface example

The above network interface contains very simple hypothetical MAC and
PHY protocols. The MAC protocol provides only acknowledgment without
other services (e.g., carrier sense, collision avoidance, collision
detection), and the PHY protocol uses one of the predefined APSK modulations
for the entire signal (preamble, header, and data) without other services
(e.g., scrambling, interleaving, forward error correction).

.. [1]
   OMNeT++ ``sendDirect()`` calls


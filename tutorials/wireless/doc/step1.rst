Step 1. Two hosts communicating wirelessly
==========================================

Goals
-----

In the first step, we want to create a network that contains two hosts,
with one host sending a UDP data stream wirelessly to the other. Our
goal is to keep the physical layer and lower layer protocol models as
simple as possible.

We'll make the model more realistic in later steps.

The model
---------

In this step, we'll use the model depicted below.

.. figure:: media/wireless-step1.png
   :width: 100%

Here is the NED source of the network:



.. literalinclude:: ../WirelessA.ned
   :language: ned
   :start-at: network WirelessA

We'll explain the above NED file below.

The playground
~~~~~~~~~~~~~~

The model contains a playground of the size 500x650 meters, with two
hosts spaced 400 meters apart. (The distance will be relevant in later
steps.) These numbers are set via display strings.

The modules that are present in the network in addition to the hosts are
responsible for tasks like visualization, configuring the IP layer, and
modeling the physical radio channel. We'll return to them later.

The hosts
~~~~~~~~~

In INET, hosts are usually represented with the :ned:`StandardHost` NED
type, which is a generic template for TCP/IP hosts. It contains protocol
components like TCP, UDP and IP, slots for plugging in application
models, and various network interfaces (NICs). :ned:`StandardHost` has some
variations in INET, for example, :ned:`WirelessHost`, which is basically a
:ned:`StandardHost` preconfigured for wireless scenarios.

As you can see, the hosts' type is parametric in this NED file (defined
via a ``hostType`` parameter and the :ned:`INetworkNode` module interface).
This is done so that in later steps we can replace hosts with a
different NED type. The actual NED type here is :ned:`WirelessHost` (given
near the top of the NED file), and later steps will override this
setting using ``omnetpp.ini``.

Address assignment
~~~~~~~~~~~~~~~~~~

IP addresses are assigned to hosts by an :ned:`Ipv4NetworkConfigurator`
module, which appears as the ``configurator`` submodule in the network.
The hosts also need to know each others' MAC addresses to communicate,
which in this model is taken care of by using per-host :ned:`GlobalArp`
modules instead of real ARP.

Traffic model
~~~~~~~~~~~~~

In the model, host A generates UDP packets that are received by host B.
To this end, host A is configured to contain a :ned:`UdpBasicApp` module,
which generates 1000-byte UDP messages at random intervals with
exponential distribution, the mean of which is 12ms. Therefore the app
is going to generate 100 kbyte/s (800 kbps) UDP traffic, not counting
protocol overhead. Host B contains a :ned:`UdpSink` application that just
discards received packets.

The model also displays the number of packets received by host B. The
text is added by the ``@figure[rcvdPkText]`` line, and the subsequent
line arranges the figure to be updated during the simulation.

Physical layer modeling
~~~~~~~~~~~~~~~~~~~~~~~

Let us concentrate on the module called ``radioMedium``. All wireless
simulations in INET need a radio medium module. This module represents
the shared physical medium where communication takes place. It is
responsible for taking signal propagation, attenuation, interference,
and other physical phenomena into account.

INET can model the wireless physical layer at various levels of detail,
realized with different radio medium modules. In this step, we use
:ned:`UnitDiskRadioMedium`, which is the simplest model. It implements a
variation of unit disc radio, meaning that physical phenomena like
signal attenuation are ignored, and the communication range is simply
specified in meters. Transmissions within range are always correctly
received unless collisions occur. Modeling collisions (overlapping
transmissions causing reception failure) and interference range (a range
where the signal cannot be received correctly, but still collides with
other signals causing their reception to fail) are optional.

NOTE: Naturally, this model of the physical layer has little
correspondence to reality. However, it has its uses in the simulation.
Its simplicity and consequent predictability are an advantage in
scenarios where realistic modeling of the physical layer is not a
primary concern, for example in the modeling of ad-hoc routing
protocols. Simulations using :ned:`UnitDiskRadioMedium` also run faster
than more realistic ones, due to the low computational cost.

In hosts, network interface cards are represented by NIC modules. Radio
is part of wireless NIC modules. There are various radio modules, and
one must always use one that is compatible with the medium module. In
this step, hosts contain :ned:`UnitDiskRadio` as part of
``AckingWirelessInterface``.

In this model, we configure the chosen physical layer model
(:ned:`UnitDiskRadioMedium` and :ned:`UnitDiskRadio`) as follows. The
communication range is set to 500m. Modeling packet losses due to
collision (termed "interference" in this model) is turned off, resulting
in pairwise independent duplex communication channels. The radio data
rates are set to 1 Mbps. These values are set in ``omnetpp.ini`` with
the ``communicationRange``, ``ignoreInterference``, and ``bitrate``
parameters of the appropriate modules.

MAC layer
~~~~~~~~~

NICs modules also contain an L2 (i.e. data link layer) protocol. The MAC
protocol in ``AckingWirelessInterface`` is configurable, the default choice
being ``MultipleAccessMac``. ``MultipleAccessMac`` implements a trivial
MAC layer which only provides encapsulation/decapsulation but no real
medium access protocol. There is virtually no medium access control:
packets are transmitted as soon as the previous packet has completed
transmission. ``MultipleAccessMac`` also contains an optional
out-of-band acknowledgment mechanism which we turn off here.

The configuration:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config Wireless01]
   :end-before: #---

Results
-------

When we run the simulation, here's what happens. Host A's
:ned:`UdpBasicApp` generates UDP packets at random intervals. These packets
are sent down via UDP and IPv4 to the network interface for
transmission. The network interface queues packets and transmits them
as soon as it can. As long as there are packets in the network
interface's transmission queue, packets are transmitted back-to-back,
with no gaps between subsequent packets.

These events can be followed on OMNeT++'s Qtenv runtime GUI. The
following image has been captured from Qtenv, and shows the inside of
host A during the simulation. One can see a UDP packet being sent down
from the ``udpApp`` submodule, traversing the intermediate protocol
layers, and being transmitted by the wlan interface.

.. video:: media/step1_10.mp4
   :width: 396
   :height: 462

The next animation shows the communication between the hosts, using
OMNeT++'s default "message sending" animation.

.. video:: media/step1_1.mp4
   :width: 655
   :height: 575

When the simulation concludes at t=25s, the packet count meter indicates
that around 2000 packets were sent. A packet with overhead is 1028
bytes, which means the transmission rate was around 660 kbps.

**Number of packets received by host B: 2017**

Sources: :download:`omnetpp.ini <../omnetpp.ini>`,
:download:`WirelessA.ned <../WirelessA.ned>`

Discussion
----------

Use `this
page <https://github.com/inet-framework/inet-tutorials/issues/1>`__ in
the GitHub issue tracker for commenting on this tutorial.

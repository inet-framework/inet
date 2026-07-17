MAC Protocols for Wireless Sensor Networks (Redesigned)
========================================================

Goals
-----

INET has several MAC protocol implementations designed specifically for wireless sensor
networks, in addition to IEEE 802.15.4 models. This showcase demonstrates three
different such MAC protocols through example simulations. The showcase compares the
performance of the three protocols using statistical analysis.

| Verified with INET version: ``4.6``
| Source files location: `inet/showcases/wireless/sensornetwork <https://github.com/inet-framework/inet/tree/master/showcases/wireless/sensornetwork>`__

.. admonition:: In one minute

   -  Three wireless sensor network (WSN) MAC protocols are compared: B-MAC and
      X-MAC (CSMA-based, using low-power listening) and LMAC (TDMA-based). Four
      sensors each send a 10-byte UDP "temperature data" payload (38 bytes at
      the MAC level) every second to a server, via a gateway.
   -  Each MAC's slot duration is swept from 10 ms to 1 s in 10 ms increments,
      optimizing for the number of packets received by the server. Best values:
      0.19 s for B-MAC, 0.04 s for X-MAC (gateway; 0.1 s for the sensors), and
      40 ms for LMAC.
   -  With the best values, LMAC and X-MAC carry all 100 packets sent during
      the 25 s simulations, while B-MAC has some packet loss — and consumes
      significantly more power. X-MAC turns out to be the most energy-efficient
      MAC protocol in this scenario.
   -  Roadmap: Part 1 explains the three protocols and shows each one operating
      in a simulated warehouse; Part 2 tunes the slot durations and compares
      packets received and power consumption.

.. figure:: media/powerconsumptionperpacket.png
   :width: 80%

**With each MAC tuned to its best slot duration, X-MAC spends the least energy
per delivered packet — making it the most energy-efficient of the three
protocols in this scenario.**

Part 1: Demonstrating the MAC protocols
---------------------------------------

There are two main categories of MAC protocols for WSNs, according to
how the MAC manages when certain nodes can communicate on the channel:

-  ``Time-division multiple access (TDMA) based``: These protocols
   assign different time slots to nodes. Nodes can send messages only in
   their time slot, thus eliminating contention. Examples of this kind
   of MAC protocols include LMAC, TRAMA, etc.
-  ``Carrier-sense multiple access (CSMA) based``: These protocols use
   carrier sensing and backoffs to avoid collisions, similar to IEEE
   802.11. Examples include B-MAC, SMAC, TMAC, X-MAC.

This showcase demonstrates the WSN MAC protocols available in INET:
B-MAC, LMAC, and X-MAC. The following sections detail these protocols
briefly.

B-MAC
~~~~~

B-MAC (short for Berkeley MAC) is a widely used WSN MAC protocol; it is
part of TinyOS. It employs low-power listening (LPL) to minimize power
consumption due to idle listening:

-  Nodes have a sleep period, after which they wake up and sense the
   medium for preambles (clear channel assessment — CCA). If none is
   detected, the nodes go back to sleep.
-  If there is a preamble, the nodes stay awake and receive the data
   packet after the preamble.
-  If a node wants to send a message, it first sends a preamble for at
   least the sleep period to allow all nodes to detect it. After the
   preamble, it sends the data packet. There are optional
   acknowledgments as well.
-  After the data packet (or data packet + ACK) exchange, the nodes go
   back to sleep.

Note that the preamble doesn't contain addressing information. Since
the recipient's address is contained in the data packet, all nodes
within the sender's communication range receive the preamble and the
data packet — not just the intended recipient of the data packet.

X-MAC
~~~~~

X-MAC is a development on B-MAC and aims to improve on some of B-MAC's
shortcomings. In B-MAC, the entire preamble is transmitted, regardless
of whether the destination node woke up at the beginning of the
preamble or the end. Furthermore, with B-MAC, all nodes receive both
the preamble and the data packet. X-MAC changes this in two ways:

-  It employs a strobed preamble, i.e. sending the preamble in shorter
   bursts with pauses in between. The pauses are long enough that the
   destination node can send an acknowledgment if it is already awake.
   When the sender receives the acknowledgment, it stops sending
   preambles and sends the data packet. This mechanism can save time
   because potentially, the sender doesn't have to send the whole
   length of the preamble.
-  The preamble contains the address of the destination node. Nodes can
   wake up, receive the preamble, and go back to sleep if the packet is
   not addressed to them.

These features improve B-MAC's power efficiency by reducing the nodes'
time spent in idle listening.

LMAC
~~~~

LMAC (short for lightweight MAC) is a TDMA-based MAC protocol. It uses
timeframes divided into time slots for data transmission. The number of
time slots in a timeframe is configurable according to the number of
nodes in the network. Each node has its own time slot, in which only
that particular node can transmit. This feature saves power, as there
are no collisions or retransmissions.

Data transfer works as follows:

-  A transmission consists of a control message and a data unit. The
   control message contains the destination of the data, the length of
   the data unit, and information about which time slots are occupied.
-  All nodes wake up at the beginning of each time slot. If there is no
   transmission, the time slot is assumed to be empty (not owned by any
   nodes), and the nodes go back to sleep.
-  If there is a transmission, upon receiving the control message,
   nodes that are not the intended recipient go back to sleep. The
   recipient node and the sender node go back to sleep after
   receiving/sending the transmission. Only one message can be sent in
   each time slot.

In the first five timeframes, the network is set up and no data packets
are sent. The network is set up by nodes claiming a time slot: they
send a control message in the time slot they want to reserve. If there
are no collisions, nodes note that the time slot is claimed. If there
are multiple nodes trying to claim the same time slot, and there is a
collision, they randomly choose another unclaimed time slot.

The INET implementations
~~~~~~~~~~~~~~~~~~~~~~~~

The three MACs are implemented in INET as the :ned:`BMac`, :ned:`XMac`, and
:ned:`LMac` modules. They have parameters to adapt the MAC protocol to the
size of the network and the traffic intensity, such as slot time, clear
channel assessment duration, bitrate, etc.

The parameters have default values, so the MAC modules can be used
without setting any of their parameters. Check the NED files of the MAC
modules (``BMac.ned``, ``XMac.ned``, and ``LMac.ned``) to see all
parameters.

The MACs don't have corresponding physical layer models. They can be
used with existing generic radio models in INET, such as
:ned:`GenericRadio` or :ned:`ApskRadio`. This showcase uses :ned:`ApskRadio`
because it is more realistic than :ned:`GenericRadio`.

INET doesn't have WSN routing protocol models (such as Collection Tree
Protocol), so IPv4 and static routing are used in this showcase.

Configuration
~~~~~~~~~~~~~

The showcase contains three example simulations that demonstrate the
three MACs in a wireless sensor network. The scenario is that there are
wireless sensor nodes in a refrigerated warehouse, monitoring the
temperature at their location. They periodically transmit temperature
data wirelessly to a gateway node, which forwards the data to a server
via a wired connection.

.. admonition:: Details — in WSN terminology, the gateway is a "sink"

   Ideally, there should be a specific application in the gateway node
   called ``sink``, which receives the data from the WSN and sends it
   to the server over IP. Thus the node acts as a gateway between the
   WSN and the external IP network. In the example simulations, the
   gateway only forwards the data packets over IP.

To run the example simulations, choose the :ned:`BMac`, :ned:`LMac`, and
:ned:`XMac` configurations from :download:`omnetpp.ini <../omnetpp.ini>`.
Most of the configuration keys in the .ini file are shared between the
three simulations (they are defined in the ``General`` configuration),
except for the MAC protocol-specific settings. All three simulations
will use the same network, :ned:`SensorNetworkShowcaseA`, defined in
:download:`SensorNetworkShowcase.ned <../SensorNetworkShowcase.ned>`.

.. figure:: media/network.png
   :width: 100%

**Four wireless sensor nodes and the gateway communicate wirelessly;
the gateway forwards the sensor data to the server over a wired
connection.**

In the network, the wireless sensor nodes are of the type
:ned:`SensorNode`, named ``sensor1`` up to ``sensor4``, and ``gateway``.
The node named ``server`` is a :ned:`StandardHost`. The network also
contains an :ned:`Ipv4NetworkConfigurator`, an
:ned:`IntegratedCanvasVisualizer`, and an :ned:`RadioMedium` module.

.. admonition:: Fine print — the warehouse backdrop is only decoration

   The nodes are placed against the backdrop of a warehouse floorplan.
   The scene size is 60x30 meters. The warehouse is just a background
   image providing context. Obstacle loss is not modeled, so the
   background image doesn't affect the simulation.

The wireless interface in the sensor nodes and the gateway is specified
in :download:`omnetpp.ini <../omnetpp.ini>` to be the generic
:ned:`WirelessInterface` (instead of the IEEE 802.15.4 specific
:ned:`Ieee802154NarrowbandInterface`, which is the default WLAN interface
in :ned:`SensorNode`). The radio type is set to :ned:`ApskRadio`,
and configured to use the scalar analog model.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: typename
   :end-at: radio.signalAnalogRepresentation

Note that the wireless interface module's name is ``wlan`` in all host
types that have a wireless interface. The term doesn't imply that it's
WiFi but stands for wireless LAN.

We set the bitrate in :download:`omnetpp.ini <../omnetpp.ini>` to
19200 bps to match the default MAC bitrates (which is 19200 bps for all
three MAC types). The :par:`preambleDuration` is set to be very short for
better compatibility with the MACs. We also set some other parameters
of the radio to arbitrary values.

.. admonition:: Details — why ApskRadio and the scalar analog model

   We are using :ned:`ApskRadio` here because it is a relatively simple,
   generic radio. It uses amplitude and phase-shift keying modulations
   (e.g. BPSK, QAM-16 or QAM-64, BPSK by default), without additional
   features such as forward error correction, interleaving or
   spreading. Also, the scalar analog model is adequate in this
   scenario, because we're not interested in the details of signal
   reception.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: centerFrequency
   :end-at: snirThreshold

Routes are set up according to a star topology, with the gateway at the
center. This is achieved by dumping the full configuration of
:ned:`Ipv4NetworkConfigurator` (which was generated with the configurator's
default settings) and then modifying it. The modified configuration is
in the :download:`config.xml <../config.xml>` file. The following
image shows the routes.

.. figure:: media/routes.png
   :width: 100%

**All sensor traffic reaches the server through the gateway — a star
topology with the gateway at the center.**

Each sensor node will send a UDP packet with a 10-byte payload
("temperature data") every second to the server, with a random start
time around 1s. The packets will have an 8-byte UDP header and a 20-byte
IPv4 header, so they will be 38 bytes at the MAC level. The packets will
be routed via the gateway. Here are the application settings in
:download:`omnetpp.ini <../omnetpp.ini>`.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: numApps
   :end-at: localPort

The MAC-specific parameters are set in the configurations for the
individual MACs.

For B-MAC, the wireless interface's mac module type is set to
:ned:`BMac`. Also, the :par:`slotDuration` parameter is set to 0.025s (an
arbitrary value). This parameter is essentially the nodes' sleep
duration. Here is the configuration in
:download:`omnetpp.ini <../omnetpp.ini>`.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Config BMac
   :end-at: slotDuration

For X-MAC, the wireless interface's mac module type is set to
:ned:`XMac`. The MAC's :par:`slotDuration` parameter determines the duration
of the nodes' sleep periods. It is set to 0.25s for the sensor nodes
and 0.1s for the gateway.

The design of X-MAC allows setting different sleep intervals for
different nodes, as long as the sender node's sleep interval is greater
than the receiver's. Nodes transmit preambles for the duration of their
own sleep periods unless interrupted by an acknowledgment from the
destination node. We set the slot duration of the gateway to a shorter
value because it has to receive and relay data from all sensors, so it
has more traffic. Here is the configuration in
:download:`omnetpp.ini <../omnetpp.ini>`.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Config XMac
   :end-at: sensor

For LMAC, the wireless interface's mac module type is set to
:ned:`LMac`. The :par:`numSlots` parameter is set to 8, as it is sufficient
(there are only five nodes in the wireless sensor network). The
:par:`slotDuration` parameter is set to 50ms, and :par:`reservedMobileSlots`
to 0. Here is the configuration in
:download:`omnetpp.ini <../omnetpp.ini>`.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Config LMac
   :end-at: reservedMobileSlots

.. admonition:: Details — LMAC slot bookkeeping and setup time

   -  The :par:`reservedMobileSlots` parameter reserves some of the slots
      for mobile nodes; these slots are not chosen by any of the nodes
      during network setup. The parameter's default value is 2, but
      here it is set to 0.
   -  The :par:`slotDuration` parameter's default value is 100ms, but we
      set it to 50ms to decrease the network setup time.
   -  The duration of a timeframe will be 400ms (number of slots \*
      slot duration). The network is set up in the first five frames,
      i.e. the first 2 seconds.

The next sections demonstrate the three simulations.

B-MAC
~~~~~

The following video shows sensor nodes sending data to the server.

.. video:: media/BMac2.mp4
   :width: 100%

   <!--internal video recording, zoom 20.28, animation speed none, playback speed 1.698, normal run, crop 50 50 130 130-->

**All nodes — not just the gateway — receive sensor3's preambles and
data packet, because B-MAC preambles carry no addressing information.**

:ned:`BMac` actually sends multiple shorter preambles instead of a long
one, so that waking nodes can receive the one that starts after they
wake up.

In the video, ``sensor3`` starts sending preambles while the other
nodes are asleep. All of them wake up before the end of the preamble
transmission. When the nodes are awake, they receive the preamble and
the data packet at the physical layer (the MAC discards it if it is not
intended for them). Then the gateway sends it to the server.

X-MAC
~~~~~

In the following video, the sensors send data to the server.

.. video:: media/XMac2.mp4
   :width: 100%

   <!--internal video recording, zoom 20.28, animation speed none, playback speed 1.698, normal run, crop 50 50 130 130-->

**The gateway's acknowledgment cuts the strobed preamble short, and
sensor4 — for which the transmission is not intended — goes back to
sleep early.**

``sensor3`` starts sending preambles. ``sensor4`` wakes up and receives
one of the preambles (hence the dotted arrow representing a successful
physical layer transmission), and goes back to sleep, as the
transmission is addressed to the gateway. Then the gateway wakes up and
sends an acknowledgment after receiving one of the preambles.
``sensor3`` sends the data packet, and the gateway forwards it to the
server.

LMAC
~~~~

In the following video, sensor nodes send data to the server.

.. video:: media/LMac5.mp4
   :width: 100%

   <!--internal video recording, zoom 20.28, animation speed none, playback speed 1.698, normal run, crop 50 50 130 130-->

**Each node transmits in its own time slot; there are no collisions.**

Note that all nodes receive the control message (indicated by the
physical link visualizer arrows), but only the destination node
receives the data unit.

Part 2: Optimizing for packet loss and comparing power consumption
------------------------------------------------------------------

In this section, we'll compare the three MAC protocols in terms of a few
statistics, such as the number of UDP packets carried by the network,
and power consumption. In order to compare the three protocols, we want
to find the parameter values for each MAC that lead to the best
performance of the network in a particular scenario. We'll optimize for
the number of packets received by the server, i.e. we want to minimize
packet loss.

The scenario will be the same as in the :ned:`BMac`, :ned:`XMac`, and :ned:`LMac`
configurations (each sensor sending data every second to the server),
except that it will use a similar, but more generic network layout
instead of the warehouse network.

.. figure:: media/statisticnetwork.png
   :width: 60%
   :align: center

**Part 2 replaces the warehouse network with this more generic layout —
same traffic, same star routing through the gateway.**

We'll run three parameter studies, one for each MAC protocol, and
optimize just one parameter of each MAC: the slot duration. We'll
choose the best performing parameters according to the number of
packets received by the server.

.. admonition:: Fine print — scope of the optimization

   Ideally, one would want to optimize multiple parameters to find a
   more optimal set of parameter values, but it is out of scope for
   this showcase. The choices for the values of the other parameters
   are arbitrary.

The parameter study configurations for the three MAC protocols will
extend the ``StatisticBase`` config (as well as the ``General``
configuration).

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Config StatisticBase
   :end-at: repeat

In this base configuration, we set the simulation time limit, the number
of repetitions, and turn vector recording off to speed up the runs.
The parameter studies for the individual MACs are detailed in the
following sections.

Optimizing B-MAC
~~~~~~~~~~~~~~~~

The goal is to optimize :ned:`BMac`'s :par:`slotDuration` parameter for the number
of packets received by the server. The configuration in
:download:`omnetpp.ini <../omnetpp.ini>` for this is
``StatisticBMac``. Here is the configuration.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: StatisticBMac
   :end-at: slotDuration

In the study, :par:`slotDuration` will run from 10ms to 1s in 10ms
increments (the default of :par:`slotDuration` is 100ms). The number of
packets received by the server for each :par:`slotDuration` value is shown
on the following image (time in seconds).

.. figure:: media/bmac.png
   :width: 100%

**The best performing value for slotDuration is 0.19s — but even then,
the network cannot carry all the traffic in this scenario.**

The sensors send 25 packets each during the 25s, thus 100 packets in
total. The results outline a smooth curve.

Optimizing X-MAC
~~~~~~~~~~~~~~~~

Again, we optimize the :par:`slotDuration` parameter for the number of packets
received by the server. As in the :ned:`XMac` configuration, the
``slotDuration`` for the gateway will be shorter than for the sensors. The
configuration in :download:`omnetpp.ini <../omnetpp.ini>` for this is
``StatisticXMac``. Here is the configuration.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: StatisticXMac
   :end-at: slotDuration

The default of :par:`slotDuration` for :ned:`XMac` is 100ms. In the study, the
gateway's :par:`slotDuration` will run from 10ms to 1s in 10ms increments,
similarly to the parameter study for B-MAC. The :par:`slotDuration` for the
sensors will be 2.5 times that of the gateway (an arbitrary value). Here
are the results (time in seconds).

.. figure:: media/xmac.png
   :width: 100%

**The optimal value for the gateway's slotDuration is 0.04s (0.1s for
the sensors).**

Optimizing LMAC
~~~~~~~~~~~~~~~

We'll optimize the :par:`slotDuration` parameter for the number of packets
received by the server. The configuration for this study in
:download:`omnetpp.ini <../omnetpp.ini>` is ``StatisticLMac``.
Here is the configuration.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: StatisticLMac
   :end-at: reservedMobileSlots

We set :par:`reservedMobileSlots` to 0, and :par:`numSlots` to 8. The
:par:`slotDuration` parameter will run from 10ms to 1s in 10ms steps. The
number of received packets is displayed on the following image (time in
seconds).

.. figure:: media/lmac.png
   :width: 100%

**Unlike with B-MAC and X-MAC, the network can carry almost all the
traffic; the best performing value for slotDuration is 40ms.**

.. admonition:: Fine print — why 40ms and not a longer slot

   The lowest :par:`slotDuration` values up until 120ms yield
   approximately the same results (around 100 packets), with the 40ms
   value performing marginally better. Choosing the higher
   :par:`slotDuration` value would result in about the same performance
   but lower power consumption, but we are optimizing for the number of
   packets here.

Measuring power consumption
~~~~~~~~~~~~~~~~~~~~~~~~~~~

We will examine the three simulations with the chosen parameters in
terms of power consumption. We want to record the power consumption of
the radios in the wireless nodes. The :ned:`SensorNode` host type
contains the required energy modules by default:

-  **Energy consumer**: :ned:`SensorStateBasedEpEnergyConsumer`, a
   submodule of the radio. This module assigns a constant power
   consumption value to each radio mode and transmitter/receiver
   state. It has default power consumption values typical for wireless
   sensor nodes, thus we leave the module's parameters on default.
-  **Energy storage**: :ned:`IdealEpEnergyStorage`. This module keeps
   track of the node's energy consumption. It stores an infinite
   amount of energy, and cannot get fully charged or depleted, and it
   is useful if we only want to model and measure power consumption,
   but not energy capacity, charging, etc.

.. admonition:: Fine print — where the consumption values come from

   The values for each state are parameters of the energy consumer
   module and don't come from other modules (such as the radio), so
   they need to be set correctly to obtain accurate power consumption
   results.

The results for the parameter studies contain the required power
consumption data. We just need to select the result files corresponding
to the best performing parameter values selected in the previous
sections. We will plot the results on bar charts. We'll examine the
following statistics:

-  ``Total number of packets received``: All the packets received by the
   server. The UDP applications in the sensors each send 25 packets
   during the 25s simulations, for a total of 100
   packets. As there are 100 packets, this value is also the successful packet reception in percent,
   and indirectly, packet loss.
-  ``Network total power consumption``: The sum of the power consumption
   of the four sensors and the gateway (values in Joules).
-  ``Power consumption per packet``: Network total power consumption /
   Total number of packets received, thus power consumption per packet
   in the entire network (values in Joules).

Note that the values for the ``residualEnergyCapacity`` statistic are
negative, so they are inverted in the graph. Here are the results.

.. figure:: media/packetsreceived.png
   :width: 80%

**LMac and XMac carried all 100 packets; BMac had some packet loss.**

.. figure:: media/powerconsumption.png
   :width: 80%

**BMac consumed significantly more power than the other two MACs.**

.. figure:: media/powerconsumptionperpacket.png
   :width: 80%

**BMac also has significantly more power consumption per packet.**

The conclusion is that in this scenario, with the selected parameter
values, :ned:`XMac` turned out to be the most energy efficient MAC
protocol.

Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`SensorNetworkShowcase.ned <../SensorNetworkShowcase.ned>`, :download:`config.xml <../config.xml>`

Further information
-------------------

Here are the documents describing the MAC protocols and the INET
implementations:

-  `Versatile Low Power Media Access for Wireless Sensor Networks
   (B-MAC) <https://people.eecs.berkeley.edu/~culler/papers/sensys04-bmac.pdf>`__
-  `Implementation of the B-MAC Protocol for WSN in
   MiXiM <https://www.researchgate.net/publication/235930503_Code_Contribution_Implementation_of_the_B-MAC_Protocol_for_WSN_in_MiXiM>`__
-  `X-MAC: A Short Preamble MAC Protocol for Duty-Cycled Wireless Sensor
   Networks <http://www.cs.cmu.edu/~andersoe/papers/xmac-sensys.pdf>`__
-  `Has Time Come to Switch From Duty-Cycled MAC Protocols to Wake-Up
   Radio for Wireless Sensor Networks?
   (X-MAC) <http://ieeexplore.ieee.org/document/7024195/>`__
-  `A Lightweight Medium Access Protocol (LMAC) for Wireless Sensor
   Networks <https://www.researchgate.net/publication/242150051_A_Lightweight_Medium_Access_Protocol_LMAC_for_Wireless_Sensor_Networks_Reducing_Preamble_Transmissions_and_Transceiver_State_Switches>`__


Try It Yourself
---------------

If you already have INET and OMNeT++ installed, start the IDE by typing
``omnetpp``, import the INET project into the IDE, then navigate to the
``inet/showcases/wireless/sensornetwork`` folder in the `Project Explorer`. There, you can view
and edit the showcase files, run simulations, and analyze results.

Otherwise, there is an easy way to install INET and OMNeT++ using `opp_env
<https://omnetpp.org/opp_env>`__, and run the simulation interactively.
Ensure that ``opp_env`` is installed on your system, then execute:

.. code-block:: bash

    $ opp_env run inet-4.6 --init -w inet-workspace --install --build-modes=release --chdir \
       -c 'cd inet-4.6.*/showcases/wireless/sensornetwork && inet'

This command creates an ``inet-workspace`` directory, installs the appropriate
versions of INET and OMNeT++ within it, and launches the ``inet`` command in the
showcase directory for interactive simulation.

Alternatively, for a more hands-on experience, you can first set up the
workspace and then open an interactive shell:

.. code-block:: bash

    $ opp_env install --init -w inet-workspace --build-modes=release inet-4.6
    $ cd inet-workspace
    $ opp_env shell

Inside the shell, start the IDE by typing ``omnetpp``, import the INET project,
then start exploring.

Discussion
----------

Use `this
page <https://github.com/inet-framework/inet-showcases/issues/24>`__ in
the GitHub issue tracker for commenting on this showcase.

MAC Protocols for Wireless Sensor Networks
==========================================

Goals
-----

INET has several MAC protocol implementations designed specifically for wireless sensor
networks, in addition to IEEE 802.15.4 models. This showcase demonstrates three
different such MAC protocols through example simulations. The showcase compares the
performance of the three protocols using statistical analysis.

| INET version: ``4.0``
| Source files location: `inet/showcases/wireless/sensornetwork <https://github.com/inet-framework/inet/tree/master/showcases/wireless/sensornetwork>`__

Part 1: Demonstrating the MAC protocols
---------------------------------------

.. todo::

   <!-- V1

   The devices that make up wireless sensor networks (WSNs) are often power constrained,
   and the networks have high latency and low throughput, as compared to WLANs, for example.
   There are two main categories of MAC protocols for WSNs: time-division multiple access based
   (TDMA), such as LMAC, and carrier-sense multiple access (CSMA) based, such as B-MAC and X-MAC. -->

There are two main categories of MAC protocols for WSNs, according to
how the MAC manages when certain nodes can communicate on the channel:

-  ``Time-division multiple access (TDMA) based``: These protocols
   assign different time slots to nodes. Nodes can send messages only in
   their time slot, thus eliminating contention. Examples of this kind
   of MAC protocols include LMAC, TRAMA, etc.
-  ``Carrier-sense multiple access (CSMA) based``: These protocols use
   carrier sensing and backoffs to avoid collisions, similarly to IEEE
   802.11. Examples include B-MAC, SMAC, TMAC, X-MAC.

This showcase demonstrates the WSN MAC protocols available in INET:
B-MAC, LMAC, and X-MAC. The following sections detail these protocols
briefly.

B-MAC
~~~~~

B-MAC (short for Berkeley MAC) is a widely used WSN MAC protocol; it is
part of TinyOS. It employs low-power listening (LPL) to minimize power
consumption due to idle listening. Nodes have a sleep period, after
which they wake up and sense the medium for preambles (clear channel
assessment - CCA.) If none is detected, the nodes go back to sleep. If
there is a preamble, the nodes stay awake and receive the data packet
after the preamble. If a node wants to send a message, it first sends a
preamble for at least the sleep period in order for all nodes to detect
it. After the preamble, it sends the data packet. There are optional
acknowledgments as well. After the data packet (or data packet + ACK)
exchange, the nodes go back to sleep. Note that the preamble doesn't
contain addressing information. Since the recipient's address is
contained in the data packet, all nodes receive the preamble and the
data packet in the sender's communication range (not just the intended
recipient of the data packet.)

X-MAC
~~~~~

X-MAC is a development on B-MAC and aims to improve on some of B-MAC's
shortcomings. In B-MAC, the entire preamble is transmitted, regardless of
whether the destination node awoke at the beginning of the preamble or
the end. Furthermore, with B-MAC, all nodes receive both the preamble
and the data packet. X-MAC employs a strobed preamble, i.e. sending the
same length preamble as B-MAC, but as shorter bursts, with pauses in
between. The pauses are long enough that the destination node can send
an acknowledgment if it is already awake. When the sender receives the
acknowledgment, it stops sending preambles and sends the data packet.
This mechanism can save time because potentially, the sender doesn't have to send
the whole length preamble. Also, the preamble contains the address of the
destination node. Nodes can wake up, receive the preamble, and go back
to sleep if the packet is not addressed to them. These features improve
B-MAC's power efficiency by decreasing nodes' time spent in idle
listening.

LMAC
~~~~

LMAC (short for lightweight MAC) is a TDMA-based MAC protocol. There are
data transfer timeframes, which are divided into time slots. The number
of time slots in a timeframe is configurable according to the number of
nodes in the network. Each node has its own time slot, in which only
that particular node can transmit. This feature saves power, as there are no
collisions or retransmissions. A transmission consists of a control
message and a data unit. The control message contains the destination of
the data, the length of the data unit, and information about which time
slots are occupied. All nodes wake up at the beginning of each time
slot. If there is no transmission, the time slot is assumed to be empty
(not owned by any nodes), and the nodes go back to sleep. If there is a
transmission, after receiving the control message, nodes that are not
the recipient go back to sleep. The recipient node and the sender node
goes back to sleep after receiving/sending the transmission. Only one
message can be sent in each time slot. In the first five timeframes, the
network is set up and no data packets are sent. The network is set up by
nodes claiming a time slot. They send a control message in the time slot
they want to reserve. If there are no collisions, nodes note that the
time slot is claimed. If there are multiple nodes trying to claim the
same time slot, and there is a collision, they randomly choose another
unclaimed time slot.

The INET implementations
~~~~~~~~~~~~~~~~~~~~~~~~

The three MACs are implemented in INET as the :ned:`BMac`, :ned:`XMac`, and
:ned:`LMac` modules. They have parameters to adapt the MAC protocol to the
size of the network and the traffic intensity, such as slot time, clear
channel assessment duration, bitrate, etc. The parameters have default
values, thus the MAC modules can be used without setting any of their
parameters. Check the NED files of the MAC modules (``BMac.ned``,
``XMac.ned``, and ``LMac.ned``) to see all parameters.

The MACs don't have corresponding physical layer models. They can be
used with existing generic radio models in INET, such as
:ned:`UnitDiskRadio` or :ned:`ApskRadio`. We're using :ned:`ApskRadio` in this
showcase because it is more realistic than :ned:`UnitDiskRadio`.

INET doesn't have WSN routing protocol models (such as Collection Tree
Protocol), so we're using Ipv4 and static routing.

Configuration
~~~~~~~~~~~~~

The showcase contains three example simulations, which demonstrate the
three MACs in a wireless sensor network. The scenario is that there are
wireless sensor nodes in a refrigerated warehouse, monitoring the
temperature at their location. They periodically transmit temperature
data wirelessly to a gateway node, which forwards the data to a server
via a wired connection.

Note that in WSN terminology, the gateway would be called sink. Ideally,
there should be a specific application in the gateway node called
``sink``, which would receive the data from the WSN, and send it to the
server over IP. Thus the node would act as a gateway between the WSN and
the external IP network. In the example simulations, the gateway just
forwards the data packets over IP.

To run the example simulations, choose the :ned:`BMac`, :ned:`LMac` and
:ned:`XMac` configurations from :download:`omnetpp.ini <../omnetpp.ini>`.
Most of the configuration keys in the ini file are shared between the
three simulations (they are defined in the ``General`` configuration),
except for the MAC protocol-specific settings. All three simulations
will use the same network, :ned:`SensorNetworkShowcaseA`, defined in
:download:`SensorNetworkShowcase.ned <../SensorNetworkShowcase.ned>`:

.. figure:: media/network.png
   :width: 100%

In the network, the wireless sensor nodes are of the type
:ned:`SensorNode`, named ``sensor1`` up to ``sensor4``, and ``gateway``.
The node named ``server`` is a :ned:`StandardHost`. The network also
contains an :ned:`Ipv4NetworkConfigurator`, an :ned:`IntegratedVisualizer`,
and an :ned:`ApskScalarRadioMedium` module. The nodes are placed against
the backdrop of a warehouse floorplan. The scene size is 60x30
meters. The warehouse is just a background image providing context.
Obstacle loss is not modeled, so the background image doesn't affect
the simulation in any way.

The wireless interface in the sensor nodes and the gateway is specified
in :download:`omnetpp.ini <../omnetpp.ini>` to be the generic
:ned:`WirelessInterface` (instead of the Ieee802154 specific
:ned:`Ieee802154NarrowbandInterface`, which is the default wlan interface
in :ned:`SensorNode`). The radio type is set to :ned:`ApskScalarRadio`:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: typename
   :end-at: radio.typename

Note that the wireless interface module's name is ``wlan`` in all host
types that have a wireless interface. The term doesn't imply that it's
Wifi but stands for wireless LAN.

We are using :ned:`ApskScalarRadio` here because it is a relatively
simple, generic radio. It uses amplitude and phase-shift keying
modulations (e.g. BPSK, QAM-16 or QAM-64, BPSK by default), without
additional features such as forward error correction, interleaving or
spreading. We set the bitrate in
:download:`omnetpp.ini <../omnetpp.ini>` to 19200 bps, to match the
default for the MAC bitrates (we'll use the default bitrate in the MACs,
which is 19200 bps for all three MAC types.) The :par:`preambleDuration` is
set to be very short for better compatibility with the MACs. We also set
some other parameters of the radio to arbitrary values:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: centerFrequency
   :end-at: snirThreshold

Routes are set up according to a star topology, with the gateway at the
center. This is achieved by dumping the full configuration of
:ned:`Ipv4NetworkConfigurator` (which was generated with the configurator's
default settings), and then modifying it. The modified configuration is
in the :download:`config.xml <../config.xml>` file. The following
image shows the routes:

.. figure:: media/routes.png
   :width: 100%

Each sensor node will send an UDP packet with a 10-byte payload
("temperature data") every second to the server, with a random start
time around 1s. The packets will have an 8-byte UDP header and a 20-byte
Ipv4 header, so they will be 38 bytes at the MAC level. The packets will
be routed via the gateway. Here are the application settings in
:download:`omnetpp.ini <../omnetpp.ini>`:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: numApps
   :end-at: localPort

The MAC-specific parameters are set in the configurations for the
individual MACs.

For B-MAC, the wireless interface's :par:`macType` parameter is set to
:ned:`BMac`. Also, the :par:`slotDuration` parameter is set to 0.025s (an
arbitrary value.) This parameter is essentially the nodes' sleep
duration. Here is the configuration in
:download:`omnetpp.ini <../omnetpp.ini>`:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Config BMac
   :end-at: slotDuration

For X-MAC, the wireless interface's :par:`macType` parameter is set to
:ned:`XMac`. The MAC's :par:`slotDuration` parameter determines the duration
of the nodes' sleep periods. It is set to 0.25s for the sensor nodes
and 0.1s for the gateway. Nodes transmit preambles for the duration of
their own sleep periods unless interrupted by an acknowledgment from
the destination node. The design of X-MAC allows setting different sleep
intervals for different nodes, as long as the sender node's sleep
interval is greater than the receiver's. (?). We set the slot duration
of the gateway to a shorter value because it has to receive and relay
data from all sensors, thus it has more traffic. Here is the
configuration in :download:`omnetpp.ini <../omnetpp.ini>`:


.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Config XMac
   :end-at: sensor

For LMAC, the wireless interface's :par:`macType` parameter is set to
:ned:`LMac`. The :par:`numSlots` parameter is set to 8, as it is sufficient
(there are only five nodes in the wireless sensor network.) The
:par:`reservedMobileSlots` parameter reserves some of the slots for mobile
nodes; these slots are not chosen by any of the nodes during network
setup. The parameter's default value is 2, but it is set to 0. The
:par:`slotDuration` parameter's default value is 100ms, but we set it to
50ms to decrease the network setup time. The duration of a timeframe
will be 400ms (number of slots \* slot duration.) The network is set up
in the first five frames, i.e. in the first 2 seconds. Here is the
configuration in :download:`omnetpp.ini <../omnetpp.ini>`:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Config LMac
   :end-at: reservedMobileSlots

The next sections demonstrate the three simulations.

B-MAC
~~~~~

The following video shows sensor nodes sending data to the server:

.. video:: media/BMac2.mp4
   :width: 100%

   <!--internal video recording, zoom 20.28, animation speed none, playback speed 1.698, normal run, crop 50 50 130 130-->

:ned:`BMac` actually sends multiple shorter preambles instead of a long
one, so that waking nodes can receive the one that starts after they
woke up. ``sensor3`` starts sending preambles, while the other nodes are
asleep. All of them wake up before the end of the preamble transmission.
When the nodes are awake, they receive the preamble, and receive the
data packet as well, at the physical layer (the mac discards it if it is
not for them.) Then the gateway sends it to the server. Note that all
nodes receive the preambles and the data packet as well.

X-MAC
~~~~~

In the following video, the sensors send data to the server:

.. video:: media/XMac2.mp4
   :width: 100%

   <!--internal video recording, zoom 20.28, animation speed none, playback speed 1.698, normal run, crop 50 50 130 130-->

``sensor3`` starts sending preambles. ``sensor4`` wakes up and receives
one of the preambles (hence the dotted arrow representing a successful
physical layer transmission), and goes back to sleep, as the
transmission is addressed to the gateway. Then the gateway wakes up and
sends an acknowledgment after receiving one of the preambles.
``sensor3`` sends the data packet, and the gateway forwards it to the
server.

LMAC
~~~~

In the following video, sensor nodes send data to the server:

.. video:: media/LMac5.mp4
   :width: 100%

   <!--internal video recording, zoom 20.28, animation speed none, playback speed 1.698, normal run, crop 50 50 130 130-->

Each node transmits in its own time slot; there are no collisions. Note
that all nodes receive the control message (indicated by the physical
link visualizer arrows), but only the destination node receives the data
unit.

Part 2: Optimizing for packet loss and comparing power consumption
------------------------------------------------------------------

In this section, we'll compare the three MAC protocols in terms of a few
statistics, such as the number of UDP packets carried by the network,
and power consumption. In order to compare the three protocols, we want
to find the parameter values for each MAC, which lead to the best
performance of the network in a particular scenario. We'll optimize for
the number of packets received by the server, i.e. we want to minimize
packet loss.

The scenario will be the same as in the :ned:`BMac`, :ned:`XMac` and :ned:`LMac`
configurations (each sensor sending data every second to the server),
except that it will use a similar, but more generic network layout
instead of the warehouse network:

.. figure:: media/statisticnetwork.png
   :width: 60%
   :align: center

We'll run three parameter studies, one for each MAC protocol. We want to
optimize just one parameter of each MAC, the slot duration. Ideally, one
would want to optimize multiple parameters to find a more
optimal set of parameter values, but it is out of scope for this
showcase. The choices for the values of the other parameters are
arbitrary. 

.. The simulations will be run for 100s, and each iteration will
   be run 10 times to get smoother results. 

We'll choose the best
performing parameters according to the number of packets received by the
server.

The parameter study configurations for the three MAC protocols will
extend the ``StatisticBase`` config (as well as the ``General``
configuration):

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Config StatisticBase
   :end-at: repeat

In this base configuration, we set the simulation time limit, the number
of repetitions, and turn vector recording off to speed up the runs.

The parameter
studies for the individual MACs are detailed in the following sections.

Optimizing B-MAC
~~~~~~~~~~~~~~~~

The goal is to optimize :ned:`BMac`'s :par:`slotTime` parameter for the number
of packets received by the server. The configuration in
:download:`omnetpp.ini <../omnetpp.ini>` for this is
``StatisticBMac``. Here is the configuration:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: StatisticBMac
   :end-at: slotDuration

In the study, :par:`slotDuration` will run from 10ms to 1s in 10ms
increments (the default of :par:`slotDuration` is 100ms.) The number of
packets received by the server for each :par:`slotDuration` value is shown
on the following image (time in seconds):

.. figure:: media/bmac.png
   :width: 100%

The sensors send 25 packets each during the 25s, thus
100 packets total. It is apparent from the results that the network
cannot carry all traffic in this scenario. The results also outline a
smooth curve. The best performing value for
:par:`slotDuration` is 0.19s.

Optimizing X-MAC
~~~~~~~~~~~~~~~~

Again, we optimize the :par:`slotTime` parameter for the number of packets
received by the server. As in the :ned:`XMac` configuration, the
``slotTime`` for the gateway will be shorter than for the sensors. The
configuration in :download:`omnetpp.ini <../omnetpp.ini>` for this is
``StatisticXMac``. Here is the configuration:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: StatisticXMac
   :end-at: slotDuration

The default of :par:`slotDuration` for :ned:`XMac` is 100ms. In the study, the
gateway's :par:`slotDuration` will run from 10ms to 1s in 10ms increments,
similarly to the parameter study for B-MAC. The :par:`slotDuration` for the
sensors will be 2.5 times that of the gateway (an arbitrary value.) Here
are the results (time in seconds):

.. figure:: media/xmac.png
   :width: 100%

According to this, the optimal value for the gateway's :par:`slotDuration`
is 0.04s (0.1s for the sensors).

Optimizing LMAC
~~~~~~~~~~~~~~~

We'll optimize the :par:`slotDuration` parameter for the number of packets
received by the server. The configuration for this study in
:download:`omnetpp.ini <../omnetpp.ini>` is ``StatisticLMac``. 
Here is the configuration:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: StatisticLMac
   :end-at: reservedMobileSlots

We set :par:`reservedMobileSlots` to 0, and :par:`numSlots` to 8. The
:par:`slotDuration` parameter will run from 10ms to 1s in 10ms steps. The
number of received packets are displayed on the following image (time in
seconds):

.. figure:: media/lmac.png
   :width: 100%

It is apparent from the results that the network can carry almost all
the traffic in this scenario (as opposed to the :ned:`XMac` and :ned:`LMac`
results.) The best performing value for :par:`slotDuration` is 40ms. Note
that the lowest :par:`slotDuration` values up until 120ms yield
approximately the same results (around 100 packets), with the 40ms
value performing marginally better. Choosing the higher :par:`slotDuration`
value would result in about the same performance but lower power
consumption, but we are optimizing for the number of packets here.

Measuring power consumption
~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. todo::

   <!-- TODO: rewrite this...the sensornode already has the power modules.
   these configs are not needed...just have to select the proper configs
   doesnt seem trivial...maybe its easier to just run those again

   We will run the three simulations with the chosen parameters again,
   this time measuring power consumption as well.

   There is a simulation for each MAC measuring the power consumption.
   These are `PowerBMac`, `PowerXMac`, and `PowerLMac`
   in :download:[omnetpp.ini](../omnetpp.ini).
   All of them extend the `PowerBase` configuration
   (which itself is an extension of `StatisticBase`):

   literalinclude:: ../omnetpp.ini
      :language: ini
      :start-at: PowerBase???
      :end-at: energyStorageType

   The power configs will use the same network as the statistic configs, :ned:`SensorNetworkShowcaseB`
   defined in :download:[SensorNetworkShowcase.ned](../SensorNetworkShowcase.ned). The simulations
   will be run for 100s. Vector recording is turned off, because we are not interested in how energy
   levels change over time, we only require the last value of the `residualEnergyCapacity` statistic.
   To get more relevant results, we run the power simulations 10 times. -->

We will examine the three simulations with the chosen parameters in
terms of power consumption.

.. V1

.. We want to record energy consumption in the radios of the wireless
   nodes. The :ned:`SensorNode` type has, by default, an energy storage model
   which will record the energy consumption, and an energy consumer model,
   which will provide how much energy is consumed. The energy storage
   module is :ned:`IdealEpEnergyStorage`, which is an energy/power-based
   storage model (the two available types of storage models are the simpler
   energy/power-based, and the more realistic charge/current-based, denoted
   by ``Ep`` and ``Cc`` in the type name.) :ned:`IdealEpEnergyStorage`
   provides infinite energy, cannot be fully charged or depleted, and it is
   useful if we only want to model and measure power consumption, but not
   energy capacity, charging, etc. We are interested in the
   ``residualEnergyCapacity`` statistic of the energy storage module. (Note
   that the ``residualEnergyCapacity`` statistic will have negative values,
   because energy generation is assumed to be positive, while energy
   consumption to be negative.)

.. The :ned:`SensorNode` type has an energy consumer module in its radio by
   default, it is :ned:`SensorStateBasedEpEnergyConsumer`. State based energy
   consumer models associate energy consumption values to the different
   radio modes and transmitter/receiver states. Energy consumption is
   dependent on these values and on how much time the radio spends in each
   mode/state. Note that the energy consumption values are parameters of
   the consumer module, they don't come from the radio.
   :ned:`SensorStateBasedEpEnergyConsumer` is an extension of
   :ned:`StateBasedEnergyConsumer` with default consumption values typical for
   low power wireless sensor nodes. (Note that the consumption values
   should be set to more accurate ones if needed.)


.. todo::

   <!-- TODO rewrite or delete
   v1
   We leave the consumption parameters on default, but for increased accuracy,
   they should be adjusted to the exact values in a simulation.
   v2
   Note that these default values are typical, but might not be accurate enough.
   If higher accuracy of the power consumption is required, the values should be
   set according to the simulated radio transceiver model and the power consumption
   associated with the actual transmission power, for example.
   v3
   These values are typical...but if you want more accuracy, you need to set them to more accurate ones
   v4
   Note that the consumption values can be set to more accurate ones if needed. -->

.. V2

We want to record the power consumption of the radios in the wireless
nodes. The :ned:`SensorNode` host type has an energy consumer submodule in
its radio by default, :ned:`SensorStateBasedEpEnergyConsumer`. This module
assigns a constant power consumption value to each radio mode and
transmitter/receiver state. Note that the values for each state are
parameters of the energy consumer module and don't come from other
modules (such as the radio), so they need to be set correctly
to obtain accurate power consumption results.
:ned:`SensorStateBasedEpEnergyConsumer` has default power consumption
values typical for wireless sensor nodes, thus we leave module's
parameters on default. :ned:`SensorNode` has an energy storage submodule by
default, :ned:`IdealEpEnergyStorage`. This module keeps track of the node's
energy consumption. It stores an infinite amount of energy, and cannot
get fully charged or depleted, and it is useful if we only want to model
and measure power consumption, but not energy capacity, charging, etc.


.. todo::

   <!-- The configurations extending the `PowerBase` configurations in
   :download:[omnetpp.ini](../omnetpp.ini) will use the
   best-performing parameter values selected in the previous section:

   literalinclude:: ../omnetpp.ini
      :language: ini
      :start-at: PowerBMac???
      :end-at: reservedMobileSlots

   -->

   <!-- We'll select the result files corresponding to the three best performing
   parameter values for the three MACs, and -->

The results for the parameter studies contain the required power
consumption data. We just need to select the result files corresponding to the
best performing parameter values selected in the previous sections. We
will plot the results on bar charts. We'll examine the following
statistics:

-  ``Total number of packets received``: All the packets received by the
   server. The UDP applications in the sensors each send 25 packets
   during the 25s simulations, for a total of 100
   packets. As there are 100 packets, this value is also the successful packet reception in percent,
   and indirectly, packet loss.
-  ``Network total power consumption``: The sum of the power consumption
   of the four sensors and the gateway (values in Joules.)
-  ``Power consumption per packet``: Network total power consumption /
   Total number of packets received, thus power consumption per packet
   in the entire network (values in Joules.)

.. -  ``Packet loss``: Total number of packets received / total number of
   packets sent, thus how many packets from the 100 sent are lost. **TODO** not sure its needed

Note that the values for the ``residualEnergyCapacity`` statistic are
negative, so it is inverted in the anf file. Here are the results:

.. figure:: media/packetsreceived.png
   :width: 100%

.. figure:: media/powerconsumption.png
   :width: 100%

.. figure:: media/powerconsumptionperpacket.png
   :width: 100%

.. .. figure:: media/packetloss.png
   :width: 100%

From this, it is apparent that LMac and XMac carried all packets, and
:ned:`BMac` had some packet loss. :ned:`BMac` consumed significantly more power than the
others. BMac also has significantly more power
consumption per packet. The conclusion is that in this scenario, with
the selected parameter values, :ned:`XMac` turned out to be the most energy
efficient MAC protocol. 

.. .. note:: XMac might perform better with another slot duration value for the sensors, for example. However, this is out of scope for this showcase.

| Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`SensorNetworkShowcase.ned <../SensorNetworkShowcase.ned>`, :download:`config.xml <../config.xml>`
| Extra configurations: :download:`extras.ini <../extras.ini>`, :download:`ExtrasSensorNetworkShowcase.ned <../ExtrasSensorNetworkShowcase.ned>` 

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
   Networks <https://ris.utwente.nl/ws/portalfiles/portal/5427399>`__

Discussion
----------

Use `this
page <https://github.com/inet-framework/inet-showcases/issues/24>`__ in
the GitHub issue tracker for commenting on this showcase.

:orphan:

Measuring time along Packet Flows
=================================

Goals
-----

This showcase demonstrates measuring time associated with a packet as it travels/when it's present in a network, such as total elapsed time, time spent in queues, or transmission time. 

The Model
---------

Overview of Packet Flows
~~~~~~~~~~~~~~~~~~~~~~~~

    so

    - the default statistic collection in inet is based on modules
    - the modules can only collect data they have access to
    - for more versatile timing measurements, packet flows can be defined
    - what measurements its good for
    - what are packet flows
    - how are they created

.. By default, statistics in INET are collected by modules, based on data the modules can access. For more complex timing measurements that require access to data of multiple modules, `packet flows` can be defined.

.. For example, a TCP module communicating with multiple other TCP modules can't distinguish between the packets based on the path they took.

By default, statistics in INET are collected by modules, only based on data the modules can access. Sometimes this might not be sufficient. For example, a TCP module communicating with multiple other TCP modules can't distinguish between the packets based on the path they took.

For more complex timing measurements that require access to data of multiple modules, `packet flows` can be defined. A packet flow is a logical classification of packets, identified by its name, in the whole network and over the whole duration of the whole simulation. Flows are defined active modules which create entry and exit points of the flow in the network. The following are some more properties of packet flows:

- A packet can be part of any number of flows
- A packet can enter and exit a flow multiple times
- Flows can overlap in time and in network topology
- Flows can have multiple entry and exit points

    so

    - packets enter the flow (can be filtering)
    - define the flow name and a filter and measurements
    - measurements are added on the way by modules in the network as metadata
    - packet exit the flow and the statistics are collected

    so

    - packets enter the flow by being tagged by active modules/the module

    active modules tag certain packets, so that they enter the flow (get a FlowTag or something)
    it tags certain packets, that is those matching a filter, for example
    similarly, the X module does the same for packets exiting the flow

    the modules have parameters, like the name of the flow

    what is the process like? actually

    - active modules can be inserted into the path of packets in the network, anywhere in the network
    - these modules classify packets to be entering the flow, e.g. packets matching a filter
    - it adds a tag
    - measurements are added on the way by modules in the network as metadata/tags
    - the packets exit the flow at module X, and statistics of the flow are collected there

The FlowMeasurementStarter module serves as the flow entry point for packets. It classifies certain packets (those matching a filter) to be entering the flow,
by adding a TODO tag to the packet. TODO Also, flow name, required measurements (packet/packetData filter)

The module has parameters for flow name, the required measurements, and packet filter/packet data filter. 

.. so

  - overview: the flow...certain packets enter the flow at active modules (by matching a filter)
  - the active modules add a TAG that describes the kind of measurements requested
  - based on the tag, the measurements are added to the packet as metadata by certain modules as it travels in the network
  - for example, we specify queueing time to be measured. The queues along the packet's route add the measured queueing time to the packet's metadata
  - packets exit the flow at modules acting as exit points. The measurements are recorded there as statistics.(as that module's statistics)(containing the total queueing time incurred by the packet

Certain packets are classified to be entering the flow by FlowMeasurementStarter modules, for example by matching a filter. The modules add a TAG that describes the kind of measurements requested by the user. Based on the tag, the measurements are added to the packet as metadata by certain modules as it travels in the network.

For example, we specify queueing time to be measured. The queues along the packet's route add the measured queueing time to the packet's metadata.
Packets exit the flow at FlowMeasurementRecorder modules. The measurements are recorded there as the FlowMeasurementRecorder module's statistics, containing the total queueing time incurred by the packet.

**TODO** the details of FlowMeasurementStarter and FlowMeasurementRecorder; it can be put anywhere in the packets' path; measurementLayer; limitations

The FlowMeasurementStarter and FlowMeasurementRecorder modules have the same set of parameters. The main parameters are the following:

- :par:`flowName`: Identifies the flow the packets are entering or exiting.
- :par:`measure`: Specifies the requested measurements. The values can be one or the combination of the following:
  
  - ``elapsedTime``: the total elapsed time for the packet being in the flow
  - ``delayingTime``
  - ``queueingTime``
  - ``processingTime``
  - ``transmissionTime``
  - ``propagationTime``
  - ``packetEvent``: Record all events that happen to the packet (more details below)
  
- :par:`packetFilter` and :par:`packetDataFilter`: to filter which packets enter or exit the flow

The ``packetEvent`` measurement is special, in the sense that it doesn't record certain durations as the other measurements. It adds to the packet as metadata all the events that happen to the packet, i.e. those measured by the other measurements. For example, it records events of the packet's delaying, queueing, processing, transmission, etc. **TODO** the user needs to create a module to extract and use this data

The FlowMeasurementStarter and FlowMeasurementRecorder modules can be placed in the packets' path anywhere in the network. However, certain modules, such as interfaces, have optional :ned:`MeasurementLayer` submodules. The :ned:`MeasurementLayer` module contains both measurment modules, and can be places easily in the network.

.. note:: FlowMeasurmentStarter and FlowMeasurmentRecorder modules can be placed in any submodule by creating an extended version of that module. See the TODO section for an example.

**TODO** limitations

**TODO** packetFlowVisualizer

The Example Simulations
-----------------------

The showcase contains the following examples simulations:

- TODO: Demonstrates creating packet flows by putting measurement modules into optional slots
- TODO: Demonstrates adding measurement modules to any module

.. FlowMeasurementStarter and FlowMeasurementRecorder

Both simulations use the following network:

.. figure:: media/Network.png
   :align: center

It contains :ned:`StandardHost`'s connected by :ned:`EthernetSwitch`'s in a dumbbell topology.

In both simulations, each client sends packets to the two servers. The simulations differ only in what packet flows and measurements are defined.

Config: Measurement Modules in Optional Slots
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In this simulation, we'll put a measurement thing in the UDP app and the switches.

In this simulation, we'll specify and measure five packet flows:

- 4 packet flows going from each client to the two servers (client1->server1, client1->server2, client2->server1, client2->server2)
- A packet flow going from switch1 to switch2

.. note:: The four packet flows originating in the hosts "meet" at switch1, so that the switch1 packet flow overlaps with all four of them

so

- we put the measurement thing in the UDP app
- also in the switch
- we measure what
- how to configure that
- illustrate the flows

.. figure:: media/flows2.png
   :align: center
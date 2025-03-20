TSNsched-based Gate Scheduling
==============================

Goals
-----

This showcase demonstrates the TSNsched gate schedule configurator that solves
the autoconfiguration problem with an external SAT solver tool.

| INET version: ``4.4``
| Source files location: `inet/showcases/tsn/gatescheduling/tsnsched <https://github.com/inet-framework/inet/tree/master/showcases/tsn/gatescheduling/tsnsched>`__

Background
----------

.. Time-Aware Shapers and Gate Scheduling
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. As described in the `Eager Gate Schedule Configuration <../eager/doc/index.html>`_ showcase, Time-Aware Shapers control when different traffic
.. classes can transmit (see the `Time-Aware Shaper
.. <../../../trafficshaping/timeawareshaper/doc/index.html>`_ showcase). TODO refine

In TSN networks, gate scheduling coordinates when each transmission gate in the
time-aware shapers opens and closes across multiple devices. INET provides several
gate schedule configurators that automatically generate these schedules based on
network topology and traffic requirements.

See the `Gate Schedule Configurators section <../../eager/doc/index.html#gate-schedule-configurators>`_ in the Eager showcase for more information about gate schedule configurators.

The Model
---------

The :ned:`TSNschedGateScheduleConfigurator` module provides a gate scheduling
configurator that uses the `TSNsched` tool that is available at
https://github.com/ACassimiro/TSNsched (check the NED documentation of the
configurator for installation instructions).

The TSNsched tool has several limitations compared to the built-in gate schedule
configurators:

1. **Transmission completion requirement**: All transmissions must complete within
   a single gate cycle. This restricts the traffic density that can be handled,
   requiring longer packet intervals.

2. **Inter-frame gap handling**: The tool requires explicit accounting for the
   12-byte inter-frame gap in packet size calculations, which is handled internally
   by other configurators.

:ned:`TSNschedGateScheduleConfigurator` prduces gate schedules as follows:

1. It exports the network configuration to an XML format that the external tool can process
2. It invokes the external TSNsched tool as a separate process
3. It imports the resulting schedule back into INET

Network Configuration
~~~~~~~~~~~~~~~~~~~~~

The simulation uses a dumbbell network topology with two clients (:ned:`TsnDevice`), two switches (:ned:`TsnSwitch`), and 
two servers (:ned:`TsnDevice`). All links are 100Mbps Ethernet connections:

.. figure:: media/Network.png
    :align: center

In this network:

- Two clients (client1 and client2) are connected to switch1
- Two servers (server1 and server2) are connected to switch2
- The switches are connected to each other
- All traffic flows from clients to servers through both switches

The time-aware shapers are located in the Ethernet interfaces of the switches. When 
egress traffic shaping is enabled with ``*.switch*.hasEgressTrafficShaping = true`` 
in the configuration, this adds an ``Ieee8021qTimeAwareShaper`` to the MAC layer of 
each Ethernet interface in the switches.

Traffic Configuration
~~~~~~~~~~~~~~~~~~~~

Each client generates two types of traffic:

1. **Best effort traffic** (PCP 0):

   - 1000-byte packets sent every 1000μs (~8Mbps)
   - Sent from client1 to server1 and from client2 to server1

2. **Video traffic** (PCP 4):

   - 500-byte packets sent every 500μs (~8Mbps)
   - Sent from client1 to server2 and from client2 to server2

The traffic is classified based on destination port and assigned appropriate PCP 
values using stream identification and stream encoding in the clients.

Gate Scheduling Configuration
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The TSNschedGateScheduleConfigurator is configured with:

- A gate cycle duration of 1ms
- Maximum latency requirement of 500μs for all flows
- Two traffic classes with corresponding gates (gate index 0 for best effort, 
  gate index 1 for video)

Here is the automatic gate scheduling configuration:

.. literalinclude:: ../omnetpp.ini
    :language: ini
    :start-at: # automatic gate scheduling
    :end-before: # gate scheduling visualization

.. note:: There are two important differences in this configuration compared to the Eager and SAT configurator showcases, due to limitations in the TSNsched tool: 
    
          - **Lower traffic density**: The traffic is half as dense as in the Eager and SAT configurator showcases (1000μs/500μs intervals instead of 500μs/250μs). This is because the TSNsched tool currently can only find solutions where all transmissions complete within a single gate cycle.
          
          - **Additional overhead bytes**: 12 bytes need to be added to the packet size to account for the inter-frame gap in the TSNsched tool's calculations.

Results
-------

The following sequence chart shows a complete gate cycle
period (1ms):

.. figure:: media/seqchart3.png
    :align: center

Frames are forwarded immediately by the switches, with
minimal queueing delay.

The following sequence chart shows frame transmissions, with the best effort gate state displayed on the axes for the two switches.

.. figure:: media/gates.png
    :align: center

Note that the gates are open so that two frames can be transmitted, and the frame transmissions and the send window are closely aligned in time.

The following figure shows application end-to-end delay for the four streams:

.. figure:: media/delay.png
    :align: center

All streams experience consistent delay values that approach the theoretical minimum
for this network configuration. Unlike the Eager configurator which produces varying
delays with distinct cluster points, the TSNsched tool creates a schedule where:

1. Delay is uniform across all packets in a stream
2. Packets are forwarded immediately upon arrival at switches
3. The schedule efficiently utilizes available bandwidth

However, it's important to note that this optimal scheduling is achieved with half
the traffic density of the other showcases, due to the limitation that all
transmissions must complete within a single gate cycle.

Sources: :download:`omnetpp.ini <../omnetpp.ini>`


Try It Yourself
---------------

If you already have INET and OMNeT++ installed, start the IDE by typing
``omnetpp``, import the INET project into the IDE, then navigate to the
``inet/showcases/tsn/gatescheduling/tsnsched`` folder in the `Project Explorer`. There, you can view
and edit the showcase files, run simulations, and analyze results.

Otherwise, there is an easy way to install INET and OMNeT++ using `opp_env
<https://omnetpp.org/opp_env>`__, and run the simulation interactively.
Ensure that ``opp_env`` is installed on your system, then execute:

.. code-block:: bash

    $ opp_env run inet-4.4 --init -w inet-workspace --install --chdir \
       -c 'cd inet-4.4.*/showcases/tsn/gatescheduling/tsnsched && inet'

This command creates an ``inet-workspace`` directory, installs the appropriate
versions of INET and OMNeT++ within it, and launches the ``inet`` command in the
showcase directory for interactive simulation.

Alternatively, for a more hands-on experience, you can first set up the
workspace and then open an interactive shell:

.. code-block:: bash

    $ opp_env install --init -w inet-workspace inet-4.4
    $ cd inet-workspace
    $ opp_env shell

Inside the shell, start the IDE by typing ``omnetpp``, import the INET project,
then start exploring.

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/793>`__ page in the GitHub issue tracker for commenting on this showcase.

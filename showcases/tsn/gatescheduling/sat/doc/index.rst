SAT-Solver-based Gate Schedule Configuration
============================================

Goals
-----

This showcase demonstrates a gate schedule configurator that solves the
autoconfiguration problem using a multivariate linear inequality system,
directly producing the gate control lists from the variables.

| INET version: ``4.4``
| Source files location: `inet/showcases/tsn/gatescheduling/sat <https://github.com/inet-framework/inet/tree/master/showcases/tsn/gatescheduling/sat>`__

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

SAT-Solver-Based Gate Scheduling
--------------------------------

The SAT (Boolean satisfiability) solver approach to gate scheduling represents the
scheduling problem as a system of constraints that must be satisfied. Unlike greedy
algorithms that make local decisions, a SAT solver considers the entire problem
space to find a globally optimal solution.

The Z3GateScheduleConfigurator uses the Z3 theorem prover, a state-of-the-art
SAT solver developed by Microsoft Research, to solve the gate scheduling
problem. As with other gate schedule configurators, the constraint requirements
(e.g. maximum delay or jitter) are specified using the :par:`configuration` parameter (see `flow specification <../../eager/doc/index.html#flow-spec>`_ in the Eager Gate Schedule Configuration showcase).

In addition to the parameters of GateScheduleConfiguratorBase, Z3GateScheduleConfigurator has a :par:`optimizeSchedule` parameter. It controls whether any solution
is accepted, or the solution should be optimal for the total end-to-end delay of all packet flows (true by default).

Z3GateScheduleConfigurator works by:

1. Formulating the gate scheduling problem as a set of linear inequalities
2. Representing timing constraints, resource conflicts, and latency requirements as
   mathematical expressions
3. Using the Z3 solver to find variable assignments that satisfy all constraints
4. Directly generating gate control lists from these variable assignments

This approach has several advantages over greedy algorithms:

- It can solve complex scheduling problems where greedy algorithms might fail
- It can find globally optimal solutions that minimize overall latency

The trade-off is increased computational complexity, as SAT solving is an
NP-complete problem. However, modern SAT solvers like Z3 use sophisticated
heuristics and optimizations to efficiently handle practical problem instances.

The Model
---------

The SAT-solver-based gate schedule configurator requires the `Z3 Gate Scheduling
Configurator` INET feature to be enabled, and the ``libz3-dev`` or ``z3-devel``
packages to be installed.

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

   - 1000-byte packets sent every 500μs (~16Mbps)
   - Sent from client1 to server1 and from client2 to server1

2. **Video traffic** (PCP 4):

   - 500-byte packets sent every 250μs (~16Mbps)
   - Sent from client1 to server2 and from client2 to server2

The traffic is classified based on destination port and assigned appropriate PCP 
values using stream identification and stream encoding in the clients.

Gate Scheduling Configuration
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The Z3GateScheduleConfigurator is configured with:

- A gate cycle duration of 1ms
- Maximum latency requirement of 500μs for all flows
- Two traffic classes with corresponding gates (gate index 0 for best effort, 
  gate index 1 for video)

Here is the automatic gate scheduling configuration:

.. literalinclude:: ../omnetpp.ini
    :language: ini
    :start-at: # automatic gate scheduling
    :end-before: # gate scheduling visualization

The configuration includes a detailed specification of each flow with its parameters:

- PCP values and gate indices
- Source and destination
- Packet length (including protocol overhead)
- Packet interval
- Maximum allowed latency

Note the comment explaining the protocol overhead calculation (58B = UDP + IP + 802.1Q-TAG + 
ETH MAC + ETH FCS + ETH PHY), which is added to the base packet length for accurate timing 
calculations.

Results
-------

A gate cycle of 1ms is displayed on the following sequence chart. Note that the
time efficiency of the gate schedules is even better than in the `Eager` case:

.. figure:: media/seqchart.png
    :align: center

The application end-to-end delay for the different traffic classes is displayed
on the following chart:

.. figure:: media/delay.png
    :align: center
    :width: 90%

.. The delay is constant for every packet, and within the specified constraint of
.. 500us. Note that the traffic delay is symmetric among the different source and
.. sink combinations (in contrast with the `Eager` case).

The delay is constant for every packet in a traffic category, and is within the
specified constraint of 500us. The constant delay in the same traffic class is a
direct result of the SAT solver's global optimization approach. Unlike the Eager
configurator, the Z3GateScheduleConfigurator considers all constraints
simultaneously.

The next sequence chart excerpt displays one packet as it travels from the
packet source to the packet sink, with the delay indicated:

.. figure:: media/seqchart_delay.png
    :align: center

The delay for all packets is optimal, which can be calculated analytically:
``(propagation time + transmission time) * 3`` (queueing time is zero).
Inserting the values of 84.64 us transmission time and 0.05 us propagation time
per link, the delay is 254.07 us for the best effort traffic category.

The following charts compare the SAT-based and Eager gate schedule configurators
in terms of application end-to-end delay:

.. figure:: media/delay_comp.png
    :align: center

The difference is that in case of the SAT-based gate schedule configurator, all
flows in a given traffic class have the same constant delay; in case of the
eager configurator's delay, some streams have more delay than others.

Sources: :download:`omnetpp.ini <../omnetpp.ini>`


Try It Yourself
---------------

If you already have INET and OMNeT++ installed, start the IDE by typing
``omnetpp``, import the INET project into the IDE, then navigate to the
``inet/showcases/tsn/gatescheduling/sat`` folder in the `Project Explorer`. There, you can view
and edit the showcase files, run simulations, and analyze results.

Otherwise, there is an easy way to install INET and OMNeT++ using `opp_env
<https://omnetpp.org/opp_env>`__, and run the simulation interactively.
Ensure that ``opp_env`` is installed on your system, then execute:

.. code-block:: bash

    $ opp_env run inet-4.4 --init -w inet-workspace --install --chdir \
       -c 'cd inet-4.4.*/showcases/tsn/gatescheduling/sat && inet'

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

Use `this <https://github.com/inet-framework/inet/discussions/792>`__ page in the GitHub issue tracker for commenting on this showcase.


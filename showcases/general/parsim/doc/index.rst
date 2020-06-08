:orphan:

Parallel Simulation
===================

Goals
-----

Simulation models can take advantage of parallel execution to speed up large simulations,
using OMNeT++'s built-in parallel simulation support.

This showcase demonstrates parallel simulation with an example simulation model.
Different subnetworks in the network are simulated with different CPU cores.

| INET version: ``4.3``
| OMNeT++ version: ``6.0``
| Source files location: `inet/showcases/general/parsim <https://github.com/inet-framework/inet-showcases/tree/master/general/parsim>`__

About Parallel Simulation in OMNeT++
------------------------------------

Most simulation models can be parallelized. To parallelize a simulation, the model is partitioned
into several parts; each partition contains some of the modules of the original simulation. Each
part will be run in a different process, preferably on a different CPU core (on the same machine
or another one), and each process will maintain its own simulation time.

There are limitations with respect to partitioning, e.g. modules in a partition cannot call C++ methods
in another partition. Thus global modules, such as network configurators, radio medium and visualizer
modules need to be included multiple times in the network, one for each partition that needs them.
For all limitations in partitioning, read the
`corresponding section <https://doc.omnetpp.org/omnetpp/manual/#sec:parallel-exec:overview>`_
in the OMNeT++ manual.

To preserve causality of events, the partition simulations are not allowed to advance too far
ahead of the others in simulation time. To this end, partition simulations have to be synchronized
by sending each other sync messages, which contain the current simulation time of the partition
simulation and how far other partition simulations can safely advance, called lookahead.

The partition simulations can send sync messages to each other using the Message Passing Interface (MPI).
MPI needs installation and it needs to be enabled in ``configure.user`` in OMNeT++. The processes can
be run on the same computer or on different computers.

.. note:: The partition simulations can also send sync messages using Named pipes. Named pipes requires no installation, but processes can only be run on the same machine, and its performance is worse than MPI's.

Currently, the lookahead is based on link delays. In INET, the most convenient partitioning is
on the network level; thus partitions are crossed by wired links between network nodes, the delay
of the wired links is the lookahead. For good performance, the number of messages between partitions
should be as low as possible. For example, parts of the network with localized high traffic should
be in their own partitions. Also, if the lookahead is too small, the partition simulations frequently
stop to wait for each other, and it results in larger overhead of sync messages being sent.

To enable parallel simulation, the ``parallel-simulation`` key needs to be set to ``true`` in the ini file.
Also, the number of partitions has to be specified with the ``parsim-num-partitions`` parameter.
The ``parsim-communications-class`` key selects which communication method to use; either ``cMPICommunications``
or ``cNamedPipeCommunications`` (For the other keys, check the
`corresponding section <https://doc.omnetpp.org/omnetpp/manual/#sec:parallel-exec:configuration>`_
in the OMNeT++ manual).

Modules can be assigned to partitions by specifying their partition id. For example:

.. code:: ini

   *.host{1..2}**.partition-id = 0
   *.host{3..4}**.partition-id = 1

.. **TODO** ** shouldn't be needed

These keys assign hosts 1 and 2 to partition 0 and hosts 3 and 4 to partition 1.

When using MPI, the ``mpiexec`` command can be used from the command line.
The command takes the number of partitions with the ``-n`` command line option,
followed by the executable to run (in this case ``inet``).

When using named pipes, the ``-p`` command line option selects which partition to run.

The Model
---------

The Network
~~~~~~~~~~~

The example simulation features four subnetworks connected to a backbone of routers:

.. figure:: media/Network5.png
   :align: center
   :width: 80%

The subnetwork compound module contains four wired hosts connected by a switch, and
four wireless hosts connected by an access point:

.. figure:: media/subnetwork2.png
   :align: center
   :width: 80%

Each subnetwork contains an :ned:`Ipv4NetworkConfigurator` module and an :ned:`Ieee80211ScalarRadioMedium` module.
Since each subnetwork is in a different partition, each of them needs these modules due to the limitation of
partitioning, i.e. no method calls to modules in other partitions.

Wired connections in the subnetworks are ``Ethernet100``, the ones between the routers are ``PppChannel``,
defined in the NED file:

.. literalinclude:: ../Network.ned
   :start-at: Ethernet100
   :end-before: Subnetwork
   :language: ned

The connections extend :ned:`DatarateChannel`, and add a delay of 0.1 and 1 us (corresponding to about
20m and 2000m long cables). Since only the connections between the routers cross partitions, the delay
of these connections is the lookahead.

Partitioning
~~~~~~~~~~~~

The network is divided into four partitions. Each subnetwork, and the router closest to it are assigned
to a different partition:

.. literalinclude:: ../omnetpp.ini
   :start-at: *A**.partition-id
   :end-at: *D**.partition-id
   :language: ini

In each subnetwork (and thus, partition) there is a radio medium module.
Wireless nodes can only use the radio medium module in their partition, so they can't send signals to
nodes in another partition. This limitation needs to be considered when simulating interference; the
interfering nodes need to be in the same partition. In this case, we don't want to simulate the
interference between subnetworks, so they can be in different partitions.

By default, radios look for the radio medium module named ``radioMedium`` in the top level, so we
configure them to use the radio medium module in the subnetwork. Similarly, we need to specify
for each host and router to use the configurator module in its partition, the one in the subnetwork.
We specify these in the subnetwork's NED definition:

.. literalinclude:: ../Network.ned
   :start-at: Subnetwork
   :end-at: networkConfiguratorModule
   :language: ned

All access points in the network have the same SSID by default. However, the configurator would
put all wireless hosts and access points in the same wireless network, so the SSID's of the
subnetworks need to be unique:

.. literalinclude:: ../omnetpp.ini
   :start-at: subnetworkA.**.ssid
   :end-at: subnetworkD.**.defaultSsid
   :language: ini

When the same simulation is run both sequentially and in parallel, the results might not be the same.
By default, the different partition simulations use the same set of random number generators as the
sequential simulation; the random numbers drawn by the same modules in the parallel and sequential
case are different, since there are less modules in a partition that draw them compared to the sequential simulation.

Parallel simulation is susceptible to race conditions.
We want to make sure that race conditions don't cause incorrect results, so we configure the parallel
and the sequential simulations to follow the same trajectory.

To do that, we configure each partition to have an own random number generator, which the modules in
that partition will use, regardless of running the simulation sequentially or parallelly:

.. literalinclude:: ../omnetpp.ini
   :start-at: num-rngs = 4
   :end-at: *D**
   :language: ini

Each RNG is configured to use the partition ID as seed. The seeds need to be configured for the
parallel and sequential cases separately:

.. literalinclude:: ../omnetpp.ini
   :start-at: rng seed parameters for parsim
   :end-at: seed-3-mt = 3
   :language: ini

Note that the ``num-rngs`` parameter pertains to each simulation, whether its the sequential or
one of the partition simulations. Thus, each partition simulation has four RNGs, although it needs
and uses only one of them/the one with the partition ID as index.

Configuring Addresses and Routes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

We can't use a global configurator module due to the limitation of partitioning, thus, we need a
configurator module in each partition, otherwise addresses and routes wouldn't be configured.
However, the configurator needs knowledge of the complete network topology. to correctly configure addresses and routes.

To overcome this limitation, we run the simulation sequentially, dump the address and route configuration created
by the configurator to an xml file, and use that file as the configuration for each configurator module.

All four configurator's would create the same xml file, so we just dump one of them.
The ``GenerateNetworkConfiguration`` config in omnetpp.ini can be used for this purpose:

.. literalinclude:: ../omnetpp.ini
   :start-at: GenerateNetworkConfiguration
   :end-at: Routes
   :language: ini

The ``network-configuration.xml`` file is used by all network configurators in the parallel simulation:

.. literalinclude:: ../omnetpp.ini
   :start-at: configurator.config
   :end-at: configurator.config
   :language: ini

To generate ``network-configuration.xml``, run the ``generate-network-configuration``
script in the showcase's folder. The script runs the simulation sequentially:

.. literalinclude:: ../generate-network-configuration
   :language: bash

Now the configurators set the IP addresses and routes for the network nodes in their own partitions.

.. note:: The generated xml configuration file is included in the showcase's folder.

Traffic
~~~~~~~

In the subnetworks, we set up high local traffic, and sporadic cross-partition traffic.

In each partition, ``wiredHost1`` pings ``wirelessHost1`` in the next subnetwork
(A -> B -> C -> D -> A); similarly, ``wirelessHost1`` pings the next subnetwork's
``wiredHost1``. The frequency of the ping messages is the default 1s.

Each wired host is configured to send a TCP stream to the next one in the same subnetwork
(1 -> 2 -> 3 -> 4 -> 1); wireless hosts are configured to do the same. This generates
substantial local traffic, which is needed for good parallel performance gains.

Running the Simulations
-----------------------

Two shell scripts in the showcase's folder can be used to run the parallel simulation.
The ``runparsim-mpi`` script uses MPI:

.. literalinclude:: ../runparsim-mpi
   :language: bash

The ``runparsim`` script uses the named pipes communication method:

.. literalinclude:: ../runparsim
   :language: bash

To run the simulations, execute one of the scripts from the command line.
By default, the simulations are run with Qtenv:

.. code-block:: bash

   $ ./runparsim-mpi

.. code-block:: bash

   $ ./runparsim

In this case, four Qtenv windows open. Click the run simulation button in
all of them to start the parallel simulation. The simulation can only
progress if all partition simulations are running; the partition simulations
stop and wait after a lookahead duration until all of them are started:

To start the simulations in Cmdenv, append ``-u Cmdenv`` to the command:

.. code-block:: bash

   $ ./runparsim-mpi -u Cmdenv

.. code-block:: bash

   $ ./runparsim -u Cmdenv

Results
-------

Here is a video of running the simulations in Cmdenv:

.. video:: media/cmdenv.mp4
   :width: 100%

The outputs of the four simulations are mixed, but the messages for successfully
received ping replies are observable.

Here is a video of the simulations running in Qtenv:

.. video:: media/qtenv4.mp4
   :width: 100%

We compared the parallel and sequential simulations by plotting the ping round-trip
time of ``wiredHost1`` in ``subnetworkA``:

.. figure:: media/rtt.png
   :align: center

The values match exactly, for the other hosts as well.

We measured the run time of the sequential and the parallel simulation. Using MPI, the simulation ran about 2.5 times faster than the sequential simulation on a Ryzen 7 4800HS 8-core CPU (4 cores were used) and 32 GB RAM.

.. note:: When the simulations are run in Qtenv, all network nodes are present in all Qtenv windows. However, Qtenv uses placeholder modules for those which are not in the partition the Qtenv instance is running. These placeholder modules are empty. However, one can still observe the messages going between partitions in the Qtenv packet log.

Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`Network.ned <../Network.ned>`

Discussion
----------

Use `this <https://github.com/inet-framework/inet-showcases/issues/TODO>`__ page in the GitHub issue tracker for commenting on this showcase.

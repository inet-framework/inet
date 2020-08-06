:orphan:

Parallel Simulation
===================

Goals
-----

.. so

  - parallel simulation is running parts of a simulation model in one process/cpu and others on others
  - so its about instead of running the simulation in one process, it can be run in multiple processes in a parallel way
  - a process simulates part of the model, i.e. some of the modules
  - it is useful because by parallelizing a simulation it can run faster
  - so the model is divided into logical partitions
  - each module of the network belongs to one of the partitions
  - each logical partition maintains its own simulation time
  - the key is to somehow preserve causality
  - that is, a message might arrive at a partition from another one, but the other might have advanced its simulation time by then
  - to preserve casuality, simulation time cannot advance too far
  - for this, the processes need synchronization
  - currently, this is based on link delay
  - so we know that a message can arrive on a link, and a link has a certain delay
  - so we know that we can get ahead of the source partition in simulation time by the delay
  - the partitions send sync messages
  - this is the lookahead

  so lookahead depends on link delay
  if the lookahead is too small, the partitions stop all the time

  the goals of parallel simulations

Simulation models can take advantage of parallel execution to speed up large simulations, using OMNeT++'s built-in parallel simulation support.

This showcase demonstrates parallel simulation with an example simulation model. Different LANs in the network are simulated with different CPU cores.

| INET version: ``4.3``
| OMNeT++ version: ``6.0``
| Source files location: `inet/showcases/general/parsim <https://github.com/inet-framework/inet-showcases/tree/master/general/parsim>`__

.. **TODO** inet 4.2? or is that 4.3? needs omnetpp-6.0?

.. **TODO** omnet 6.0

About Parallel Simulation in OMNeT++
------------------------------------

Most simulation models can be parallelized. To parallelize a simulation, the model is partitioned into several parts; each partition contains some of the modules of the original simulation. Each part will be run in a different process, preferably on a different CPU core (on the same machine or another one), and each process will maintain its own simulation time.

.. , no additional C++ code is needed TODO not quite.

.. The logical partitions can be run on different CPUs on the same machine or on another machine.

.. Any simulation model can be parallelized. To parallelize a simulation, the model is divided into several logical partitions; each partition contains some of the modules of the simulation, will be run in its own process.
   The logical partitions can be run on different CPUs on the same machine or on another machine.

.. **TODO** each partition/process has its own simulation time

.. To partition a simulation model, all modules in the network are assigned to one of the partitions.

There are limitations with respect to partitioning, e.g. modules in a partition cannot call C++ methods in another partition.
Thus global modules, such as network configurators, radio medium and visualizer modules need to be included multiple times in the network, one for each partition that needs them. For all limitations in partitioning, read the `Parallel Simulation section <https://doc.omnetpp.org/omnetpp/manual/#cha:parallel-exec>`_ in the OMNeT++ manual.

.. The main limitation in partitioning is that only messages can be passed between partitions, e.g. method calls.

.. **TODO**

.. **TODO** for all limitations read the documentation?

.. Thus global modules, such as network configurators, radio medium and visualizer modules multiple instances of need to be included

.. Thus multiple instances of global modules (network configurator, radio medium, visualizer, etc.) are needed, one for each partition TODO not all of them...

.. To preserve casuality, none of the partitions can get too far ahead of the others in simulation time.

.. because an event in one of partitions might affect the trajectory of the simulation in another.

.. An event in one of the partitions might affect the trajectory of the simulation in another. A partition might receive a message from another partition in its past, i.e. the message might arrive at a partition but the simulation time in that partition might have already advanced beyond the message's arrival time. To preserve causality of events, this behavior is prevented; partitions are not allowed to advance too far ahead of the others in simulation time. To this end, partitions have to be synchornized by sending each other sync messages, which contain the current simulation time of the partition and how far other partitions can safely advance in simulation time, called lookahead.

To preserve causality of events, the partition simulations are not allowed to advance too far ahead of the others in simulation time. To this end, partition simulations have to be synchronized by sending each other sync messages, which contain the current simulation time of the partition simulation and how far other partition simulations can safely advance, called lookahead.

.. **TODO** rövidebben ^ -> particiok kulon tekernek de van time sync kozottuk es message-ekkel kommunikalnak

.. .. note:: Lookahead could be based on other metrics, TODO it can be implemented refer to the manual

The logical processes can send sync messages to each other using either the Message Passing Interface (MPI), or named pipes.
MPI needs installation and it needs to be enabled in ``configure.user`` in OMNeT++, but the processes can be run on different computers. Named pipes requires no installation, but processes can only be run on the same machine.

.. TODO mpi-t engedelyezni kell a configure-ban

.. The logical processes can send sync messages to each other in the following ways:

   - using the Message Passing Interface (MPI)

.. - message types (pipe, mpi)
   - limitations (can only pass messages)

.. Only messages can pass between partitions, not method calls or direct sends.

.. Running a simulation model in parallel has a few constraints. Only messages can be sent between partitions, method calls and direct sends are not supported. TODO so you need multiple configurators for example. Lookahead is required, present in the form of link delays between partitions.

Currently, the lookahead is based on the delay of the wired links between partitions. TODO

TODO inet ben parallel sim esetén a legkézenfekvőbb mód a network szintu particionálás, és a wired linkek lesznek a lookahead-ek

For good performance, the number of messages between partitions should be as low as possible.
For example, parts of the network with localized high traffic should be in their own partitions.
Also, if the lookahead is too small, the partition simulations frequently stop to wait for each other, and it results in larger overhead of sync messages being sent.

TODO ez az egész 1 paragraph

.. **TODO** if the lookahead is too small, the partitions stop all the time

.. For example, if there parts of the network with localized high traffic, that part should belong a partition;

  - useful stuff (minimize messages between partitions for good performance)
  - how to partition (partition id)
  - how to run

To enable parallel simulation, the ``parallel-simulation`` key needs to be set to ``true`` in the ini file.
Also, the number of partitions has to be specified with the ``parsim-num-partitions`` parameter.
The ``parsim-communications-class`` key selects which communication method to use; either ``cNamedPipeCommunications`` or ``cMPICommunications`` (For the other keys, check the `Parallel Simulation section <https://doc.omnetpp.org/omnetpp/manual/#cha:parallel-exec>`_ in the OMNeT++ manual).

.. `Parallel Simulation section <https://doc.omnetpp.org/omnetpp/manual/#cha:parallel-exec>`_

.. **TODO** parsim-debug? or just link to the manual ? -> vannak mas parsim kapcsolok a manualban leirva

.. To run simulations in parallel, the modules need to be assigned to partitions. For example:

Modules can be assigned to partitions by specifying their partition id. For example:

.. code:: ini

   *.host{1..2}**.partition-id = 0
   *.host{3..4}**.partition-id = 1

**TODO** ** shouldn't be needed

These keys assign hosts 1 and 2 to partition 0 and hosts 3 and 4 to partition 1.

.. Note that the ``**`` after the host name is required in order to put all modules recursively into the same partition as the host.

.. To run the simulations, the ``p`` command line argument needs to be specified, selecting which partition to run in a process. TODO or the from the IDE. TODO also the scripts. TODO rewrite

.. The simulation for a specific partition can be run with the ``p`` command line parameter. TODO or from the ide
   (note that the example simulation contains a script which runs all partitions).

  so

  something like...

  - the p command line option selects which partition/process to run
  - can be set in the IDE as well
  - but it is easier to execute all partitions from the command line with a script
  - can use Cmdenv or Qtenv
  - in case of Qtenv, multiple instances are started (not sure its needed...the point is one can observe the parallel simulation from multiple instances of Qtenv)(where they can be started independently and observe that they dont advance if one of them is stopped)

When using named pipes, the ``-p`` command line option selects which partition to run.

.. It is often easier to put this in a script which runs all partitions **TODO**

.. It takes multiple commands **TODO**

When using MPI, the ``mpiexec`` command can be used from the command line. The command takes the number of partitions with the ``-n`` command line option, followed by the executable to run (in this case ``inet``).

.. For more information on parallel simulation, see the `Parallel Simulation section <https://doc.omnetpp.org/omnetpp/manual/#cha:parallel-exec>`_ in the OMNeT++ manual.

.. **TODO** mpiexec

**TODO** a linkek specifikusan mutassanak vagy csak az első legyen benne

.. ---------------------

  - we have a large simulation model, and we want to parallelize it to speed it up
  - we can do that because omnet has the functionality built it
  - almost all simulation models can be parallelized, no extra stuff required
  - to do that, we divide the simulation into logical partitions / processes
  - each of which will run in a cpu core or on another computer
  - to partition the model, all modules in the network are assigned to a partition
  - limitations
  - the partitions cant run independently
  - they need to preserve causality
  - this is achieved by syncronization and messages?
  - currently the lookahead is based on link delay
  - there is pipe and mpi
  - how to run it / scripts

   ------------------------

The Model
---------

The Network
~~~~~~~~~~~

The example simulation features a mixed wired/wireless network with several LANs and a backbone of routers:

.. figure:: media/Network3.png
   :align: center
   :width: 100%

The network contains two wired and two wireless LANs, each with two hosts connected by a switch or an access point.
The LANs are connected to a backbone of routers.

.. **TODO** X kivesz a backbonebol

.. The nodes are connected by ``Ethernet100`` connections, defined in the NED file:
   -> the wired connections are ?
   -> routers should be connected by ppp and add delay

.. .. literalinclude:: ../Network.ned
   :start-at: Ethernet100
   :end-at: }
   :language: ned

All wired connections are ``Ethernet100``, except the ones between the routers, which are ``PppChannel``, defined in the NED file:

**TODO** not 'all wired connections'

.. literalinclude:: ../Network.ned
   :start-at: Ethernet100
   :end-before: ParSimNetworkBackbone
   :language: ned

The connections extend :ned:`DatarateChannel`, and add a delay of 100us.
Since only the connections between the routers cross partitions, the delay of these connections is the lookahead.

**TODO** 100us is too much (100m -> 0.5us)

-> ethernet should be 0.1us -> ppp should be 1us
(20m and 200m)

-> hogy jönnek ki ezek az értékek -> a távolságból -> erről több

.. TODO: the emphasis is on the router connection delay -> ez lesz a lookahead
   mert a routerek vannak a hataron

Partitioning
~~~~~~~~~~~~

The network is divided into four partitions. Each LAN, together with its corresponding switch/access point, and the router closest to it are assigned to a different partition.
Each partition contains an :ned:`Ipv4NetworkConfigurator` module; wireless LANs also contain a :ned:`Ieee80211ScalarRadioMedium` module. The extra modules are needed due to the limitation of partitioning, i.e. no method calls to modules in other partitions.

.. The network is divided into four partitions. Each LAN, together with its corresponding switch/access point, and the router closest to it are assigned to a different partition.
   Each partition contains an :ned:`Ipv4NetworkConfigurator` module; wireless LANs also contain a :ned:`Ieee80211ScalarRadioMedium` module.

.. TODO minden partition külön executable; ha nincs benne configurator nem lesz kitoltve; az egyik particioban levo configurator nem tudja a tobbiben levot konfiguralni -> thats why
   az egyik particioban levo medium nem tud a masikkal kommunkalni mert method call
   de nem akarjuk az interferenciat szimulalni (nem tudjuk de ebben nem is akarjuk)
   (azert lehet kulon particioban, mert nem akarjuk az interferenciat szimulalni)
   a wireless nodeoik nem tudnak kommunikalni a masik particiban levo mediummal mert method call
   amelyik particioban nincs konfigurator ott nem lesznek konfiguralva

Two of the hosts are configured to ping another two hosts (``host1`` pings ``host5``, ``host3`` pings ``host8``).

.. TODO config

  - priority DONE
  - assign to configurator DONE
  - actually, how the routes are configured DONE
  - assign to radio medium DONE
  - fcsMode DONE
  - parsim-communications-class = "cNamedPipeCommunications" DONE

The modules are assigned to partitions, as follows:

.. literalinclude:: ../omnetpp.ini
   :start-at: radioMedium1**.partition-id
   :end-at: configurator4**.partition-id
   :language: ini

.. Also, the priorities of all modules are specified. Setting priorities defines the order of events if multiple events in different partitions happen at the same simulation time. In this case, we assign the same priority to all modules as their partition number, like so:

.. **TODO** kulonben nem lesz determinisztikus; mi az a priority es miert -> ezzel kene kezdeni; a 2. mondat

..  **V1** This is needed because if two events in different partitions happen at the same simulation time, their order is defined. **TODO**

.. .. literalinclude:: ../omnetpp.ini
   :start-at: priority = 0
   :end-at: priority = 3
   :language: ini

.. Because **TODO**:

   **TODO** try without this; might not be needed

   .. literalinclude:: ../omnetpp.ini
   :start-at: crc
   :end-at: fcs
   :language: ini

.. so

  - synchronization: the parsim algorithm...how to run simulations in parallel (the null message algorithm)
  lookahead, and what messages to send
  - communication: the channels to send the messages on. either mpi or pipe (or the text file based)

.. **TODO** radioMedium

The two wireless LANs have radio medium modules in their partitions. Wireless nodes can only use the radio medium module in their partition, so they can't send signals to nodes in another partition. This limitation needs to be considered when simulating interference; the interfering nodes need to be in the same partition. In this case, we don't want to simulate the interference between the two wireless LANs, so they can be in different partitions.

The radio medium modules don't have the default name ``radioMedium``, so the module where hosts and access points belong to, needs to be specified:

.. literalinclude:: ../omnetpp.ini
   :start-at: radioMediumModule
   :end-at: AP2.**.radioMediumModule
   :language: ini

**TODO** ssid -> ne higgye azt hogy egy networkbe tartoznak

.. **TODO** rewrite? miert kell kulon medium

.. literalinclude:: ../omnetpp.ini
   :start-at: radioMedium1
   :end-at: AP2
   :language: ini

Similarly, we need to specify for each host and router to use the configurator module in its partition:

.. literalinclude:: ../omnetpp.ini
   :start-at: networkConfiguratorModule
   :end-at: router4.**.configurator.networkConfiguratorModule
   :language: ini

.. **TODO** rng config + sequential-parallel equivalency

   so

  -we want to get the same results from sequential and parsim
  -so that we can verify that parsim does the same thing

When the same simulation is run both sequentially and in parallel, the results might not be the same. By default, the different partition simulations have their own random number generators/use the same set of rngs. **TODO** different number of rngs then with seq...use an instance of the same random number generators as the sequential simulation, but the random numbers drawn by the same modules in the parallel and sequential case are different, since there are less modules in a partition that draw them compared to the sequential simulation. The results are just as correct, but we want to verify that; we configure the parallel and the sequential simulations to follow the same trajectory.

-> to prove that the results are correct...

Parallel simulation is susceptible to race conditions.
We want to make sure that race conditions don't cause incorrect results.

  TODO a race condition ne hozzon ki incorrect eredményt
  a seqeuentialban nincs race condition
  szoval ugy tudjuk bizonyitani hogy ugyanazt hozzuk ki
  mint a sequential

  -> elég hosszu eleg bonyolult sim-ben ugyan az az eredmenyt ki tudjuk hozni, akkor biztosak lehetünk hogy nincs baj a race conditionbol

To do that, we configure each partition to have an own random number generator, which the modules in that partition will use, regardless of running the simulation sequentially or parallelly. Also, the seeds need to be set **TODO** why

-modules use the rng the same as the partition id
-the partitions use one of the 4

lehetne sequential, and parallel and common ini file
(van kulon General configjuk)

.. literalinclude:: ../omnetpp.ini
   :start-at: num-rngs
   :end-at: num-rngs
   :language: ini

.. literalinclude:: ../omnetpp.ini
   :start-at: rng seed
   :end-at: seed-3-mt =
   :language: ini

Configuring Addresses and Routes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

..  so

  - we cant use a global configurator module to configure addresses and routes due to the limitation proposed/set by partitioning
  - thats why we have four configurators -> each needs to deal with only the modules of its own partition
  - the process is the following
  - run the simulation by disabling parsim
  - dump the configurator config to xml files
  - actually, the four configurators should produce the same config
  - copy them and delete all entries/configuration elements/address and route assignments not pertaining to the modules in the same partition/the modules its responsible for
  - disable some stuff in the configurator
  - use the modified xml files as the configuration
  - re-enable parsim...the configurators configure the modules in their own partitions

.. We can't use a global configurator module to configure addresses and routes due to the limitation set (?) by partitioning,
   thus we have four configurators; each can deal with only the modules in its own partition. **TODO** ismernie kell a teljes topologiat; global knowledge kell; not possible
   The process for configuring addresses and routes per partition is the following:

We can't use a global configurator module due to the limitation of partitioning, thus, we need a configurator module in each partition, otherwise addresses and routes wouldn't be configured. However, the configurator needs knowledge of the complete network topology. to correctly configure addresses and routes.

To overcome this limitation, we run the simulation sequentially, dump the address and route configuration created
by the configurator to an xml file, and use that file as the configuration for each configurator module.

**TODO** redundant

To do this, we need to run the simulation sequentially and with the configurator set to add static routes.
All four configurator's would create the same xml file, so we just dump one of them.
The ``GenerateConfiguratorConfig`` configuration in omnetpp.ini can be used for this purpose:

**TODO** RoutesAndAddresses.xml/AddressesAndRoutes.xml

.. literalinclude:: ../omnetpp.ini
   :start-at: GenerateConfiguratorConfig
   :end-at: Routes
   :language: ini

The ``config.xml`` file is used by all network configurators in the parallel simulation:

.. TODO

.. literalinclude:: ../omnetpp.ini
   :start-at: configurator*
   :end-at: configurator*
   :language: ini

To generate ``config.xml``, run the ``generateConfiguratorConfig`` script in the showcase's folder. The script runs the simulation sequentially:

.. literalinclude:: ../generateConfiguratorConfig
   :language: bash

.. .. code-block:: bash

   $ inet -c GenerateConfiguratorConfig --parallel-simulation=false

.. so

  - we can't use a global configurator module to configure addresses and routes due to the limitation of parallel simulation (no method calls to modules in another partition)
  - thus we need a configurator module in each partition, if we want to have the addresses and routes in that partition configured
  - but the configurator needs information about the whole network topology to configure addresses and routes
  - so we run the simulation without parsim
  - dump the configuration
  - and use that file as the config file of each configurator module
  - it contains entries for modules in other partitions but the configurator ignores them

.. We can't have a global configurator module due to the limitation of parallel simulation (no method calls to modules in other partitions). Thus to configure addresses and routes, each partition needs to contain a configurator module. However, the configurator needs to know the complete network topology to configure routes.

.. **TODO** ezt a usernek kell

.. - run the simulation by disabling parsim

.. - Make sure the configurators are set to add static routes and run the simulation with parsim disabled. The configurators automatically configure addresses and routes in the network:

  .. code-block:: ini

     #parallel-simulation = true

  .. code-block:: ini

     #*.configurator*.*Routes = false

  - Dump the configurators' config to xml files (the four configurators produce the same configuration, but we dump all of them now):

  **TODO** dump just one ?

  **TODO** kulon konfig es kikommentez a parsim

  **TODO** try not deleting the lines from the xml; they can use the same file

  **TODO** kulon konfig; run from ccmdenv, override parsim

  **TODO** a configurator input file eloalittasanak a folyamata a process

    .. code-block:: ini

       *.configurator1.dumpConfig = "configurator1backbonedump.xml"
       *.configurator2.dumpConfig = "configurator2backbonedump.xml"
       *.configurator3.dumpConfig = "configurator3backbonedump.xml"
       *.configurator4.dumpConfig = "configurator4backbonedump.xml"

  - Duplicate each dump to a new xml config file and delete all address and route assignments not pertaining to the modules in the partition the configurator belongs to.

  - Use the modified xml files as the configuration:

    .. code-block:: ini

       *.configurator1.config = xmldoc("configurator1backbone.xml")
       *.configurator2.config = xmldoc("configurator2backbone.xml")
       *.configurator3.config = xmldoc("configurator3backbone.xml")
       *.configurator4.config = xmldoc("configurator4backbone.xml")

  .. - Disable adding static routes in the configurator

  - Re-enable parsim and make sure adding static routes is disabled.

Now the configurators set the IP addresses and routes for the network nodes in their own partitions.

.. note:: The generated xml configuration file is included in the showcase's folder.

.. TODO example config

   .. literalinclude:: ../configurator1backbone.xml
      :language: xml

Running the Simulations
-----------------------

.. TODO scripts

  so

  - there are two scripts in the showcase folder for running all the simulations
  - one for pipe and one for mpi
  - append -u Cmdenv to run it in Cmdenv (it runs in qtenv by default)(in which case 4 qtenv windows open)
  - which can be used to observe the simulation

  results

  - there are placeholder modules for those modules in the network which don't belong to the particular partition (they are empty)
  - but the messages going between partitions can be observed

Two shell scripts in the showcase's folder can be used to run the parallel simulation. The ``runparsim`` script uses the named pipes communication method:

.. literalinclude:: ../runparsim
   :language: bash

The ``runparsim-mpi`` script uses MPI:

.. literalinclude:: ../runparsim-mpi
   :language: bash

.. **TODO** does mpi need the parsim-num-partitions ?

.. **TODO** the scripts only work on linux

To run the simulations, execute one of the scripts from the command line. By default, the simulations are run with Qtenv:

.. code-block:: bash

   $ ./runparsim

.. code-block:: bash

   $ ./runparsim-mpi

In this case, four Qtenv windows open. Click the run simulation button in all of them to start the parallel simulation. The simulation can only progress if all partition simulations are running; the partition simulations stop and wait after a lookahead duration until all of them are started:

.. **TODO** can only progess if all partition simulations are running; a parittion szimulaciok megallnak / varakozni kezdenek a lookahead mulva

To start the simulations in Cmdenv, append ``-u Cmdenv`` to the command:

.. code-block:: bash

   $ ./runparsim -u Cmdenv

.. code-block:: bash

   $ ./runparsim-mpi -u Cmdenv

Results
-------

Here is a video of running the simulations in Cmdenv:

TODO -> need

.. **TODO** osszehasonlitas a sequential es a nem sequential kozott
   -> lehet hogy a parsim = true a scriptben kene
   -> az eredmenyeknek exact matchnek kene lennie

.. -> a 4 parallel ping log es a sequential -> diff a 2 -> a sequential csak extra sorokat tartalmaz mert interleave-elodnek
   -> statisztikak -> rtt felrajzol -> exact match -> kivon a 2 es 0nak kell lennie

We compare the parallel and sequential simulations by plotting the difference between the round-trip-time of the same ping messages in both (only for ``host3``, as ``host1``'s ping messages only use wired links, and don't draw any random numbers).

.. figure:: media/rtt.svg
   :align: center

**TODO** ez igy meaningless -> inkabb egymast fedjek

The values match exactly.

**TODO** -> lemer hogy mennyi ideig fut -> on such and such computer this is the result
-> legalabb legyen 2x gyorsabb

The outputs of the four simulations are mixed, but the messages for successfully received ping replies in ``host1`` and ``host3`` are observable.

.. When the simulations are run in Qtenv, there are placeholder modules for those modules in the network which don't belong to the particular partition (they are empty)
   - but the messages going between partitions can be observed

.. note:: When the simulations are run in Qtenv, all network nodes are present in all Qtenv windows. However, Qtenv uses placeholder modules for those which are not in the partition the Qtenv instance is running. These placeholder modules are empty. However, one can still observe the messages going between partitions in the Qtenv packet log.

.. **TODO** in the packet log / in the qtenv packet log

Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`Network.ned <../Network.ned>`

Discussion
----------

Use `this <https://github.com/inet-framework/inet-showcases/issues/TODO>`__ page in the GitHub issue tracker for commenting on this showcase.

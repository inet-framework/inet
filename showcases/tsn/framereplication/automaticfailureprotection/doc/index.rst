Automatic Stream Configuration with Failure Protection
======================================================

Goals
-----

In this showcase, we demonstrate INET's automatic FRER configuration
capabilities. Using automatic configurators that compute redundant paths based
on specified failure protection requirements, we compare network behavior with
and without frame replication during a controlled node failure, showing how FRER
maintains 100% packet delivery even during equipment outages.

| Verified with INET version: ``4.4``
| Source files location: `inet/showcases/tsn/framereplication/automaticfailureprotection <https://github.com/inet-framework/inet/tree/master/showcases/tsn/framereplication/automaticfailureprotection>`__

Frame Replication and Elimination
----------------------------------

In critical industrial and automotive networks, equipment failures are
inevitable - switches can crash, links can be severed, and components can
malfunction. Traditional network architectures are vulnerable to such failures:
when a component fails, traffic is lost until the failure is detected and
alternative routes are established, resulting in unacceptable service
interruption.

Frame Replication and Elimination for Reliability (FRER), defined in IEEE
802.1CB, solves this problem by providing seamless redundancy at the frame
level. By transmitting duplicate copies of critical frames along multiple
physically diverse paths and eliminating duplicates at the destination, FRER
ensures that communication continues uninterrupted even when network components
fail. The level of protection depends on the number of redundant paths
configured - FRER can protect against single or multiple simultaneous failures.

Frame replication (IEEE 802.1CB) is a TSN mechanism that provides fault tolerance by creating and transmitting multiple copies of critical frames along diverse paths through the network. This ensures that even if one path fails due to link or node failures, at least one copy reaches the destination, enabling seamless network redundancy without packet loss.

**Key Principles:**

1. **Replication**: At designated replication points (typically near the source), each frame is duplicated and sent along multiple disjoint network paths

2. **Diverse Paths**: Frame copies travel through different network nodes and links to ensure path independence - if one path experiences a failure, other paths remain operational

3. **Sequence Numbering**: Each frame receives a unique sequence number (added via the IEEE 802.1CB header) to enable duplicate detection at the destination

4. **Elimination**: At elimination points (typically near the destination), duplicate frames are identified using their sequence numbers. Only the first arriving copy is forwarded to the application layer, while subsequent duplicates are discarded

5. **Transparent Operation**: From the application's perspective, frame replication is completely transparent - packet delivery continues uninterrupted even during network failures, with the application receiving each packet exactly once

The Model
---------

In this case, we use an automatic stream redundancy configurator that
takes the link and node failure protection requirements for each redundant stream
as an argument. The automatic configurator computes the different paths that each
stream must take in order to be protected against any of the listed failures so
that at least one working path remains.

The simulation demonstrates a controlled failure scenario:

- At ``t=20ms``, switch ``s2a`` crashes and becomes unavailable
- At ``t=80ms``, switch ``s2a`` recovers and becomes operational again

We run two configurations to compare the behavior:

1. **NoFrameReplication**: Standard operation without frame replication, demonstrating single point of failure
2. **FrameReplication**: Frame replication enabled with automatic configuration for failure protection

Here is the network:

.. figure:: media/Network.png
   :align: center

Configuration
~~~~~~~~~~~~~

The configuration is structured in two parts: a common [General] section that applies to both scenarios, and two derived configurations that enable or disable frame replication.

**Application Configuration**

The source generates UDP traffic at a constant rate:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-after: # source application
   :end-before: # destination application

The source sends 1200-byte UDP packets every 1ms (100 packets during the 100ms simulation). The destination simply receives and counts the packets:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-after: # destination application
   :end-before: # configure node shutdown/startup scenario

**Failure Scenario**

The ScenarioManager creates a deterministic node failure:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-after: # configure node shutdown/startup scenario
   :end-before: # visualizer

Switch s2a crashes at 20ms and recovers at 80ms, creating a 60ms outage window to test frame replication effectiveness.

**Frame Replication Configuration** (FrameReplication configuration only)

Enabling frame replication and elimination on all network devices:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: # enable frame replication and elimination
   :end-at: *.*.hasStreamRedundancy = true

This enables IEEE 802.1CB frame replication and elimination functionality at all nodes.

**Automatic Configurators** (FrameReplication configuration only)

Two configurators work together to set up the redundant paths:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: # enable all automatic configurators
   :end-at: *.failureProtectionConfigurator.typename = "FailureProtectionConfigurator"

The FailureProtectionConfigurator analyzes network topology and failure
requirements to compute redundant paths. The StreamRedundancyConfigurator then
configures the replication and elimination points based on these computed paths.

**MAC Configuration Requirements** (FrameReplication configuration only)

Two special MAC-related settings are required:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: # disable automatic MAC forwarding table configuration
   :end-before: # enable frame replication and elimination

The MAC forwarding table configurator must be disabled because frame replication
uses IEEE 802.1CB custom forwarding rules rather than standard MAC learning. All
destination interfaces share the same MAC address so frames arriving on any path
can be accepted and properly eliminated as duplicates.

**Failure Protection Rules** (FrameReplication configuration only)

The core of the configuration is the failure protection specification:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: # TSN configuration
   :end-before: # visualizer

This defines stream "S1" with traffic characteristics (1264B frames at 1ms
intervals, 100us max latency) and protection requirements: survive any single
node failure from {s2a, s2b, s3a, s3b}, and survive various link failure
combinations. The configurator uses these requirements to automatically compute
redundant paths.

Understanding the Failure Protection Configuration
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The automatic frame replication configuration relies on two key configurators
working together:

**Configurator Roles**

1. **FailureProtectionConfigurator**: Analyzes the network topology and failure
   protection requirements to compute redundant paths that survive specified
   failures

2. **StreamRedundancyConfigurator**: Based on the computed paths, configures the
   actual replication and elimination points throughout the network

**Stream Definition**

The configuration defines a stream named "S1" for traffic flowing from the source node to the destination node, specifically targeting packets from application instance app[0]. Each frame totals 1264B, consisting of 1200B application data plus 64B of protocol overhead (UDP, IP, IEEE 802.1CB frame replication, IEEE 802.1Q VLAN, Ethernet MAC/FCS, and PHY preamble). The stream operates with priority class 0, sends packets every 1ms, and requires delivery within 100us maximum latency.

The packet length overhead breakdown:

- **Application data**: 1200B (configured in source application)
- **UDP header**: 8B
- **IP header**: 20B
- **IEEE 802.1CB (Frame Replication)**: 4B
- **IEEE 802.1Q (VLAN)**: 6B
- **Ethernet MAC**: 14B (header) + 4B (FCS)
- **Ethernet PHY**: 8B (preamble)
- **Total overhead**: 64B
- **Total frame length**: 1200B + 64B = 1264B

**Node Failure Protection**

.. code-block::

   nodeFailureProtection: [{any: 1, of: "s2a or s2b or s3a or s3b"}]

This rule specifies that the stream must survive the failure of **any single
node** from the set {s2a, s2b, s3a, s3b}. The configurator will compute paths
such that if any one of these switches fails, at least one complete path from
source to destination remains operational.

**Link Failure Protection**

.. code-block::

   linkFailureProtection: [
       {any: 1, of: "*->* and not source->s1"},
       {any: 2, of: "s1->s2a or s2a->s2b or s2b->s3b"},
       {any: 2, of: "s1->s2b or s2b->s2a or s2a->s3a"}
   ]

These three rules define link failure protection:

1. **Rule 1**: ``{any: 1, of: "*->* and not source->s1"}`` 
   
   Protects against failure of any single link except the source->s1 link (which
   is unavoidable as the only connection from source)

2. **Rule 2**: ``{any: 2, of: "s1->s2a or s2a->s2b or s2b->s3b"}``
   
   Protects against simultaneous failure of any 2 links from the set {s1->s2a,
   s2a->s2b, s2b->s3b}, ensuring the upper path through s2a remains viable even
   if two of these links fail

3. **Rule 3**: ``{any: 2, of: "s1->s2b or s2b->s2a or s2a->s3a"}``
   
   Protects against simultaneous failure of any 2 links from the set {s1->s2b,
   s2b->s2a, s2a->s3a}, ensuring the lower path through s2b remains viable even
   if two of these links fail

The link failure protection rules are somewhat redundant for demonstration
purposes, but they illustrate how to specify complex failure scenarios.

**Special Configuration Requirements**

For frame replication to work properly, two special settings are required:

1. **Disable MAC Forwarding Table Configurator**:

   .. code-block::
   
      *.macForwardingTableConfigurator.typename = ""

   The automatic MAC forwarding table configurator must be disabled because
   frame replication uses custom forwarding rules (based on IEEE 802.1CB) rather
   than standard MAC learning.

2. **Unified MAC Address at Destination**:

   .. code-block::
   
      *.destination.eth[*].address = "0A-AA-12-34-56-78"

   All destination interfaces must share the same MAC address so that frames
   arriving on any path (via different interfaces) are accepted and can be
   properly eliminated as duplicates.

**Path Computation Result**

Based on these failure protection rules, the configurators automatically:

- Identify that frames must be replicated at switch s1 (the first divergence point)
- Route copies along two disjoint paths: s1→s2a→s3a→destination and s1→s2b→s3b→destination
- Configure elimination at the destination to remove duplicates
- Set up appropriate forwarding rules at each switch along both paths

This automatic configuration ensures that even if s2a fails (as in our
scenario), frames continue to flow via the s2b path without interruption.

**Redundant Path Visualization**

The following figures illustrate how the automatic configurators set up redundant paths in
the network.

The first figure shows where redundant streams are sent at each node in the network - the routing configuration that enables frames to traverse both the upper and lower redundant paths.

.. figure:: media/redundancy.png
   :align: center
   :width: 100%

The second figure shows the computed redundant paths through the network. Frames
are replicated at s1 and sent along two disjoint paths (highlighted): the upper
path through s2a→s3a and the lower path through s2b→s3b. At the destination,
duplicate frames are eliminated, ensuring the application receives each packet
exactly once.

.. figure:: media/redundantpaths.png
   :align: center
   :width: 100%

Results
-------

NoFrameReplication Configuration
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Without frame replication, the network has a single point of failure. When switch ``s2a`` 
crashes at 20ms, all packets following that path are lost. After the switch recovers 
at 80ms, packet delivery resumes, but packets sent during the outage period (20-80ms) 
are permanently lost.

FrameReplication Configuration
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

With frame replication enabled, the source replicates packets and sends them along 
multiple redundant paths. When ``s2a`` crashes, packets continue to be delivered via 
the alternative path through ``s2b`` and ``s3b``. The destination eliminates duplicate 
packets, ensuring continuous delivery throughout the simulation even during the failure period.

Here is the number of received and sent packets:

.. figure:: media/packetsreceivedsent.png
   :align: center
   :width: 100%

Here is the ratio of received and sent packets:

.. figure:: media/packetratio.png
   :align: center

The comparison clearly shows the advantage of frame replication:

- **NoFrameReplication**: Significant packet loss during the failure period (20-80ms)
- **FrameReplication**: All packets successfully delivered despite the node failure

.. The following video shows the behavior in Qtenv:

   .. video:: media/behavior.mp4
      :align: center
      :width: 90%

   Here are the simulation results:

   .. .. figure:: media/results.png
      :align: center
      :width: 100%


Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`AutomaticFailureProtectionShowcase.ned <../AutomaticFailureProtectionShowcase.ned>`


Try It Yourself
---------------

If you already have INET and OMNeT++ installed, start the IDE by typing
``omnetpp``, import the INET project into the IDE, then navigate to the
``inet/showcases/tsn/framereplication/automaticfailureprotection`` folder in the `Project Explorer`. There, you can view
and edit the showcase files, run simulations, and analyze results.

Otherwise, there is an easy way to install INET and OMNeT++ using `opp_env
<https://omnetpp.org/opp_env>`__, and run the simulation interactively.
Ensure that ``opp_env`` is installed on your system, then execute:

.. code-block:: bash

    $ opp_env run inet-4.5 --init -w inet-workspace --install --build-modes=release --chdir \
       -c 'cd inet-4.5.*/showcases/tsn/framereplication/automaticfailureprotection && inet'

This command creates an ``inet-workspace`` directory, installs the appropriate
versions of INET and OMNeT++ within it, and launches the ``inet`` command in the
showcase directory for interactive simulation.

Alternatively, for a more hands-on experience, you can first set up the
workspace and then open an interactive shell:

.. code-block:: bash

    $ opp_env install --init -w inet-workspace --build-modes=release inet-4.5
    $ cd inet-workspace
    $ opp_env shell

Inside the shell, start the IDE by typing ``omnetpp``, import the INET project,
then start exploring.

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/787>`__ page in the GitHub issue tracker for commenting on this showcase.

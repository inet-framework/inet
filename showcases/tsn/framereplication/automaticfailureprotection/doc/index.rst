Automatic Stream Configuration with Failure Protection
======================================================

Goals
-----

In this example, we demonstrate the automatic stream redundancy configuration based
on the link and node failure protection requirements. We compare the behavior with
and without frame replication when a node fails during the simulation, showing how
frame replication provides resilience against single points of failure.

| Verified with INET version: ``4.4``
| Source files location: `inet/showcases/tsn/framereplication/automaticfailureprotection <https://github.com/inet-framework/inet/tree/master/showcases/tsn/framereplication/automaticfailureprotection>`__

Frame Replication and Elimination
----------------------------------

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

Here is the configuration:

.. literalinclude:: ../omnetpp.ini
   :language: ini

Configuration Details
~~~~~~~~~~~~~~~~~~~~~

The key configuration parameters include:

- **Traffic Pattern**: UDP packets sent at 1ms intervals (100 packets total in 100ms simulation)
- **Packet Size**: 1200B application data + protocol overhead
- **Node Status**: Enabled to support the ScenarioManager crash/startup events
- **Failure Protection**: Configured to protect against failure of any single node (s2a, s2b, s3a, or s3b)

In the **FrameReplication** configuration:

- Frame replication and elimination is enabled on all devices
- The FailureProtectionConfigurator automatically determines redundant paths
- The StreamRedundancyConfigurator configures the replication points and elimination points
- All destination interfaces share the same MAC address to accept packets from all streams

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

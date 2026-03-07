Step 9. Dumbbell Topology: Two Clusters with a Bottleneck
==========================================================

Goals
-----

This step introduces a larger, dumbbell-shaped network where two internally
meshed switch clusters are connected only through two bridge switches in the
middle. After RSTP converges, both bridge switches are shut down
simultaneously, splitting the network into two disconnected halves. This
demonstrates what happens when a bottleneck — the only connection between two
parts of a network — is lost entirely.

The Network
-----------

The simulation uses the ``BottleneckNetwork`` network. It consists of:

- **Left cluster**: four switches (``switchL1``–``switchL4``) with redundant
  internal links and two hosts (``host1``, ``host2``) on the outer edge.
- **Right cluster**: four switches (``switchR1``–``switchR4``) with the same
  meshed structure and two hosts (``host3``, ``host4``).
- **Bottleneck**: two bridge switches (``switch11``, ``switch12``) that provide
  the only paths between the two clusters. ``switch11`` connects ``switchL3``
  to ``switchR3``, and ``switch12`` connects ``switchL4`` to ``switchR4``.

.. figure:: media/BottleneckNetwork.png
   :align: center

The dumbbell shape means that all cross-cluster traffic must traverse at least
one of the two bridge switches. Within each cluster, the mesh provides
redundant paths, but between clusters only two independent paths exist.

Configuration
~~~~~~~~~~~~~

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config Step9]

RSTP is enabled on all switches. Fixed MAC addresses ensure deterministic root
bridge election (``switchL1`` has the lowest address and becomes root).
All four hosts send traffic across the bottleneck starting at t=15 s, after
RSTP has converged.

The bridge switch failures are driven by
:download:`bottleneck_failure.xml <../bottleneck_failure.xml>`:

.. literalinclude:: ../bottleneck_failure.xml
   :language: xml

At t=30 s — well after RSTP convergence — both ``switch11`` and ``switch12``
are shut down simultaneously.

Network Design Considerations
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This scenario illustrates an important principle: **STP and RSTP can only
remove redundant paths to prevent loops — they cannot create paths that do not
physically exist.** If the physical topology becomes disconnected (here, by
losing both bridge switches), no spanning tree protocol can restore
connectivity between the separated parts.

The dumbbell topology is a common pattern in real networks: two groups of
switches (e.g. two buildings, two floors, or two data-center pods) connected
through a limited number of uplinks. These uplinks form a *bottleneck* — and
if all of them fail simultaneously, the network partitions.

Strategies to mitigate this in real deployments include:

- **Diverse physical paths**: running links through different conduits, rooms,
  or buildings so that a single physical event cannot sever all connections.
- **More uplinks**: increasing the number of independent paths between clusters
  so that losing one or two does not cause a partition.
- **Link aggregation**: bundling multiple physical links into a single logical
  link (IEEE 802.1AX), which STP/RSTP treats as one connection but which
  survives individual member link failures.

Results
~~~~~~~

**Before failure (t=0–30 s)**: RSTP converges in ~6 s. The spanning tree
spans both clusters through the bridge switches. All four hosts communicate
across the bottleneck.

.. figure:: media/step9before.png
   :width: 90%
   :align: center

**At t=30 s** — Both bridge switches shut down. The two clusters become
completely disconnected. RSTP detects the failure and rebuilds a spanning tree
within each half: hosts in the left cluster (``host1``, ``host2``) can still
reach each other, and likewise for the right cluster (``host3``, ``host4``).
Cross-cluster traffic (e.g. ``host1`` → ``host3``) is lost entirely — as
discussed above, RSTP cannot bridge a physical gap.

.. figure:: media/step9after.png
   :width: 90%
   :align: center

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`StpTutorial.ned <../StpTutorial.ned>`,
:download:`bottleneck_failure.xml <../bottleneck_failure.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet/discussions>`__ in
the GitHub issue tracker for commenting on this tutorial.

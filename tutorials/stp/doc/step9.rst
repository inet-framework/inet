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

Results
~~~~~~~

**Before failure (t=0–30 s)**: RSTP converges in ~6 s. The spanning tree
spans both clusters through the bridge switches. All four hosts communicate
across the bottleneck.

.. figure:: media/step9before.png
   :width: 90%
   :align: center

**At t=30 s** — Both bridge switches shut down. The two clusters become
completely disconnected. Hosts in the left cluster (``host1``, ``host2``)
can still reach each other through the left cluster's internal mesh, and
likewise for the right cluster (``host3``, ``host4``). However, cross-cluster
traffic (e.g. ``host1`` → ``host3``) is lost entirely — there is no
alternate path between the two halves.

.. figure:: media/step9after.png
   :width: 90%
   :align: center

RSTP detects the failure and rebuilds a spanning tree within each
disconnected half, but it cannot restore cross-cluster connectivity because
no physical path exists.

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`StpTutorial.ned <../StpTutorial.ned>`,
:download:`bottleneck_failure.xml <../bottleneck_failure.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet/discussions>`__ in
the GitHub issue tracker for commenting on this tutorial.

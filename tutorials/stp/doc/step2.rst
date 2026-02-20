Step 2. Basic STP Operation
============================

Goals
-----

This step demonstrates how the Spanning Tree Protocol (STP) builds a loop-free
tree topology from a meshed Ethernet network. We will observe the root bridge
election, port role assignment, and which links are blocked to break the loops.

The Network
-----------

The simulation uses the ``SwitchNetwork`` network, which contains seven Ethernet
switches (``switch1``–``switch7``) interconnected with redundant links, and two
hosts (``host1`` and ``host2``) attached to ``switch6`` and ``switch5``
respectively.

.. figure:: media/step2network.png
   :width: 80%
   :align: center

About STP
~~~~~~~~~

STP (IEEE 802.1D-1998) builds a spanning tree over a bridged network to ensure
a loop-free topology. The protocol works as follows:

1. **Root bridge election**: The switch with the lowest bridge ID (priority +
   MAC address) becomes the root bridge. By default all switches have the same
   priority (32768), so the one with the lowest MAC address wins.

2. **Root port selection**: Each non-root switch selects the port with the
   lowest path cost to the root as its *root port*.

3. **Designated port selection**: On each network segment, one port is elected
   *designated port* (the one closest to the root). The other ports on that
   segment are put in *blocking* state.

4. **Port states**: STP ports transition through *Blocking → Listening →
   Learning → Forwarding*, with each transition taking ``forwardDelay`` seconds
   (default 15 s). The total convergence time is therefore approximately 30–50 s.

In INET, STP is implemented by the :ned:`Stp` module, which is enabled per
switch via the ``hasStp = true`` and ``spanningTreeProtocol = "Stp"`` parameters.

Configuration
~~~~~~~~~~~~~

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config Step2]
   :end-before: ------

The MAC addresses are fixed (``AAAAAA000001`` … ``AAAAAA000007``) so that
``switch1`` (lowest MAC) is always elected root. The ``visualize = true``
parameter colors blocked links gray and active (forwarding) links black, and
highlights the root bridge in cyan.

Traffic from ``host2`` to ``host1`` starts at t=60s, after STP has converged.

Results
~~~~~~~

After the simulation starts, STP BPDUs (Bridge Protocol Data Units) are exchanged
between switches. After approximately 50 s, the spanning tree has converged:

- **Root bridge**: ``switch1`` (lowest MAC address ``AAAAAA000001``)
- **Blocked ports**: the ports creating redundant paths are blocked (shown in gray)
- **Active tree**: a subset of links forms a tree rooted at ``switch1``

.. figure:: media/step2result.png
   :width: 80%
   :align: center

After convergence (t=60s), ``host2`` begins sending frames to ``host1`` and
receives replies, confirming that the tree provides full connectivity while
remaining loop-free.

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`StpTutorial.ned <../StpTutorial.ned>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet/discussions>`__ in
the GitHub issue tracker for commenting on this tutorial.

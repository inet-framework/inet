Step 6. Topology Change with STP: Switch Failure and Recovery
=============================================================

Goals
-----

This step demonstrates how STP handles a topology change caused by a switch
failure. We will observe how the spanning tree rebuilds after a switch goes
down, and how long STP takes to restore connectivity.

The Network
-----------

The simulation uses the ``LargeNet`` network: 11 switches and 6 hosts with a
heavily meshed topology, providing many redundant paths. This gives the spanning
tree more work to do when a failure occurs.

.. figure:: media/step6network.png
   :width: 90%
   :align: center

Configuration
~~~~~~~~~~~~~

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config Step6]
   :end-before: ------

Switch failures are driven by :download:`failure.xml <../failure.xml>`:

.. literalinclude:: ../failure.xml
   :language: xml

``switch4`` shuts down at t=60 s and restarts at t=120 s. The ``hasStatus = true``
parameter enables lifecycle operations (shutdown/startup) on all nodes.

Traffic between all host pairs starts at t=10 ms and runs continuously,
so any disruption to connectivity is immediately visible.

Results
~~~~~~~

**Before failure (t=0–60 s)**: STP converges in ~50 s. All hosts communicate
through the active spanning tree. ``switch1`` is root (lowest MAC address).

**At t=60 s** — ``switch4`` shuts down. The switches that had ports leading
to ``switch4`` detect the loss of BPDUs after ``maxAge`` seconds (default 20 s).
Then the tree rebuilds, going through the full Listening → Learning →
Forwarding cycle (another 30 s). Total re-convergence time after failure:
approximately **50 s**.

During this period, hosts that depended on paths through ``switch4`` lose
connectivity. When the tree re-stabilizes, they can communicate again via
the alternate paths (the topology has enough redundancy).

**At t=120 s** — ``switch4`` restarts. STP again goes through the full
convergence cycle before the switch rejoins the active tree.

.. figure:: media/step6failure.png
   :width: 90%
   :align: center

The slow convergence after failure is the main limitation of STP. The next
step shows how RSTP handles the same failure much faster.

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`StpTutorial.ned <../StpTutorial.ned>`,
:download:`failure.xml <../failure.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet/discussions>`__ in
the GitHub issue tracker for commenting on this tutorial.

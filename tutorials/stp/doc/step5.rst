Step 5. Topology Change with STP: Switch Failure and Recovery
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

.. figure:: media/LargeNet.png
   :align: center

Configuration
~~~~~~~~~~~~~

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config Step5]
   :end-before: ------

Switch failures are driven by :download:`failure.xml <../failure.xml>`:

.. literalinclude:: ../failure.xml
   :language: xml

``switch4`` shuts down at t=60 s and restarts at t=120 s. The ``hasStatus = true``
parameter enables lifecycle operations (shutdown/startup) on all nodes.

Traffic between all host pairs starts at t=10 ms and runs continuously,
so any disruption to connectivity is immediately visible.

How STP Handles Topology Changes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

When a switch detects a topology change — such as a port going down or a new
port transitioning to Forwarding — STP uses a two-phase notification process:

1. The detecting switch sends a *Topology Change Notification* (TCN) BPDU
   upstream toward the root bridge via its root port. Each intermediate switch
   relays the TCN further upstream and acknowledges it downstream.
2. When the root bridge receives the TCN, it sets the *Topology Change* (TC)
   flag in its periodic Configuration BPDUs for a period of
   ``maxAge + forwardDelay`` seconds (default 20 + 15 = 35 s).
3. All switches that receive a TC-flagged BPDU reduce their MAC address aging
   time from the normal ``agingTime`` (default 300 s) to ``forwardDelay``
   (15 s). This causes stale entries to expire quickly, forcing the switches
   to re-learn the correct port mappings through flooding.

This mechanism is slow because it depends on the root bridge as a relay point,
and the MAC tables are only gradually flushed through accelerated aging rather
than immediately cleared.

The total reconvergence time after a failure can be broken down as:

- **maxAge** (20 s): neighboring switches wait for the failed switch's stored
  BPDU information to expire before they accept that the topology has changed.
- **Listening** (15 s): ports with new roles go through the Listening state.
- **Learning** (15 s): then through the Learning state.
- **Total: ~50 s** before data flows again on the reconfigured tree.

Results
~~~~~~~

**Before failure (t=0–60 s)**: STP converges in ~50 s. All hosts communicate
through the active spanning tree. ``switch1`` is root (lowest MAC address).

**At t=60 s** — ``switch4`` shuts down. The switches that had ports connected
to ``switch4`` stop receiving BPDUs on those ports. After ``maxAge`` (20 s)
expires, they discard the stored BPDU information and re-evaluate their port
roles. The affected ports then transition through Listening → Learning →
Forwarding (another 30 s). During this ~50 s period, hosts that depended on
paths through ``switch4`` lose connectivity.

When the tree re-stabilizes, they can communicate again via alternate paths
(the topology has enough redundancy).

**At t=120 s** — ``switch4`` restarts. STP again goes through the full
convergence cycle before the switch rejoins the active tree.

.. figure:: media/step6failure.png
   :width: 90%
   :align: center

The slow convergence after failure is the main limitation of STP. The next
step introduces RSTP, which converges much faster both at startup and after
topology changes.

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`StpTutorial.ned <../StpTutorial.ned>`,
:download:`failure.xml <../failure.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet/discussions>`__ in
the GitHub issue tracker for commenting on this tutorial.

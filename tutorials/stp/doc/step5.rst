Step 5. RSTP: Faster Convergence
=================================

Goals
-----

This step introduces the Rapid Spanning Tree Protocol (RSTP, IEEE 802.1D-2004),
which dramatically reduces convergence time compared to STP. We will run the
same network as Steps 2–4 with RSTP enabled and observe the difference.

About RSTP
~~~~~~~~~~

The Rapid Spanning Tree Protocol (RSTP, IEEE 802.1D-2004) is the successor to
STP. It produces the same loop-free spanning tree but converges dramatically
faster — typically in a few seconds instead of 30–50 s. RSTP achieves this by
replacing STP's passive timer-based transitions with active handshakes between
neighboring switches.

**Simplified port states.** RSTP reduces the four STP states to three:

- **Discarding**: combines STP's Blocking and Listening. The port does not
  forward data and does not learn MAC addresses. (This is the state displayed
  by INET's visualization for both STP and RSTP.)
- **Learning**: the port learns MAC addresses but does not yet forward data.
- **Forwarding**: the port forwards data normally.

**Port roles.** RSTP keeps the Root and Designated roles from STP and adds two
new ones:

- **Alternate port**: a port that has a viable path to the root through a
  *different* upstream switch, but with a higher cost than the current root
  port. It serves as a hot standby — if the root port fails, the best
  alternate port can be promoted to root port *immediately*, without running a
  new election.
- **Backup port**: a port that receives BPDUs from the *same* switch it
  belongs to (possible on shared/hub segments). It is a backup for a
  designated port. This role is rare in modern switched networks.

The distinction matters for failure recovery: an Alternate port already knows
it has a loop-free path to the root, so it can take over without delay.

**The Proposal/Agreement mechanism.** This is the key innovation that
eliminates the Listening/Learning delays. When a switch determines that one of
its ports should become Designated, the following handshake occurs:

1. The switch sends a BPDU with the *Proposal* flag set on the port.
2. The downstream switch receives the proposal. Before it can agree, it must
   ensure no loops will form — so it puts all its own non-edge ports into
   Discarding state (a process called *sync*).
3. Once sync is complete, the downstream switch sends a BPDU with the
   *Agreement* flag back to the upstream switch.
4. The upstream switch receives the agreement and immediately transitions its
   port to Forwarding — no timer wait needed.

This handshake cascades through the network: each switch syncs and agrees with
its upstream neighbor, rippling outward from the root. The entire tree
converges within a few hello intervals.

**Edge ports.** Ports connected to end hosts (not other switches) can never
create loops, so they can transition directly to Forwarding without any delay
or handshake. RSTP calls these *edge ports*. In INET, edge ports are detected
automatically when ``autoEdge = true`` (the default): if a port does not
receive any BPDUs within the first ``migrateTime`` seconds, it is classified
as an edge port. If an edge port later receives a BPDU (indicating a switch
has been connected), it loses its edge status and participates in the normal
proposal/agreement process.

**Topology change handling.** When RSTP detects a topology change (e.g. a
non-edge port transitioning to Forwarding), the detecting switch sets the *TC*
(Topology Change) flag directly in its BPDUs, and this flag is flooded
throughout the network. Each switch that receives a TC-flagged BPDU flushes
learned MAC addresses on all ports except the one the BPDU arrived on and any
edge ports. How this compares to STP's topology change mechanism is discussed
in Steps 6 and 7.

In INET, RSTP is implemented by the :ned:`Rstp` module. Key parameters:

- ``forwardDelay`` (default 6 s): used only as a fallback when the
  proposal/agreement handshake cannot be used; rapid transitions bypass this
- ``migrateTime`` (default 3 s): time before an unassigned port becomes
  Designated; also used for edge port detection
- ``autoEdge`` (default true): automatically detect and configure edge ports

Configuration
~~~~~~~~~~~~~

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config Step5]
   :end-before: ------

The only change from the STP configuration is ``spanningTreeProtocol = "Rstp"``.
The simulation runs for only 30 s (versus 200 s for the STP step), because
RSTP converges quickly. Traffic starts at t=15 s.

Results
~~~~~~~

With RSTP, the spanning tree converges in approximately 6 s — roughly eight
times faster than STP. The port state labels on switch interfaces cycle
much more quickly through the RSTP states (Discarding → Learning → Forwarding).

.. figure:: media/step5result.png
   :width: 80%
   :align: center

Edge ports (ports connected to ``host1`` and ``host2``) transition to Forwarding
almost immediately (within one hello interval ≈ 2 s), because they are
automatically identified as edge ports.

At t=15 s, traffic from ``host2`` reaches ``host1`` reliably, demonstrating
that the tree has converged well before the STP equivalent would have.

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`StpTutorial.ned <../StpTutorial.ned>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet/discussions>`__ in
the GitHub issue tracker for commenting on this tutorial.

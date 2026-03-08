Step 6. RSTP: Faster Convergence
=================================

Goals
-----

The previous steps showed that STP takes approximately 50 s to converge — both
at startup and after a topology change. This step introduces the Rapid Spanning
Tree Protocol (RSTP, IEEE 802.1D-2004), which dramatically reduces convergence
time. We will run the same base network as Steps 2–4 with RSTP enabled and
observe the difference.

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
eliminates the Listening/Learning delays. The basic idea is that a port can
skip the long timer-based transition to Forwarding *if its downstream neighbor
explicitly confirms that doing so is safe* (i.e. will not create a loop). This
confirmation happens through a quick two-message handshake.

To understand the mechanism, it helps to think of it as a **wave of activation
spreading outward from the root bridge**, one hop at a time:

1. **The root bridge starts the wave.** After election, the root bridge knows
   all its ports should be Designated. It sends a BPDU with the *Proposal*
   flag set on each port, effectively asking each downstream neighbor: "I want
   to start forwarding on this link — is it safe?"

2. **Each downstream switch syncs.** When a switch receives a Proposal on a
   port, it must guarantee that agreeing will not create a loop. It does this
   by temporarily forcing *all* its other non-edge ports into Discarding state.
   This is called *sync*: the switch is essentially saying "I'll block
   everything else first, so there is provably no path through me that could
   form a loop." Edge ports (host-facing) are exempt because they can never
   create loops.

3. **The downstream switch agrees.** Once sync is complete (all other non-edge
   ports are Discarding), the switch sends a BPDU with the *Agreement* flag
   back on the port where it received the Proposal. It also marks that port as
   its Root port and moves it to Forwarding immediately.

4. **The upstream switch activates.** Upon receiving the Agreement, the
   upstream switch moves its Designated port to Forwarding immediately — no
   timer wait needed. The link between these two switches is now fully active
   in both directions.

5. **The wave continues.** The downstream switch now has several ports that
   were forced into Discarding during sync. For each port that should be
   Designated (toward further downstream switches), it starts *its own*
   Proposal/Agreement handshake — repeating steps 1–4 one hop further from the
   root. As each handshake completes, the temporarily blocked ports are
   released from Discarding and moved to Forwarding.

6. **Leaf switches finish the wave.** When the wave reaches a switch whose
   only downstream connections are edge ports (end hosts), no further
   handshakes are needed. Edge ports go to Forwarding immediately.

The net effect is that the tree is activated **hop by hop from root to leaves**.
Each hop takes only one round-trip exchange (a few milliseconds on a LAN), so
the entire tree converges within a few hello intervals — typically under 6 s
for the whole network, compared to STP's 30–50 s.

Note that during the brief sync phase at each hop, some ports are temporarily
in Discarding. This is the cost of safety: RSTP momentarily interrupts
forwarding on those ports to guarantee loop-freedom. However, since each
handshake completes in milliseconds, these interruptions are negligible in
practice.

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
edge ports. Compare this with STP's slower TCN mechanism described in Step 5:
RSTP does not route notifications through the root, and flushes MAC tables
immediately rather than merely reducing the aging time.

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
   :start-at: [Config Step6]
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

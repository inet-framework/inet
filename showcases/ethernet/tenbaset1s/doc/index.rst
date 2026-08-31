10BASE-T1S Multidrop Ethernet with PLCA
=======================================

Goals
-----

Automotive and industrial networks are beginning to replace CAN and similar
fieldbuses with 10BASE-T1S, a 10 Mbps Ethernet that connects up to a few dozen
nodes to one unshielded twisted pair. Carrying ordinary Ethernet frames all the
way out to the sensor and actuator edge removes a protocol translation from the
vehicle or the machine, but it also inherits Ethernet's original problem: on a
shared wire, arbitration by collision produces random waiting times with no
upper limit. Control traffic cannot be designed against a delay that has no
worst case.

Physical Layer Collision Avoidance (PLCA) is the mechanism the standard adds to
solve this. It rotates a single transmit opportunity through the nodes of the
segment in a fixed order, so frames never collide on the wire, and the longest
a node can ever wait is one full rotation — a duration that can be computed in
advance from the node count and the frame size.

In this showcase, we compare the two arbitration schemes on the same
10BASE-T1S segment, with identical hardware, traffic and cabling, switched
between them by a single configuration line. We measure worst-case medium
access latency against the analytical bound, how that bound scales as nodes are
added, and what each scheme delivers when every station always has a frame to
send. By the end of this showcase, you will understand how PLCA arbitrates the
medium, how to compute the access-latency guarantee for a segment you are
designing, and what plain CSMA/CD (Carrier Sense Multiple Access with Collision
Detection) costs on the same wire.

| Verified with INET version: ``4.7``
| Source files location: `inet/showcases/ethernet/tenbaset1s <https://github.com/inet-framework/inet/tree/master/showcases/ethernet/tenbaset1s>`__

.. TODO: confirm the release version number that contains commits e342d04b0e,
   1bc2af2784, 5e0d2fce22 before publishing; also applies to the opp_env
   version below.

About 10BASE-T1S and PLCA
-------------------------

10BASE-T1S (IEEE 802.3cg-2019) is the short-reach member of the single-pair
Ethernet family. Its multidrop mode connects up to at least 8 nodes — a floor,
not a cap — to one twisted pair of up to at least 25 m: a shared bus the
standard calls a mixing segment. It targets the sensor and actuator edge of
vehicles and machines, where it is positioned to take over roles held by CAN,
LIN, and FlexRay, using ordinary Ethernet frames. Do not confuse it with
10BASE-T1L, the long-reach, point-to-point sibling.

On a shared wire, plain CSMA/CD access is random: colliding nodes back off for
random times, so waiting times have no upper limit. PLCA avoids this. It
rotates a single transmit opportunity through the node IDs in fixed order;
node 0 — often called the PLCA coordinator (OPEN Alliance) — sends a short
beacon signal to start each cycle. A node with a frame ready transmits in its
own opportunity; a node with nothing to send yields it after a few
microseconds. The Ethernet MAC itself is unchanged: PLCA is a thin sublayer
below it that simply delays the MAC until the node's opportunity arrives.
Unlike CAN, the rotation carries no frame priorities — every node waits its
turn, whatever the message.

The Model
---------

The network is one mixing segment: N edge nodes and one zonal controller. In a
real design the controller bridges this segment to the vehicle backbone; that
side is not modeled here. All nodes are :ned:`EthernetPlcaHost` — a host whose
applications exchange raw Ethernet frames through an interface with a PLCA
sublayer. The pair itself is a chain of :ned:`WireJunction` modules connected
by :ned:`EthernetMultidropLink` cables, with 1 m of trunk between neighbors and
10 cm stubs down to the nodes, spacing that real harnesses use.

.. figure:: media/network.png
   :align: center

..
   FIGURE RECIPE (redo via the "omnetpp-mcp-sim" skill)
   type:     canvas
   config:   CycleAnatomy   # omnetpp.ini
   seed:     default (r 0)
   shows:    one 10BASE-T1S mixing segment - controller + 8 edge nodes on a
             daisy-chained WireJunction bus (N=8 operating point)
   anchor:   initial state (t=0, before run). Structural self-check: controller,
             j[0..7], node[0..7]; if the module set/links differ, the NED changed.
   capture:  get_canvas_image, module_path=MultidropNetwork, area=all_elements,
             margin=10; cropped to 1174x400 (empty bottom band removed)
   stamp:    captured 2026-08-14, INET @ 5e0d2fce22 + showcase, OMNeT++ 6.4.0aipre2


The convention used throughout is that N counts the edge nodes, and the
controller is PLCA node 0, so a cycle has N+1 transmit opportunities. The
network file assigns the two mandatory PLCA parameters accordingly, and lays
out the trunk and the stubs:

.. literalinclude:: ../MultidropNetwork.ned
   :language: ned
   :start-at: network MultidropNetwork

Every configuration inherits one of two abstract base sections. The CSMA/CD
variant differs in a single line, which removes the ``plca`` submodule; the
MAC, PHY, queue, and cable are identical on both sides:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config PlcaBase]
   :end-at: **.plca.typename = ""

The PLCA base section pins ``max_bc = 0``, which turns off bursting — sending
several frames in one opportunity. That is the default, but the in-tree example
configurations enable it, and bursting changes the bound arithmetic derived
below.

In the latency configurations, every edge node sends 64-byte frames to the
controller cyclically, and the controller sends frames back at a quarter of
that volume. A 64-byte frame is the ini's ``packetLength = 46B`` payload plus
18 bytes of header and frame check sequence. Each production interval is
jittered by ±10% so that the nodes drift out of phase, and each node starts at
a random offset. Offered load L counts total wire occupancy in both directions,
which gives these per-node periods at N=8:

===================== ========= ========= ======== ========
Offered load L        30%       50%       70%      85%
===================== ========= ========= ======== ========
Period per node       2.267 ms  1.360 ms  971 µs   800 µs
===================== ========= ========= ======== ========

In the period formula behind this table, 68 µs is one fully busy 64-byte
transmit opportunity — 680 bit times, the frame plus its gap — and the factor
1.25 accounts for the uplink plus the quarter-volume downlink. The top loads
match the aggregate regime Ford bench-tested in their node-count analysis
(cited under *Sources* below); designed operating points typically sit well
below the tested top loads.

The PLCA Cycle Up Close
-----------------------

Every PLCA node keeps a counter, ``curID``, that tracks which node ID owns the
current transmit opportunity. Watching that counter during a run shows the
rotation directly:

.. figure:: media/to-rotation.png
   :align: center

..
   FIGURE RECIPE (redo via the "inet-showcase-charts" skill)
   type:    chart (matplotlib)
   anf:     TenBaseT1S.anf   chart "Transmit opportunity rotation"
   inputs:  results/CycleAnatomy-*.vec (plca.curID vectors, controller + node[0])
   shows:   two panels. Upper: curID staircase over ~4 idle cycles (window 950-1075 us),
            IDs 0..8 in fixed order, stepping past the last ID to 9, then the node-0
            BEACON resets all counters; the grey band marks the cycle wrap that the
            lower panel expands. Lower: that one cycle wrap, with the measured 2.0 us
            BEACON shaded gold. Grey = "expanded below", gold = the beacon; the two
            shadings are deliberately different colours so they are not equated.
            The controller is drawn as a wide blue band with node[0] on top, so both
            curves are visible where they coincide (they always do).
   anchors: idle-TO dwell 3.2 us; empty cycle 30.8 us; BEACON 2.0 us (all measured in this
            window at G4). If the staircase shape or these dwells change, the PLCA timing
            model changed - re-derive.
   export:  opp_charttool imageexport TenBaseT1S.anf -n "Transmit opportunity rotation"
            -f png --dpi 150 -d doc/media   (8x7.5 in -> 1200x1125 px)
   stamp:   captured 2026-08-14, INET topic/gy/tenbaset1s-showcase @ 5e0d2fce22 + showcase
            dir, OMNeT++ 6.4.0aipre2 (omnetpp-dev). Layout revised 2026-08-17: the beacon
            zoom moved from an inset to a lower panel, same data. The superseded
            single-panel rendering is kept at doc/media/unused/to-rotation.png.


The controller and node[0] curves coincide — their sampled values never differ
in this run, and only nanosecond propagation offsets separate them — so all
nodes count in lockstep. That is why the thin orange line rides exactly on the
wide blue controller band everywhere. Each staircase tops out at 9: every node
steps past the last ID, and on node 0 passing the node count triggers the next
beacon, which resets all counters. The counters also start at 9 before the
first beacon, which lies outside the plotted window. The lower panel expands
the grey-marked cycle wrap from the upper panel and shades node 0's beacon,
whose measured duration is 2.00 µs.

The next chart shows what the opportunities themselves look like on the wire,
over the tail of a busy start-up cycle followed by one completely empty cycle:

.. figure:: media/to-timelines.png
   :align: center

..
   FIGURE RECIPE (redo via the "inet-showcase-charts" skill)
   type:    chart (matplotlib)
   anf:     TenBaseT1S.anf   chart "Transmit opportunity timelines"
   inputs:  results/CycleAnatomy-*.vec (phy signal-type lanes: controller received = the
            bus, controller transmitted = beacons, node[0] transmitted); chart form follows
            examples/ethernet/TenBaseT1S worstcase/bestcase .anf (plot_vectors_separate +
            enum strips); window 505-700 us = tail of the busy start-up cycle + one
            empty cycle
   shows:   last DATA of the busy start-up cycle, BEACON, the controller's own 3.2 us
            yielded TO,
            node[0] COMMIT+DATA, seven 3.2 us yields, then a whole 30.8 us empty cycle -
            idle slots cost microseconds, not frame times.
   anchors: BEACON 2.0 us at cycle start (controller lane); busy TO = 680 BT = 68.0 us
            total (observed here as COMMIT 9.6 us + DATA 58.4 us; the stretched COMMIT
            is start-up alignment against the preceding beacon/yield - do not chase a
            ~71 us figure); yielded TO 3.2 us. Measured at G4; if the lane grammar
            changes, the PLCA FSM or scenario changed - re-derive.
   export:  opp_charttool imageexport TenBaseT1S.anf -n "Transmit opportunity timelines"
            -f png --dpi 150 -d doc/media   (8x6 in -> 1200x900 px)
   stamp:   captured 2026-08-14, INET topic/gy/tenbaset1s-showcase @ 5e0d2fce22 + showcase
            dir, OMNeT++ 6.4.0aipre2 (omnetpp-dev)


The three lanes are a bus view. The top lane is everything the controller
receives, which is effectively the bus itself; the middle lane is what the
controller transmits, meaning its beacons and its own frames; the bottom lane
is what node[0] transmits. The thin slivers in the controller's transmit lane
are the 2 µs beacons. A COMMIT is the short claim signal a node sends at the
start of its opportunity while its frame is still on the way. Yielded
opportunities appear as 3.2 µs silences, so an idle slot costs microseconds
rather than a frame time, and a fully idle cycle takes 30.8 µs as measured
here.

That structure is what makes the worst case computable. The arithmetic comes
from the standard's own cycle structure, with two INET accounting details: a
1 ns gap after the beacon at the start of each cycle, and the end-of-stream
delimiter counted in the frame's wire time. That delimiter costs one
octet-time per the standard's own line encoding — it is not a simulator quirk.

A busy transmit opportunity costs the frame on the wire (preamble, frame and
delimiter, which is 584 bit times at 64 B) plus one 96-bit interpacket gap,
giving 680 bit times. It does not also pay the opportunity timer, because the
COMMIT stops the other nodes' timers and covers the sender's own gap. An idle
opportunity costs the 32 bit-time opportunity timer, and the beacon costs 20
bit times. The worst case for a node is therefore one fully busy cycle, in
which every other node transmits before its own turn comes round. At N=8 with
64 B frames that is 20 + 9 × 680 = 6140 bit times, or **614 µs**; in general
the bound is 68 µs × (N+1) + 2 µs, equivalently written 68·N + 70 µs.

This is the number a segment is designed against. At N=8 with 64 B frames,
every node is guaranteed a transmit opportunity at least every 0.61 ms or so,
so a 10 ms control loop fits with more than 15× margin. The bound scales with
the largest frame any node on the segment may send, which means it should be
budgeted for that frame rather than the typical one: at 1518 B the same
eight-node segment guarantees only 11.1 ms. Counting only MAC frame bits, the
fully busy cycle carries 75.0% of the wire at 64 B, or 98.6% at 1518 B.

Results
-------

Access latency under load
~~~~~~~~~~~~~~~~~~~~~~~~~

Access latency here is measured from a frame's first transmission attempt until
the transmission that succeeds begins. Time spent queued behind the station's
own earlier frames is not counted, and neither is a head-of-queue frame's
initial wait for a busy medium, which is up to about 70 µs here; even adding
that wait back, PLCA stays under the 614 µs bound.

The two sides are recorded at slightly different points. PLCA runs are measured
below the PLCA sublayer, where the delay the rotation imposes is visible, and
CSMA/CD runs at the MAC, which has no sublayer beneath it. On the PLCA runs
both vantage points recorded identical maxima in all 25 runs, so the comparison
is not an artifact of where the statistic is taken. The two sides also differ
in repetitions — 3 runs per load point for PLCA against 10 for CSMA/CD —
because PLCA is near-deterministic, its repetitions differing only in traffic
jitter, while CSMA/CD's random backoff needs more seeds to expose its tail.

The chart shows the worst observed access latency for each protocol at each
offered load, at N=8 with 64-byte frames:

.. figure:: media/access-latency-vs-load.png
   :align: center

..
   FIGURE RECIPE (redo via the "inet-showcase-charts" skill)
   type:    chart (matplotlib)
   anf:     TenBaseT1S.anf   chart "Access latency vs offered load"
   inputs:  results/PlcaLatency-*.sca, results/CsmaCdLatency-*.sca (histogram summary max
            fields; no vectors needed)
   shows:   worst observed access latency per run vs total offered load, N=8:
            PLCA (plca vantage, worst of 3 runs per load) bounded under the dashed 614 us
            cycle bound; CSMA/CD (mac vantage, per-run maxima, 10 seeds/load) growing to
            ~0.2-0.3 s, with measured retry-limit drop annotations at 70% and 85%.
   anchors: PLCA maxima 0.23-0.49 ms, all under the bound, monotone in load;
            CSMA/CD maxima 1.3-4.2 ms @30%, 9-43 ms @50%, 196-284 ms @70%, 198-266 ms @85%
            (all-station per-run maxima - the chart pools controller + edge MACs);
            drops 3-10/seed @70%, 20-33/seed @85% (recomputed from scalars at render time).
            If any of these move materially, the scenario or recording changed - re-derive.
   export:  opp_charttool imageexport TenBaseT1S.anf -n "Access latency vs offered load"
            -f png --dpi 150 -d doc/media   (8x6 in -> 1200x900 px)
   stamp:   captured 2026-08-14, INET topic/gy/tenbaset1s-showcase @ 5e0d2fce22 + showcase
            dir, OMNeT++ 6.4.0aipre2 (omnetpp-dev)


The vertical axis is logarithmic, so each gridline is a factor of ten, and the
dashed line is the analytical 614 µs cycle bound derived above. A PLCA frame
waits at most one fully busy cycle, and the measured maximum grows toward that
line with load without ever crossing it: 228–296 µs at 30% load, and 484–488 µs
at 85%, which is 79% of the bound.

CSMA/CD tells the opposite story. Its maxima are already 1.3–4.2 ms at 30% load
and 9–43 ms at 50%; past its stability limit they diverge, reaching roughly
180 ms and beyond within the 2 s run, and stations begin discarding frames
after exhausting their 16 transmission attempts. The annotations at 70% and 85%
give the measured drop counts, 3–10 and 20–33 frames per 2 s run. Those frames
are absent from the curves, because a frame that never transmits has no access
latency to record — which means the CSMA/CD curves are censored, and the worst
frames are precisely the missing ones. The plotted maxima also keep growing
with observation time, because what is being measured past the stability limit
is the delay of a diverging queue, and that has no worst case at all. PLCA is
stable at every load on the grid.

The two latency configurations set the load grid, the jittered traffic, and the
seeded repetitions:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config PlcaLatency]
   :end-before: [Config Scale40Plca]

One statistic in these runs is easy to misread: the MAC still reports
collisions on the PLCA runs. Those are local signals that pace the MAC from
below — nothing collides on the wire.

End-to-end delay is stable through 50% load on both sides, and both plateau at
a payload throughput of a little over half the wire at 64-byte frames. That
ceiling is framing overhead common to both protocols, not collision loss, and
it differs slightly between them. A 46-byte payload is 368 bits. Under plain
CSMA/CD a 64-byte frame occupies 576 bit times on the wire plus its 96-bit
interpacket gap, capping payload at 368/672 = 54.8%; under PLCA the busy
opportunity is 680 bit times, capping it at 368/680 = 54.1%. The measured
figures sit just below each cap: 53.0–53.3% for CSMA/CD, 53.9% for PLCA.

The divergence above 50% has a different cause. Under contention, CSMA/CD's
delivered share of wire occupancy tops out at about 68% (measured), and the
loss does not spread evenly — it concentrates on unlucky stations through the
capture effect. Its queues diverge at 70% and 85% load, while PLCA's stay
shallow at every load. When a station does give up, it does so on the
standard's own schedule: after 16 attempts, having accumulated up to about
366 ms of backoff at worst. The tail is long, but it ends in a drop rather than
in delivery.

Scaling with node count
~~~~~~~~~~~~~~~~~~~~~~~

Because the bound grows linearly with the node count, sizing a segment is
arithmetic. The chart puts the measured worst case at three node counts against
that bound:

.. figure:: media/latency-vs-node-count.png
   :align: center

..
   FIGURE RECIPE (redo via the "inet-showcase-charts" skill)
   type:    chart (matplotlib)
   anf:     TenBaseT1S.anf   chart "Worst-case latency vs node count"
   inputs:  results/PlcaLatency-*.sca (N=8,16 at load=85), results/Scale40Plca-*.sca
            (N=40 at 85% load; histogram summary max fields)
   shows:   measured worst-case access latency (plca vantage, worst over ALL stations -
            controller included - and runs) at N = 8/16/40 under the affine analytical
            cycle bound; the N=40 config runs its downlink halved, total offered load
            ~76.5% (not the 85% grid value)
            bound(N) = 68*N + 70 us (beacon 20 BT + (N+1) x 680 BT busy TOs).
   anchors: measured 0.488 / 0.837 / 1.232 ms at N = 8/16/40 (all-station maxima; the
            N=40 all-station max is the controller's 1232.0 us, edge-only 1215.9) -
            monotone, under the bound at every N (79% / 72% / 44% of it). If a point
            crosses the bound or the monotonicity breaks, the model or scenario
            changed - re-derive.
   export:  opp_charttool imageexport TenBaseT1S.anf -n "Worst-case latency vs node count"
            -f png --dpi 150 -d doc/media   (8x6 in -> 1200x900 px)
   stamp:   captured 2026-08-14, INET topic/gy/tenbaset1s-showcase @ 5e0d2fce22 + showcase
            dir, OMNeT++ 6.4.0aipre2 (omnetpp-dev)


Measured maxima at 85% load are 488 µs at N=8, 837 µs at N=16, and 1232 µs at
N=40, taken over all stations including the controller, against bounds of
614 µs, 1.158 ms, and 2.79 ms respectively. Forty nodes is the envelope that
ODVA — the industrial-automation standards body behind EtherNet/IP — and
shipping transceivers specify for in-cabinet use. The 40-node point sits at
44.2% of its bound: with 41 de-synchronized stations, a fully busy rotation is
statistically unreachable, which is a useful reminder that the bound is a
guarantee rather than an operating point.

Two practical notes. With burst off, the controller can serve at most one
downlink frame per cycle, so downlink-heavy zones need to be sized with that in
mind. The 40-node configuration therefore runs its downlink halved, at one
frame per 32 ms per node, which puts its total offered load at about 76.5%
rather than the grid's 85%; the bound comparison is unaffected, because the
bound does not depend on load. Separately, node counts beyond about 8 are an
electrical qualification exercise on real cabling — node spacing, stub
capacitance — and the simulation models the protocol, not the analog limits.

Delivery at saturation
~~~~~~~~~~~~~~~~~~~~~~

What happens when every station always has a frame to send? Both protocols fill
the wire, in configurations ``PlcaUtilization`` and ``CsmaCdUtilization``: PLCA
delivers 9.744 Mbps of application goodput at 1518-byte frames, and CSMA/CD
delivers 9.56–9.59 Mbps, about 98% of PLCA's. With maximum-size frames the
per-frame overhead is small, which is why nearly all of the 10 Mbps arrives as
payload, in contrast to the roughly 54% payload ceiling at 64 bytes. The
aggregate is therefore not where the two differ. The difference is who gets to
send, and what gets lost:

.. figure:: media/per-station-goodput.png
   :align: center

..
   FIGURE RECIPE (redo via the "inet-showcase-charts" skill)
   type:    chart (matplotlib)
   anf:     TenBaseT1S.anf   chart "Per-station delivered goodput at saturation"
   inputs:  results/PlcaUtilization-payloadLength=1500-#0.sca,
            results/CsmaCdUtilization-payloadLength=1500-#0.sca (per-station successful
            transmissions = mac packetPendingDelay:count; drop counts alongside)
   shows:   per-station delivered payload goodput at saturation, 1500-byte payloads,
            seed #0 of each side: PLCA exactly equal shares and zero drops; CSMA/CD
            unequal shares (capture effect) plus retry-limit drops. Aggregate parity
            (CSMA/CD ~98% of PLCA) is stated in the page sentence, not the chart.
   anchors: PLCA bars all 1.08 Mbps (180-181 frames); CSMA/CD bars spread (seed #0 ratio
            2.5x, across seeds 1.6-6.7x); drop annotation measured from the same runs
            (PLCA 0, CSMA/CD ~80 per 2 s). If shares equalize on the CSMA/CD side or PLCA
            spreads, the scenario changed - re-derive.
   export:  opp_charttool imageexport TenBaseT1S.anf
            -n "Per-station delivered goodput at saturation" -f png --dpi 150 -d doc/media
   stamp:   captured 2026-08-14, INET topic/gy/tenbaset1s-showcase @ 5e0d2fce22 + showcase
            dir, OMNeT++ 6.4.0aipre2 (omnetpp-dev)


The chart shows seed #0. PLCA's stations differ by at most one frame — 180
against 181, a ratio of 1.0056 — because the 2 s window cuts mid-cycle; the
mechanism itself is exactly fair, granting one opportunity per node per cycle,
and it dropped nothing. Across all ten CSMA/CD seeds the luckiest-to-unluckiest
ratio ranges from 1.6× to 6.7×, and 75–81 frames per 2 s run are dropped. Note
also that the controller's bar is no larger than a single node's, even though
it offers eight times the traffic: with burst off it gets one opportunity per
cycle like everyone else, which is the downlink limit described under *Scaling
with node count* above.

The fairness PLCA provides is per transmit opportunity rather than per byte.
Stations receive exactly equal frame counts, which is equal wire time only when
they send equal-size frames — a station sending 1518-byte frames takes about
21× the wire time of a 64-byte neighbor on the same segment.

The saturation runs use maximum-size (1518-byte) frames, because the analytical
utilization anchor is computed for them; a 64-byte variant shows the overhead
tax instead. PLCA's measured bus utilization matches the cycle arithmetic
exactly: 99.2% at 1518-byte frames, and 85.6% at 64-byte frames. The
utilization statistic counts DATA-signal presence only, with preamble and
delimiter included and beacons, COMMIT signals and interpacket gaps excluded —
it is DATA time divided by total time.

Limitations
-----------

The model covers the PLCA protocol rather than a complete 10BASE-T1S product,
and leaves the following out:

- PLCA and plain-CSMA/CD stations cannot share one mixing segment in the model;
  mixing is possible only through a switch port.
- Loss of node 0 and the resynchronization that follows are not modeled;
  beacons always arrive.
- There is no noise and there are no reception errors: frames are lost only to
  collisions (CSMA/CD) or queue overflow.
- Real constrained nodes often reach the PHY over a serial host interface that
  can cap their rate below wire speed; hosts here send at wire speed.
- Burst mode is not used here, and multiple node IDs per station are not
  modeled. These are the product knobs for favoring one node; every
  configuration in this showcase grants one opportunity per node per cycle.

Sources and Further Reading
---------------------------

- IEEE Std 802.3cg-2019 — 10BASE-T1S and the PLCA reconciliation sublayer.
- K. Kim, E. Choi, J.-W. Choi, "Network Performance Evaluation of IEEE
  802.3cg", J-KICS, vol. 45, no. 4, pp. 706–711, 2020,
  DOI 10.7840/kics.2020.45.4.706 — related simulation study of the same
  standard.
- D. Boggs, J. Mogul, C. Kent, "Measured Capacity of an Ethernet: Myths and
  Reality", Proc. ACM SIGCOMM '88, pp. 222–234, DOI 10.1145/52324.52347 —
  the classic measurement study showing that saturated 10 Mbps Ethernet keeps
  its aggregate throughput; our CSMA/CD goodput matches its conclusion.
- Y.-T. Wu, J. Palathanam (Ford), 10BASE-T1S node count analysis, IEEE
  Ethernet Tech Day 2024 — `slides
  <https://standards.ieee.org/wp-content/uploads/2024/11/201-yu-ting-wu-jibu-palathanam-nodecount-analysis.pdf>`__
  — the bench-tested load regime and node-count guidance cited above.
- OPEN Alliance, "10BASE-T1S PLCA Management Registers", v1.2, 2022 — the
  source of the "PLCA coordinator" term and of the register-level defaults.
- ODVA, `EtherNet/IP In-Cabinet profile
  <https://www.odva.org/technology-standards/market-drivers/ethernetip-in-cabinet/>`__
  — the 40-node, 25 m in-cabinet deployment envelope.

Sources: :download:`omnetpp.ini <../omnetpp.ini>`,
:download:`MultidropNetwork.ned <../MultidropNetwork.ned>`

Try It Yourself
---------------

If you already have INET and OMNeT++ installed, start the IDE by typing
``omnetpp``, import the INET project, then navigate to the
``inet/showcases/ethernet/tenbaset1s`` folder in the `Project Explorer`. There,
you can view and edit the showcase files, run simulations, and analyze results.

The simulation offers six configurations: ``PlcaLatency`` and
``CsmaCdLatency`` for the headline latency comparison, ``CycleAnatomy`` to
watch a single cycle up close, ``Scale40Plca`` for the 40-node point, and
``PlcaUtilization`` and ``CsmaCdUtilization`` for the saturation study.

Otherwise, there is an easy way to install INET and OMNeT++ using `opp_env
<https://omnetpp.org/opp_env>`__, and run the simulation interactively:

.. code-block:: bash

    $ opp_env run inet-4.7 --init -w inet-workspace --install --build-modes=release --chdir \
       -c 'cd inet-4.7.*/showcases/ethernet/tenbaset1s && inet'

This command creates an ``inet-workspace`` directory, installs the appropriate
versions of INET and OMNeT++ within it, and launches the ``inet`` command in the
showcase directory for interactive simulation.

Alternatively, for a more hands-on experience, you can first set up the
workspace and then open an interactive shell:

.. code-block:: bash

    $ opp_env install --init -w inet-workspace --build-modes=release inet-4.7
    $ cd inet-workspace
    $ opp_env shell

Inside the shell, start the IDE by typing ``omnetpp``, import the INET project,
then start exploring.

Discussion
----------

Use `this page <https://github.com/inet-framework/inet/discussions>`__ in the
GitHub issue tracker for commenting on this showcase.

.. TODO: create the dedicated discussion thread and replace the generic link.

10BASE-T1S Multidrop Ethernet with PLCA
=======================================

Goals
-----

Automotive and industrial networks replace CAN and similar fieldbuses with
10BASE-T1S, a 10 Mbps Ethernet that connects up to a few dozen nodes to one
unshielded twisted pair. On a shared wire, classic Ethernet arbitration by
collision gives random, unbounded waiting times — unacceptable for control
traffic. Physical Layer Collision Avoidance (PLCA) removes the collisions and
makes the worst case predictable.

This showcase measures that difference on one segment: PLCA keeps worst-case
medium access latency under an analytical bound, while the same hardware
running plain CSMA/CD (Carrier Sense Multiple Access with Collision Detection)
shows a collision-driven latency tail orders of magnitude longer that ends in
frame drops — delivery is no longer guaranteed.

| Verified with INET version: ``4.7``
| Source files location: `inet/showcases/ethernet/tenbaset1s <https://github.com/inet-framework/inet/tree/master/showcases/ethernet/tenbaset1s>`__

.. TODO: confirm the release version number that contains commits e342d04b0e,
   1bc2af2784, 5e0d2fce22 before publishing; also applies to the opp_env
   version below.

.. admonition:: In one minute

   - Physical Layer Collision Avoidance (PLCA) rotates a single transmit
     opportunity through the nodes of a 10BASE-T1S multidrop segment, so
     frames never collide on the wire — and the longest possible wait is one
     full rotation.
   - Measured on 8 nodes with 64-byte frames: PLCA worst-case access latency
     is bounded — 488 µs, under the 614 µs analytical bound — at every load.
     Plain CSMA/CD is unbounded: ≥~200 ms within a 2 s run at 70–85% load,
     with 20–33 frames per 2 s dropped after 16 failed attempts.
   - At saturation both protocols fill the wire, but PLCA delivers exactly
     equal per-station shares with zero drops; CSMA/CD's luckiest station
     sends 1.6–6.7× more than its unluckiest, and 75–84 frames per 2 s are
     dropped.
   - One configuration line switches the same interface between the two modes.
   - The page: the network, the one-line switch, the PLCA cycle up close, then
     latency, scaling, and fairness results.

Access latency is how long a frame waits from being ready until it starts onto
the wire. The chart below shows its measured worst case per protocol, per
offered load:

.. figure:: media/access-latency-vs-load.png
   :align: center

**PLCA's worst-case access latency stays bounded under the ≈0.6 ms cycle bound
for 64-byte frames at N=8 at every load, while CSMA/CD's observed maxima
exceed ~200 ms within 2 s, keep growing with observation time, and frames
start to be dropped.**

The vertical axis is logarithmic — each gridline is 10×. The dashed line is
the analytical cycle bound, derived below. The annotations at 70% and 85% give
the measured drop counts: those frames were never delivered and are not in
these curves. Plotted maxima cover delivered frames within the 2 s run and
grow with observation time — the true worst case is unbounded; PLCA is stable
at every load.

About 10BASE-T1S and PLCA
-------------------------

10BASE-T1S (IEEE 802.3cg-2019) is the short-reach member of the single-pair
Ethernet family. Its multidrop mode connects at least 8 nodes to one twisted
pair of up to at least 25 m — a bus the standard calls a mixing segment. It
targets the sensor/actuator edge of vehicles and machines, where it replaces
CAN, LIN, and FlexRay with ordinary Ethernet frames. Do not confuse it with
10BASE-T1L, the long-reach, point-to-point sibling.

On a shared wire, plain CSMA/CD access is random: colliding nodes back off for
random times, so waiting times have no upper limit. PLCA avoids this. It
rotates a single transmit opportunity through the node IDs in fixed order;
node 0 — often called the PLCA coordinator (OPEN Alliance) — sends a short
beacon signal to start each cycle. A node with a frame ready transmits in its
own opportunity; a node with nothing to send yields it after a few
microseconds. The Ethernet MAC itself is unchanged: PLCA is a thin sublayer
below it that simply delays the MAC until the node's opportunity arrives.

The Network
-----------

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
             margin=10; cropped to 1132x400 (empty bottom band removed)
   stamp:    captured 2026-08-14, INET @ 5e0d2fce22 + showcase, OMNeT++ 6.4.0aipre2

**Nine nodes share one pair; the same hardware runs PLCA or plain CSMA/CD by
one configuration line.**

The network is one mixing segment: N edge nodes and one zonal controller. In a
real design the controller bridges this segment to the vehicle backbone; that
side is not modeled here. All nodes are :ned:`EthernetPlcaHost` — a host whose
applications exchange raw Ethernet frames through an interface with a PLCA
sublayer. The pair itself is a chain of :ned:`WireJunction` modules connected
by :ned:`EthernetMultidropLink` cables: 1 m of trunk between neighbors, 10 cm
stubs — spacing that real harnesses use.

The convention used throughout: N counts the edge nodes; the controller is
PLCA node 0, so a cycle has N+1 transmit opportunities. The network file
assigns the two mandatory PLCA parameters accordingly:

.. literalinclude:: ../MultidropNetwork.ned
   :language: ned
   :start-at: controller: <> like IEthernetNetworkNode {
   :end-before: j[numNodes]: WireJunction {

PLCA vs CSMA/CD in One Line
---------------------------

Every configuration inherits one of two abstract sections. The only difference
between the two protocol stacks is removing the ``plca`` submodule — the MAC,
PHY, queue, and cable are identical:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config PlcaBase]
   :end-before: [Config CycleAnatomy]

Traffic is cyclic control traffic: every edge node sends 64-byte frames to the
controller, and the controller sends frames back at a quarter of that volume.
Each production interval is jittered ±10% so the nodes drift out of phase, and
each node starts at a random offset. Offered load L counts total wire
occupancy, both directions. The per-node period at N=8:

===================== ========= ========= ======== ========
Offered load L        30%       50%       70%      85%
===================== ========= ========= ======== ========
Period per node       2.267 ms  1.360 ms  971 µs   800 µs
===================== ========= ========= ======== ========

The top loads match Ford's bench-tested aggregate regime; designed operating
points sit at or below ~50%.

The PLCA Cycle Up Close
-----------------------

Every PLCA node keeps a counter, ``curID``, that tracks which node ID owns the
current transmit opportunity. Watching it during a run shows the rotation
directly:

.. figure:: media/to-rotation.png
   :align: center

**The transmit opportunity rotates through every node ID in fixed order;
node 0's beacon starts each cycle.**

The controller and node[0] curves coincide exactly — all nodes count in
lockstep. Each staircase tops out at 9: every node steps past the last ID, and
on node 0 passing the node count triggers the next beacon, which resets all
counters — this is also why the trace starts at 9 at t=0, before the first
beacon. The inset zooms on one beacon (measured duration 2.00 µs).

.. figure:: media/to-timelines.png
   :align: center

**A node with nothing to send yields its opportunity after 3.2 µs — idle
slots cost microseconds, not frame times.**

The three lanes are a bus view: the top lane is everything the controller
receives — effectively the bus itself; the middle lane is what the controller
transmits (its beacons and frames); the bottom lane is what node[0] transmits.
A COMMIT is the short claim signal a node sends at the start of its
opportunity while its frame is still on the way. Yielded opportunities appear
as 3.2 µs silences; a fully idle cycle takes 30.8 µs (measured).

.. admonition:: Details — the cycle arithmetic, term by term

   The arithmetic comes from the standard's own cycle structure, with two
   INET accounting details: a 1 ns gap after each cycle, and the end-of-stream
   delimiter counted in the frame's wire time. That delimiter costs one
   octet-time per the standard's own line encoding — it is not a simulator
   quirk.

   - A busy transmit opportunity costs the frame on the wire (preamble +
     frame + delimiter; 584 bit times at 64 B) plus one 96-bit interframe
     gap: 680 bit times. It does not also pay the opportunity timer — the
     COMMIT stops the other nodes' timers and covers the sender's own gap.
   - An idle opportunity costs the 32-bit opportunity timer; the beacon costs
     20 bit times.
   - Worst case = one fully busy cycle. At N=8, 64 B:
     20 + 9 × 680 = 6140 bit times = **614 µs**. In general: 68·(N+1) + 2 µs.
   - At N=8 with 64 B frames, every node is guaranteed a transmit opportunity
     at least every ≈0.61 ms — a 10 ms control loop fits with >15× margin; at
     1518 B the guarantee is ≈11.1 ms, so budget the largest frame on the
     segment.
   - Counting only MAC frame bits, the fully busy cycle carries 75.0% (64 B)
     or 98.6% (1518 B) of the wire — the "useful bits" view; the bus
     utilization numbers later on this page count all signal presence
     instead.

Access Latency Under Load
-------------------------

The hero chart above compares the two protocols on the same load grid. A PLCA
frame waits at most one fully busy cycle — the dashed 614 µs line. The
measured PLCA maximum grows toward the bound with load and never exceeds it:
229–296 µs at 30% load, 484–488 µs (79% of the bound) at 85%. The bound
scales with the largest frame any node may send — ≈11.1 ms at 1518 B.

CSMA/CD tells the opposite story. At 30% load its maxima are already 1.3–3.7
ms; at 70–85% they are unbounded — ≥~200 ms within the 2 s runs, growing with
observation time — and stations drop 3–10 (70%) and 20–33 (85%) frames per
2 s after exhausting their 16 transmission attempts. The two latency
configurations differ from the base sections only in load and repetitions:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config PlcaLatency]
   :end-before: [Config Scale40Plca]

.. admonition:: Fine print — what the latency statistic measures

   - The plotted statistic starts when the frame is ready for transmission at
     the interface and ends when its transmission starts; time spent queued
     behind earlier frames is not included.
   - PLCA runs are measured below the PLCA sublayer, CSMA/CD runs at the MAC.
     On PLCA runs the two vantage points recorded identical maxima in all 25
     runs, so the comparison is not an artifact of where we measure.
   - The CSMA/CD curves are censored: dropped frames never transmit, so the
     worst frames are missing from the delivered-frame statistics.
   - The MAC still reports collisions on PLCA runs — these are local signals
     that pace the MAC; nothing collides on the wire.
   - End-to-end delay is stable through 50% load on both sides. This is a
     finding, not a setting: with 64-byte frames, CSMA/CD's usable capacity
     on this segment tops out around 53–55%, so its queues diverge at 70% and
     85% while PLCA's stay shallow at every load.
   - The standard's own backoff schedule ends after 16 attempts, ≈366 ms of
     cumulative backoff at worst — the tail is long, but ends in a drop, not
     in delivery.
   - PLCA's fairness is per transmit opportunity: stations get exactly equal
     frame counts only when they send equal-size frames — a station sending
     1518-byte frames gets ~24× the airtime of a 64-byte neighbor.

Scaling with Node Count
-----------------------

.. figure:: media/latency-vs-node-count.png
   :align: center

**The measured worst case stays under the linearly growing bound from 8
through 40 nodes — sizing a segment is arithmetic.**

Measured maxima at 85% load: 488 µs (N=8), 837 µs (N=16), 1216 µs (N=40),
against bounds of 614 µs, 1.158 ms, and 2.79 ms. The 40-node point sits at
44% of its bound: with 41 de-synchronized stations, a fully busy rotation is
statistically unreachable — the bound is a guarantee, not the operating
point. Note that node counts beyond ~8 are an electrical qualification
exercise on real cabling (node spacing, stub capacitance); the simulation
models the protocol, not the analog limits.

Delivery at Saturation
----------------------

What happens when every station always has a frame to send? Both protocols
fill the wire: PLCA delivers 9.744 Mbps of application goodput at 1518-byte
frames and CSMA/CD delivers 9.56–9.59 Mbps — about 98% of PLCA's. The
difference is who gets to send, and what gets lost. The chart shows delivered
goodput per station:

.. figure:: media/per-station-goodput.png
   :align: center

**At saturation both protocols fill the wire — what PLCA buys is not
throughput but guaranteed, fair delivery: zero drops and an exactly equal
per-station share, where CSMA/CD dropped 75–84 frames per 2 s and its
luckiest station transmitted 1.6–6.7× more than its unluckiest.**

The chart shows seed #0; across all ten CSMA/CD seeds the luckiest-to-
unluckiest ratio ranges from 1.6× to 6.7×, while PLCA's is 1.00 in every run.
PLCA's bus utilization matches the cycle arithmetic exactly: 99.2% of the
wire carries signal at 1518-byte frames, 85.6% at 64-byte frames — small
frames pay the fixed per-opportunity cost.

Limitations
-----------

.. admonition:: Fine print — what the model leaves out

   - Loss of node 0 and the resynchronization that follows are not modeled;
     beacons always arrive.
   - PLCA and plain-CSMA/CD stations cannot share one mixing segment in the
     model; mixing is possible only through a switch port.
   - No noise or reception errors: frames are lost only to collisions
     (CSMA/CD) or queue overflow.
   - Real constrained nodes often reach the PHY over a serial host interface
     that can cap their rate below wire speed; hosts here send at wire speed.
   - Burst mode and multiple node IDs per station — the product knobs for
     favoring one node — are not exercised; every configuration runs one
     opportunity per node per cycle.

Sources and Further Reading
---------------------------

- IEEE Std 802.3cg-2019 — 10BASE-T1S and the PLCA reconciliation sublayer.
- K. Kim, E. Choi, J.-W. Choi, "Network Performance Evaluation of IEEE
  802.3cg", J-KICS, vol. 45, no. 4, 2020, DOI 10.7840/kics.2020.45.4.706 —
  related simulation study of the same standard.
- D. Boggs, J. Mogul, C. Kent (1988) — the classic measurement study showing
  that saturated 10 Mbps Ethernet keeps its aggregate throughput; our
  CSMA/CD goodput matches its conclusion.

.. TODO: domain expert verifies/completes the citation forms at G5.

Sources: :download:`omnetpp.ini <../omnetpp.ini>`,
:download:`MultidropNetwork.ned <../MultidropNetwork.ned>`

Try It Yourself
---------------

If you already have INET and OMNeT++ installed, start the IDE by typing
``omnetpp``, import the INET project, then navigate to the
``inet/showcases/ethernet/tenbaset1s`` folder in the `Project Explorer`. There,
you can view and edit the showcase files, run simulations, and analyze results.

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

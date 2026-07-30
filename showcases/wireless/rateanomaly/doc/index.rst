IEEE 802.11 Rate Anomaly
========================

Goals
-----

A Wi-Fi cell is a shared medium: at any instant only one station can be
transmitting. When several stations have traffic to send, the 802.11 Distributed
Coordination Function (DCF) shares the channel between them *fairly* — but "fairly"
turns out to mean something surprising. This showcase demonstrates the **802.11 rate
anomaly**: a single station transmitting at a low bitrate does not just get less for
itself, it drags the throughput of *every* other station down to its own level, and
the whole network loses a large fraction of its capacity.

We reproduce the effect with a small 802.11g network and measure just how much one
slow station costs everyone else. The page then shows the two standard cures for the two
forms of the anomaly: the 802.11e TXOP on the contention-based uplink, and an
airtime-fair transmit queue at the access point on the downlink.

| Verified with INET version: ``4.6``
| Source files location: `inet/showcases/wireless/rateanomaly <https://github.com/inet-framework/inet/tree/master/showcases/wireless/rateanomaly>`__

About 802.11 Channel Access and the Rate Anomaly
------------------------------------------------

802.11 stations share a single half-duplex radio channel — only one of them can be on
the air at a time. They coordinate access with the Distributed Coordination Function
(DCF), a carrier-sense multiple access scheme with collision avoidance (CSMA/CA): a
station listens before transmitting, and when the channel is busy it waits a random
*backoff* — a random number of idle slots — before trying again.
Averaged over time, this random backoff gives every contending station a statistically
**equal number of opportunities to transmit**. That is the fairness DCF provides.

802.11 is also a multi-rate technology. A station with a weaker or noisier link falls
back to a lower bitrate so that its frames remain decodable. The catch is that a frame
sent at a low bitrate occupies the channel *longer*. The data bits take proportionally
longer to clock out — about nine times longer at 6 Mbps than at 54 Mbps, since the rate
is nine times lower. The whole on-air frame grows by a slightly smaller factor, about
eight times, because the physical-layer preamble and header take a fixed time that does
not shrink with the data rate. And the full channel cost of a transmission — the frame
plus the fixed interframe spaces and the acknowledgment (which for the slow station is
itself sent at a low rate) — comes to a smaller factor still, roughly six times for
6 versus 54 Mbps. However it is counted, a slow frame ties up the shared medium several
times longer than a fast one.

Now combine the two facts. DCF equalizes the *number* of transmissions, not the *time*
each station spends transmitting. A slow station transmits about as often as everyone
else, but each of its transmissions ties up the channel far longer — so it consumes a
disproportionate share of channel time. Because the medium is shared, the time the slow
station occupies is time the fast stations cannot use. The fast stations are therefore
throttled down toward the slow station's throughput. In the limit, *all* stations end
up with roughly the **same** throughput, close to what the slowest station would
achieve on its own. This is the IEEE 802.11 performance anomaly, first characterized by
`Heusse, Rousseau, Berger-Sabbatel, and Duda
<https://ieeexplore.ieee.org/document/1208921/>`__ (INFOCOM 2003).

The root cause is that standard DCF provides **transmission-opportunity fairness**
(equal access count) rather than **airtime fairness** (equal channel time). Scheduling
disciplines that enforce airtime fairness avoid the anomaly, but they are outside plain
DCF.

These airtime-fair disciplines take two forms, because the anomaly itself does — and this
showcase reproduces both. One form is *uplink*: independent stations contending, each running
its own DCF. Fixing it means every *contending* station must bound its own share, which is
what the 802.11e standard adds with the transmission opportunity (TXOP). The mirror-image
*downlink* form appears when one access point serves many clients of mixed rates. There is no
contention to arbitrate — a single transmitter divides *its own* airtime among its
destinations, a scheduling problem rather than a contention one.

The downlink form is the one production access points face. The Linux ``mac80211`` stack
schedules its own frames with a deficit round-robin whose deficit is counted in airtime
(microseconds) instead of bytes (Høiland-Jørgensen et al., `"Ending the Anomaly"
<https://arxiv.org/abs/1703.00064>`__, USENIX ATC 2017), and many commercial access points
expose an equivalent "airtime fairness" knob. Such a transmit-side scheduler only governs the
frames a node sends itself, so it cures the downlink case but does nothing for distributed
uplink contention — the two forms need separate fixes, and both are shown below.

The Model
---------

802.11 Configuration in INET
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The network uses three kinds of node:

- :ned:`WirelessHost` — the contending stations, each with an 802.11 interface;
- :ned:`AccessPoint` — the access point the stations associate with; it bridges the
  wireless cell to a wired Ethernet segment;
- :ned:`StandardHost` — a wired server that receives the stations' traffic.

Each station's data rate is pinned with the wlan interface's ``bitrate`` parameter, and
``opMode`` selects 802.11g. No rate-control algorithm is active, so every station
transmits at exactly its configured rate regardless of conditions. This is what lets us
make one station slow and the rest fast purely by configuration:

.. literalinclude:: ../omnetpp.ini
   :start-at: *.sta[*].wlan[*].opMode
   :end-at: pendingQueue.packetCapacity
   :language: ini

The relevant settings:

- ``opMode = "g(erp)"`` — every interface runs 802.11g.
- ``bitrate`` — the fixed data rate. Here it is 54 Mbps everywhere; the configurations
  below override it for one station to open the rate gap.
- The transmit ``power`` is generous and all stations sit close to the access point, so
  every link is error-free at every rate. The slow station is slow *by configuration*,
  not because of a weak signal — this isolates the anomaly from packet loss.
- A MAC queue of ten frames (``pendingQueue.packetCapacity``) together with an
  over-provisioned UDP source keeps every station continuously backlogged, so the channel
  is fully contended. This is the saturated regime in which the rate anomaly is defined.
  The exact depth does not matter as long as the queue never drains — ten frames is simply
  enough to keep one always ready to send while the excess is dropped at the queue rather
  than buffered into a growing delay.

Every station sends a saturating UDP stream to the server on its own port, so
throughput can be measured separately for each station:

.. literalinclude:: ../omnetpp.ini
   :start-at: [Config UplinkBase]
   :end-at: *.server.app[*].localPort
   :language: ini

These traffic settings, together with the radio and node placement in ``[General]``, make up
an abstract ``[Config UplinkBase]`` that every uplink configuration extends; the downlink
section later builds a parallel ``[Config DownlinkBase]``.

The Network
~~~~~~~~~~~

The network contains an access point with five wireless stations clustered nearby, and
a wired server reachable through the access point. All five stations upload a saturating
UDP flow to the server at the same time, so they continuously contend for the channel.
``sta[0]`` is the station the later configurations slow down; the other four always stay
at 54 Mbps.

.. figure:: media/network.png
..
   FIGURE RECIPE (redo via the "omnetpp-mcp-sim" skill)
   type:     canvas
   config:   UplinkHomogeneous   # ../omnetpp.ini (all three Uplink* configs share these
             positions; the Downlink* ones spread the stations vertically instead)
   seed:     default
   shows:    topology -- the configurator/radioMedium/visualizer infrastructure modules,
             five sta[*] wireless hosts clustered ~8-9 m from the accessPoint, and the
             wired server reachable over Eth100M
   anchor:   initial state (t=0, before run). Structural -- no timing; if the module set
             or links differ, the NED changed.
   capture:  Qtenv + MCP server; set_canvas_view {module_path:"<root>", zoom:30} then
             get_canvas_image {module_path:"<root>", area:"all_elements", margin:8}.
             zoom 30 (not fit:true) keeps the 2 m station spacing wide enough for the
             labels not to collide; all_elements still leaves the unused lower half of
             the playground, so the result is cropped to its content bounding box with
             an 18 px margin (background #d1d1d1, threshold 18, ignoring the green
             playground frame). Was 858x688 with a third of it empty; now 798x540.
   stamp:    captured 2026-07, INET 4.6

The Configurations
~~~~~~~~~~~~~~~~~~

Six runnable configurations are defined, in two families of three. The uplink family has the
stations contending; the downlink family has the access point serving them. Within each
family there is a no-rate-gap baseline, the anomaly, and the fix:

===================  ==========  ==========================================================
configuration        runs        what it shows
===================  ==========  ==========================================================
UplinkHomogeneous    1           baseline — all five stations at 54 Mbps
UplinkAnomaly        6 (sweep)   the anomaly — ``sta[0]`` slowed, 36 Mbps down to 6 Mbps
UplinkTxop           6 (sweep)   the fix — the same rate gap under 802.11e TXOP
DownlinkHomogeneous  1           baseline — the AP serves all five clients at 54 Mbps
DownlinkAnomaly      1           the anomaly — frame-fair AP queue, ``sta[0]`` at 6 Mbps
DownlinkAirtimeFair  1           the fix — airtime-fair AP queue, same rate gap
===================  ==========  ==========================================================

Each is run from the showcase directory with, for example:

.. code-block:: bash

    $ inet -u Cmdenv -c UplinkAnomaly          # all six sweep points
    $ inet -u Qtenv  -c DownlinkAirtimeFair    # watch one run in the GUI

The two sweep configurations iterate ``sta[0]``'s bitrate, so they contain six runs each;
add ``-r <n>`` to pick one. The sections below work through the uplink family first and the
downlink family after it, each time establishing the anomaly before introducing the fix.

The UplinkHomogeneous and UplinkAnomaly Configurations
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The first two uplink configurations set up the contrast. In **UplinkHomogeneous**, all five
stations transmit at 54 Mbps — the baseline, in which the channel is shared fairly and the
network runs at full 802.11g capacity. In **UplinkAnomaly**, one station is slowed while the
other four stay at 54 Mbps; its bitrate is swept from 36 Mbps down to 6 Mbps to show how the
damage grows as the rate gap widens:

.. literalinclude:: ../omnetpp.ini
   :start-at: [Config UplinkHomogeneous]
   :end-at: slowBitrate
   :language: ini

Results
-------

Each station's application-level throughput is measured at the server over the
steady-state interval, after association settles. Each run lasts 5 s, with the first
1 s discarded as warmup, so throughput is averaged over the remaining 4 s.

DCF is not deterministic — the random backoff draws from the RNG — so results vary from
seed to seed. The charts in this section come from a single run each, which is enough
because the effect is far larger than the spread: individual stations differ by a few
percent, the anomaly halves the whole cell. The two later charts that compare a *fix*
against the anomaly average three repetitions per point, since there the run-to-run
spread is comparable to some of the differences being plotted.

In the **UplinkHomogeneous** baseline, all five stations achieve nearly the same throughput,
about 4.6–5.0 Mbps each, for an aggregate of roughly 24 Mbps — full 802.11g saturation
throughput at this payload size.

When one station is slowed to 6 Mbps in **UplinkAnomaly**, every station — including the
four still configured for 54 Mbps — drops to about 2.3–2.6 Mbps. The fast stations do
not merely lose a little throughput; they are pulled down to nearly the slow station's
level, settling just above the floor it sets:

.. figure:: media/per-station-throughput.png
..
   FIGURE RECIPE (redo via the "inet-showcase-charts" skill)
   type:     chart (matplotlib)
   anf:      RateAnomalyShowcase.anf   chart "Per-station throughput"
   inputs:   results/*.sca   from configs UplinkHomogeneous + UplinkAnomaly (already recorded)
   shows:    per-station application throughput, UplinkHomogeneous (all 54 Mbps) vs the
             rate anomaly (sta[0] at 6 Mbps); the four fast stations collapse to the
             slow station's level (dotted line)
   anchor:   data is structural — server.app[*] packetReceived:count x 0.002 -> Mbps.
             If the per-station series set changes, the scenario/recording changed -> re-derive.
   backend:  matplotlib -> identical in IDE and headless
   export:   opp_charttool imageexport RateAnomalyShowcase.anf -n "Per-station throughput"
             -f png --dpi 150 -d doc/media/   ; size 1200x900, 8x6 in via image_export_width/height
   stamp:    captured 2026-06, INET 4.6

The reason is visible in the raw frame counts: over the measurement interval every
station — fast or slow — successfully transmits a similar *number* of frames (between
roughly 1,150 and 1,300). DCF gave each station a nearly equal number of transmission
opportunities, exactly as designed. But each of the slow station's frame exchanges tied
up the channel several times longer — around six times, once the rate-independent
preamble, interframe spaces, and acknowledgment are included — so it consumed
most of the channel time and left little for the others.

.. figure:: media/frames-per-station.png
..
   FIGURE RECIPE (redo via the "inet-showcase-charts" skill)
   type:     chart (matplotlib)
   anf:      RateAnomalyShowcase.anf   chart "Frames per station"
   inputs:   results/*.sca   from config UplinkAnomaly slowBitrate=6 (already recorded)
   shows:    frames successfully transmitted per station in the rate-anomaly case
             (slow = 6 Mbps); near-equal counts = DCF's equal transmission opportunities
   anchor:   data is structural — server.app[*] packetReceived:count for the UplinkAnomaly
             slowBitrate=6 run. If that run is absent or counts diverge, re-derive.
   backend:  matplotlib -> identical in IDE and headless
   export:   opp_charttool imageexport RateAnomalyShowcase.anf -n "Frames per station"
             -f png --dpi 150 -d doc/media/   ; size 1200x900, 8x6 in via image_export_width/height
   stamp:    captured 2026-06, INET 4.6

Where the frame counts show that each station gets an equal *number* of turns, a
sequence chart shows how unequal those turns are in *duration*. The chart below captures
about 2.6 ms of the shared channel in a reduced two-station illustration — one station
fixed at 6 Mbps, the other at 54 Mbps, each sending a light, collision-free stream (the
other three stations idle) so every frame stands alone. Each frame is drawn as a block
whose width is the time it holds the medium:

.. figure:: media/frame-airtime-sequence.png
..
   FIGURE RECIPE (redo via the "omnetpp-ide-mcp" skill)
   type:     seqchart
   config:   UplinkAnomaly run 5 (slowBitrate=6), reduced to two active stations (apps on
             sta[2..4] disabled) at LIGHT load so frames don't collide: sta[0]=6 Mbps
             sending every 3 ms, sta[1]=54 Mbps sending every 0.8 ms
   seed:     default
   shows:    four narrow 54 Mbps frames (sta[1], seq 2500-2503) then one wide 6 Mbps frame
             (sta[0], seq 667) carrying the same 1000-byte payload -- block width = airtime,
             so the slow frame occupies the channel ~8x longer (measured 7.8x on-air)
   record:   inet -u Cmdenv -c UplinkAnomaly -r 5
               --*.sta[2].numApps=0 --*.sta[3].numApps=0 --*.sta[4].numApps=0
               --*.sta[0].app[0].sendInterval=3ms --*.sta[1].app[0].sendInterval=0.8ms
               --record-eventlog=true --eventlog-recording-intervals=2s..2.05s
               --sim-time-limit=2.06s --result-dir=results/elog3
   source:   results/elog3/UplinkAnomaly-slowBitrate=6-#0.elog
   axes:     sta[0] (6 Mbps), sta[1] (54 Mbps), accessPoint   (this top-to-bottom order)
   display:  NETWORK_COMMUNICATION; timeline SIMULATION_TIME (linear -- required so block
             width equals airtime; NONLINEAR flattens the contrast)
   anchor:   window 2.004000s..2.006700s -- the collision-free 6 Mbps frame occupies
             [2.005044s, 2.006494s]. Any clean (non-colliding) slow frame works; if the
             wide/narrow width ratio stops being ~8x, the bitrates changed.
   capture:  Sequence Chart screenshot, top band (ruler + position/range readout) cropped
             off so the bottom ruler carries the time axis; was 977x328
   stamp:    captured 2026-07, INET 4.6

Each narrow block is a 54 Mbps frame; the wide block is a single 6 Mbps frame carrying
the same 1000 bytes, so its on-air time is about eight times longer — the data alone
would take nine times as long at the lower rate, but the fixed preamble keeps the whole
frame just under that. The slow frame alone holds the channel about as long as the whole
run of fast frames beside it. That per-frame airtime gap is the mechanism behind the
anomaly: because DCF hands the stations a similar *number* of transmissions (the
frame-count chart above), the slow station's far longer frames let it swallow a
correspondingly larger share of channel time — dragging every station's throughput down
toward its own.

The damage scales with the rate gap. As the slow station's rate falls from 54 to
6 Mbps, the aggregate network throughput falls from about 24 to 12 Mbps — a single slow
station halves the capacity of the entire cell:

.. figure:: media/throughput-vs-rate.png
..
   FIGURE RECIPE (redo via the "inet-showcase-charts" skill)
   type:     chart (matplotlib)
   anf:      RateAnomalyShowcase.anf   chart "Throughput vs slow-station rate"
   inputs:   results/*.sca   from configs UplinkHomogeneous + UplinkAnomaly (already recorded)
   shows:    aggregate, fast-station-average, and slow-station throughput vs the slow
             station's bitrate (54 = UplinkHomogeneous baseline, 36..6 = UplinkAnomaly sweep)
   anchor:   data is structural — server.app[*] packetReceived:count grouped by the
             slowBitrate itervar (UplinkHomogeneous -> 54). If the sweep points change, re-derive.
   backend:  matplotlib -> identical in IDE and headless
   export:   opp_charttool imageexport RateAnomalyShowcase.anf -n "Throughput vs slow-station rate"
             -f png --dpi 150 -d doc/media/   ; size 1200x900, 8x6 in via image_export_width/height
   stamp:    captured 2026-06, INET 4.6

============================  ===============  ==============  ====================
slow station rate (Mbps)      aggregate        slow station    fast stations (avg)
============================  ===============  ==============  ====================
54 (UplinkHomogeneous)        24.2             5.02            4.80
36                            23.1             4.24            4.70
24                            21.2             3.83            4.34
18                            19.9             3.40            4.12
12                            17.8             2.81            3.74
9                             15.2             2.74            3.11
6                             12.2             2.31            2.47
============================  ===============  ==============  ====================

(All values in Mbps, application-level throughput. The 54 Mbps row is the UplinkHomogeneous
baseline — the zero-gap reference point; the rows below it are the UplinkAnomaly sweep.)
The fast stations' own throughput — the rightmost column — falls almost in step with the
slow station's, even though their configuration never changes. That drop is the rate
anomaly: equal access, unequal airtime.

Solving the Anomaly with Airtime Fairness
-----------------------------------------

The anomaly follows from DCF sharing *transmission opportunities* equally. The IEEE 802.11e
amendment adds a mechanism that shares *airtime* instead: the **transmission opportunity
(TXOP)**. 802.11e replaces DCF with EDCA, which sorts traffic into four access categories;
ordinary best-effort traffic uses AC_BE. A station that wins the channel for a category may
then transmit a burst of frames for up to a bounded *time* — that category's ``txopLimit`` —
before it has to contend again. DCF still hands out wins equally often, so an equal *time* per
win means an equal share of airtime: a fast station simply packs many frames into its slice, a
slow station only a few.

The ``[Config UplinkTxop]`` configuration switches both the stations and the access point to
the QoS (EDCA) MAC and grants best-effort traffic a time-based TXOP. AC_BE's *default* TXOP
limit is zero — one frame per win — so the fix is simply to set a nonzero limit; here we borrow
the standard's Video-category value (3.008 ms). (Best-effort traffic maps to AC_BE, which
INET indexes as ``edcaf[1]``.) The MAC queue is deepened too, so a fast
station has enough frames buffered to fill a burst; those are the only changes:

.. literalinclude:: ../omnetpp.ini
   :start-at: [Config UplinkTxop]
   :end-at: pendingQueue.packetCapacity
   :language: ini

At a zero TXOP limit EDCA is *nearly* plain DCF — one frame per win either way — but not
exactly: AC_BE contends with AIFSN 3, one slot time longer than DCF's DIFS. So this
comparison changes the MAC as well as the TXOP limit. The difference works against the
result shown below rather than for it: the longer AIFS makes EDCA marginally slower per
win, so it can only understate the gain that follows.

Rerunning the rate sweep tells a very different story. Where plain DCF's throughput
collapses as the slow station slows — dragging the fast stations down with it — TXOP holds
the aggregate roughly flat and keeps the fast stations near their full throughput:

.. figure:: media/txop-throughput-vs-rate.png
..
   FIGURE RECIPE (redo via ../txop-chart.py)
   type:     chart (matplotlib)
   shows:    aggregate and fast-station-average application throughput vs the slow station's
             bitrate, plain DCF vs 802.11e TXOP; DCF collapses as the slow rate drops, TXOP
             stays flat and high
   inputs:   results/solve/UplinkAnomaly-*.sca and results/solve/UplinkTxop-*.sca, BOTH 3 reps
             (DCF is not deterministic -- same RNG/backoff -- so both are averaged alike);
             results/UplinkHomogeneous-#0.sca for the all-fast baseline line
   record:   inet -u Cmdenv -c UplinkAnomaly     -r 0..17 --repeat=3 --result-dir=results/solve
             inet -u Cmdenv -c UplinkTxop        -r 0..17 --repeat=3 --result-dir=results/solve
             inet -u Cmdenv -c UplinkHomogeneous -r 0               --result-dir=results
   metric:   server.app[*] packetReceived:count x 0.002 -> Mbps; aggregate = sum over the 5
             apps per run, fast-avg = mean over app[1..4]; both configs averaged over 3 reps
   anchor:   structural -- if the UplinkAnomaly/UplinkTxop configs or sweep points change,
             re-derive. The sweep results are kept out of results/ root so they don't
             contaminate the DCF-only .anf charts (whose filters match
             packetReceived:count of any config).
   plot:     ../txop-chart.py (matplotlib; DCF red/orange, TXOP blue; 8x6 in @ dpi 150)
   stamp:    captured 2026-07, INET 4.6

At the widest gap — the slow station at 6 Mbps — the fast-station average recovers from about
2.5 Mbps under DCF to about 5.5 Mbps under TXOP, and the aggregate climbs from ~12 back to ~23
Mbps, most of the way to the ~24 Mbps all-fast baseline (the dashed line). The slow station
itself falls from ~2.3 to ~0.9 Mbps — and that *is* airtime fairness, not a side-effect: given
only its fair share of time, a 6 Mbps station can clock out proportionally fewer bits, so it
stops monopolising the medium and the others get their time back.

Once the rate gap narrows past about 9 Mbps, the TXOP aggregate rises *above* the plain-DCF
all-fast baseline altogether — it crosses the dashed line between the 9 and 12 Mbps sweep
points and stays above it for the rest of the sweep, reaching ~28 Mbps at 36 Mbps. Bursting
amortises the fixed per-exchange overhead — backoff, SIFS/DIFS, the PHY
preamble and PLCP header — over several frames, so even a healthy all-fast cell gets more
throughput under TXOP than under plain DCF. (At the two widest gaps the aggregate is still
recovering and stays just under the baseline: 22.7 and 23.5 against 24.2 Mbps.) That is why
modern 802.11 (n/ac/ax) does much the
same all the time — frame aggregation (A-MPDU), typically carried inside a TXOP, amortises the
same overhead; the showcase keeps a plain-DCF baseline only to isolate the rate-anomaly
mechanism from this efficiency bonus. The TXOP limit is still a tradeoff, not "bigger is always
better": longer bursts raise the other stations' latency and, under permanent saturation,
can starve a station outright — the EDCA lockout noted below.

TXOP and the access-point airtime scheduler noted earlier are two fixes for two faces of the
same anomaly, not the same fix twice. TXOP is *distributed* — it bounds each *contending*
station's win, so it is the one that applies to the uplink scenario shown here. The
``mac80211`` scheduler is *centralised* — it apportions a single transmitter's airtime among
its own destinations, so it is the fix for the *downlink*, AP-to-clients form. Both act on the
same principle — allocate channel *time*, not transmission *count* — but in different places.

One honest caveat — and the reason the chart plots only aggregate and group-average curves.
TXOP restores the *aggregate* capacity and the fast-station *group* reliably, but it does
**not** hand each individual station equal airtime here. Under this permanent, extreme
saturation INET's EDCA sometimes locks a station out completely: in several of the runs a
fast station receives *zero* throughput for the whole measurement window while its neighbours
absorb its share. Averaging over repetitions is what makes the group curves smooth — a
per-station plot would be dominated by that run-to-run lockout, so it is deliberately left
out. The system-level result (capacity recovered, the fast group no longer penalised) is
robust; perfect per-station fairness is not.

The Downlink Form: Airtime Fairness at the Access Point
-------------------------------------------------------

Everything so far has been *uplink*: independent stations contending, cured by bounding each
station's own airtime with TXOP. The anomaly has a mirror image on the **downlink**, when one
access point serves many clients of mixed rates. There is no contention to arbitrate now — a
single transmitter holds the channel and must divide *its own* airtime among its destinations,
so the fix is a **scheduling** decision inside the access point's transmit queue rather than a
contention rule in the MAC. This is the form production access points face, and the one the
Linux ``mac80211`` deficit-airtime scheduler cures; INET models that scheduler as the
:ned:`AirtimeFairnessQueue`.

A mixed-rate downlink cell
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``[Config DownlinkBase]`` reverses the traffic — the server sources one saturating UDP flow per
station and the access point relays them over the air — and makes ``sta[0]`` the slow client by
pinning the rate the access point uses toward it:

.. literalinclude:: ../omnetpp.ini
   :start-at: [Config DownlinkBase]
   :end-at: dataFrameBitratePerReceiver
   :language: ini

The access point serves ``sta[0]`` at 6 Mbps and every other client at the interface-wide 54 Mbps.
A real access point *rate-adapts* to each client — a distant or obstructed one settles on a low
rate on its own — and INET's ``dataFrameBitratePerReceiver`` lets us set that per-client rate
directly, keyed by the client's interface (resolved to a MAC address at run time). The result is a
**mixed-rate cell**: one AP radio transmitting at 6 Mbps to ``sta[0]`` and at 54 Mbps to the other
four — the setup that makes airtime scheduling necessary.

For the same slow client produced the *realistic* way — a wall in front of ``sta[0]`` plus rate
control, so its 6 Mbps is earned from a weak link rather than declared — see the ``ratecontrol``
showcase (its ``DownlinkRateAnomaly`` config). Pinning the rate here keeps the focus on the airtime
scheduling itself, free of rate-adaptation transients and packet loss.

Frame fairness versus airtime fairness at the AP
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The access point's transmit queue chooses which client's frame to send next; the two configs
flip a single switch on it, ``fairnessEnabled``:

.. literalinclude:: ../omnetpp.ini
   :start-at: pendingQueue.typename = "AirtimeFairnessQueue"
   :end-at: pendingQueue.fairnessEnabled = true
   :language: ini

``[Config DownlinkAnomaly]`` runs the queue **frame-fair** (``fairnessEnabled = false``): it serves
each client an equal number of *frames*, round-robin — the downlink analog of DCF's equal channel
access. ``[Config DownlinkAirtimeFair]`` runs it **airtime-fair** with a *deficit round-robin*:
each client carries a time budget, topped up by a fixed *quantum* — 1500 µs by default, which is
roughly one full-length frame's airtime at the low end of the rate range and several frames'
worth at the high end. Each frame the client sends is charged its own on-air duration, computed
from the rate and length actually used, so a slow client's long frames drain its budget fast.
The top-up is granted lazily rather than every round: the scheduler passes over a backlogged
client whose budget has gone negative, hands it one quantum, and moves on, returning to it once
it is back in credit. Over time every client therefore gets the same share of transmit time
whatever its frame size. (The charge is the frame's own air time — preamble, header and
payload — not the acknowledgment and interframe gaps around it. It is applied after the fact,
per transmission, so retransmissions are charged too; here the links are error-free and the
access point is the only transmitter, so there are effectively none.)

Both configurations also cap the queue at 50 frames *shared* across all clients, and that shared
cap needs an overflow rule. ``AirtimeFairnessQueue`` drops from the longest per-client backlog
rather than from the tail of whichever flow happens to overflow, which keeps a slow client's
backlog from crowding the others out of the queue. This matters more than it sounds: it is what
makes the frame-fair setting a clean *frame-fairness* baseline rather than a mixture of two
effects.

That is also why the anomaly baseline here is a per-client round robin and not a plain FIFO. A
stock access point with a single drop-tail FIFO does *not* reproduce the rate anomaly cleanly —
running this same cell with INET's default ``PendingQueue`` instead gives 4.1 Mbps to ``sta[0]``
and only 0.7–0.8 Mbps to each fast client, for an aggregate of 8.5 Mbps. That is a different and
deeper pathology: with one shared FIFO the slow client's backlog captures a growing share of the
queue and it ends up with *more* throughput than anyone else, on top of the airtime effect. The
round robin isolates the rate anomaly from that head-of-line blocking, so the comparison below
changes one thing only — how the airtime is divided.

Both runs are visible in the cell itself. The rate visualizer draws each client's PHY rate as a bar
above the access point — a short red bar at 6 Mbps for ``sta[0]``, tall green bars at 54 Mbps for
the other four — identical in both runs, since the rates are pinned. What changes between the two is
how the AP divides its transmit time, and each client's running received-packet count shows the
effect.

Serving equal *frames*, the frame-fair queue lets ``sta[0]``'s slow transmissions dominate the
channel — all five counts climb together to nearly the same value (459 to 492), the four fast clients
dragged down to the slow client's pace:

.. figure:: media/downlink-cell-anomaly.png
..
   FIGURE RECIPE (redo via the "omnetpp-mcp-sim" skill)
   type:     canvas (get_canvas_image, live inspector)
   config:   DownlinkAnomaly, run 0
   seed:     default
   shows:    the downlink cell under the frame-fair AP queue -- rate-visualizer bars (sta[0]=6 red,
             sta[1..4]=54 green) and per-station received-packet counts (all clients 459-492)
   anchor:   at t=2s (1s past warmup); counts scale with time, the anomaly/fix contrast is
             structural. If the bars stop showing 6 vs 54, the dcf datarateSelected wiring changed.
   capture:  get_canvas_image (post-2026-07 fix it renders the live inspector = Qtenv's "Export to
             Image", so the visualizer overlays land in place, no chrome) -- run to 2s express,
             set_canvas_view(RateAnomalyShowcase, fit=true), then
             get_canvas_image(RateAnomalyShowcase, area=module_rectangle, margin=5). No cropping.
   record:   inet -u Qtenv -c DownlinkAnomaly --mcp-server-address localhost:8765
             (DownlinkBase spreads the clients 4 m apart so the per-station readouts don't overlap)
   stamp:    captured 2026-07, INET 4.6

Switching the queue to airtime-fair leaves the rates and the layout untouched — the bars are
unchanged — but now each client is charged for the *time* its frames hold the channel, not their
number. ``sta[0]``'s long 6 Mbps frames spend its budget fast, so it is served rarely (its count
barely moves, ~132), while the four fast clients, cheap in airtime, are served far more often and
pull ahead (~870-960):

.. figure:: media/downlink-cell-fair.png
..
   FIGURE RECIPE (redo via the "omnetpp-mcp-sim" skill)
   type:     canvas (get_canvas_image, live inspector)
   config:   DownlinkAirtimeFair, run 0
   seed:     default
   shows:    the same cell under the airtime-fair AP queue -- identical rate-visualizer bars
             (sta[0]=6 red, sta[1..4]=54 green) but per-station counts now split: sta[0]=132,
             the four fast clients ~870-960
   anchor:   at t=2s (1s past warmup); counts scale with time, the anomaly/fix contrast is
             structural. If the bars stop showing 6 vs 54, the dcf datarateSelected wiring changed.
   capture:  same get_canvas_image live-inspector capture as the anomaly figure above, for this config.
   record:   inet -u Qtenv -c DownlinkAirtimeFair --mcp-server-address localhost:8765
             (DownlinkBase spreads the clients 4 m apart so the per-station readouts don't overlap)
   stamp:    captured 2026-07, INET 4.6

The per-station throughput makes the same point quantitatively:

.. figure:: media/downlink-throughput.png
..
   FIGURE RECIPE (redo via ../dl-chart.py)
   type:     chart (matplotlib)
   shows:    per-station application throughput, frame-fair (anomaly) vs airtime-fair AP queue,
             against the all-fast per-client baseline (dashed); frame-fair collapses all five
             clients to ~2.7 Mbps, airtime-fair lifts the four fast clients to ~5.3 -- slightly
             ABOVE the 4.9 baseline -- while holding sta[0] to its ~0.7 Mbps time share
   inputs:   results/dl/DownlinkAnomaly-*.sca, results/dl/DownlinkAirtimeFair-*.sca and
             results/dl/DownlinkHomogeneous-*.sca, ALL 3 reps
             (DCF backoff uses the RNG, so all three configs are averaged alike)
   record:   inet -u Cmdenv -c DownlinkAnomaly     -r 0..2 --repeat=3 --result-dir=results/dl
             inet -u Cmdenv -c DownlinkAirtimeFair -r 0..2 --repeat=3 --result-dir=results/dl
             inet -u Cmdenv -c DownlinkHomogeneous -r 0..2 --repeat=3 --result-dir=results/dl
   metric:   sta[i].app[0] packetReceived:count x 0.002 -> Mbps; per-station mean over 3 reps;
             baseline = mean over the five DownlinkHomogeneous clients
   anchor:   structural -- if the Downlink configs change, re-derive
   plot:     ../dl-chart.py (matplotlib; anomaly red, fix blue; 8x6 in @ dpi 150)
   stamp:    captured 2026-07, INET 4.6

Frame-fair scheduling reproduces the anomaly. ``sta[0]``'s frames sit on the air several times
longer, and serving them one-for-one with the fast clients' frames lets them swallow most of the
airtime — so all five clients collapse to about 2.7 Mbps and the aggregate falls to ~14 Mbps.
Airtime fairness reverses it: charged for the time they occupy, the four fast clients climb back
to about 5.3 Mbps each — almost exactly double — and the aggregate recovers to ~22 Mbps. (They
stop a little short of the ~5.5 Mbps a fast station reaches under uplink TXOP: here all five flows
funnel through the one access-point queue rather than five independent radios.)

The dashed line is what the same cell delivers with no rate gap at all — ``[Config
DownlinkHomogeneous]``, all five clients at 54 Mbps — which works out to 4.9 Mbps each, 24.5 Mbps
aggregate. The recovered fast clients land slightly *above* it, at 5.3. That is not a
measurement artefact but a consequence of what the queue charges: the deficit counts a frame's
own air time and not the acknowledgment and interframe gaps around it, and those fixed costs are
proportionally much larger for a short 54 Mbps frame than for a long 6 Mbps one. Equal *charged*
airtime therefore hands the fast clients slightly more than a fifth of the real channel time, and
``sta[0]`` slightly less. The effect is small, and it is the price of an accounting rule simple
enough to compute from the frame alone.

``sta[0]`` moves the other way, down to ~0.7 Mbps — and that is the fairness *working*, not the
slow client being punished. Under frame-fair scheduling ``sta[0]`` was the culprit rather than the
victim: it had been taking far more than its one-fifth share of channel *time*, so its 2.7 Mbps
was inflated by exactly the airtime it stole from the others. Airtime fairness holds it to an equal
*time* share, and ~0.7 Mbps is honestly all a 6 Mbps link delivers in roughly a fifth of the airtime —
while the aggregate *rises* from ~14 to ~22 Mbps, so nothing is taken from anyone. The fast
clients' recovery is precisely the airtime ``sta[0]`` was monopolising, handed back.

And unlike the uplink TXOP result, this fix is cleanly *per-station* fair: a single transmitter
running a deterministic scheduler has no contention lockout to average away, so all four fast
clients land on the same throughput.

Sources: :download:`omnetpp.ini <../omnetpp.ini>`,
:download:`RateAnomalyShowcase.ned <../RateAnomalyShowcase.ned>`

Try It Yourself
---------------

If you already have INET and OMNeT++ installed, start the IDE by typing
``omnetpp``, import the INET project into the IDE, then navigate to the
``inet/showcases/wireless/rateanomaly`` folder in the `Project Explorer`. There, you can
view and edit the showcase files, run simulations, and analyze results.

Otherwise, there is an easy way to install INET and OMNeT++ using `opp_env
<https://omnetpp.org/opp_env>`__, and run the simulation interactively.
Ensure that ``opp_env`` is installed on your system, then execute:

.. code-block:: bash

    $ opp_env run inet-4.6 --init -w inet-workspace --install --build-modes=release --chdir \
       -c 'cd inet-4.6.*/showcases/wireless/rateanomaly && inet'

.. TODO:: opp_env installs inet-4.6, which does not yet contain this showcase or the
   AirtimeFairnessQueue (still on a topic branch). Bump to the first release that ships
   them (likely 4.7) once available — the try-it path is not reproducible against 4.6.

This command creates an ``inet-workspace`` directory, installs the appropriate
versions of INET and OMNeT++ within it, and launches the ``inet`` command in the
showcase directory for interactive simulation.

Alternatively, for a more hands-on experience, you can first set up the
workspace and then open an interactive shell:

.. code-block:: bash

    $ opp_env install --init -w inet-workspace --build-modes=release inet-4.6
    $ cd inet-workspace
    $ opp_env shell

Inside the shell, start the IDE by typing ``omnetpp``, import the INET project,
then start exploring.

References
----------

- M. Heusse, F. Rousseau, G. Berger-Sabbatel, and A. Duda, "Performance Anomaly of
  802.11b," *Proc. IEEE INFOCOM 2003*, pp. 836–843.
  https://ieeexplore.ieee.org/document/1208921/
- T. Høiland-Jørgensen, M. Kazior, D. Täht, P. Hurtig, and A. Brunström, "Ending the
  Anomaly: Achieving Low Latency and Airtime Fairness in WiFi," *Proc. USENIX ATC 2017*,
  pp. 139–151. https://arxiv.org/abs/1703.00064

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-showcases/issues/TODO>`__ in
the GitHub issue tracker for commenting on this showcase.

Manual Stream Configuration
===========================

Goals
-----

In this example, we demonstrate manual configuration of stream identification,
stream splitting, stream merging, stream encoding, and stream decoding to achieve
the desired stream redundancy using the Frame Replication and Elimination for
Reliability (FRER) mechanism defined in IEEE 802.1CB.

| Verified with INET version: ``4.4``
| Source files location: `inet/showcases/tsn/framereplication/manualconfiguration <https://github.com/inet-framework/inet/tree/master/showcases/tsn/framereplication/manualconfiguration>`__

Overview
--------

FRER (Frame Replication and Elimination for Reliability)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Frame Replication and Elimination for Reliability (FRER) is a mechanism standardized
in IEEE 802.1CB that provides seamless redundancy for time-sensitive networking
applications. The core idea is to protect critical data streams against link failures
and packet loss by:

1. **Replicating frames**: At strategic points in the network, frames from a stream
   are duplicated and sent along multiple disjoint paths toward the destination.

2. **Eliminating duplicates**: At merge points and at the destination, duplicate
   frames are identified (using sequence numbers) and eliminated, ensuring that
   only one copy of each frame is delivered to the application.

Key FRER Concepts
^^^^^^^^^^^^^^^^^

**Stream Identification**
  Each packet flow is assigned to a named stream. Stream identification uses packet
  filters (typically matching MAC addresses, VLAN IDs, or other header fields) to
  determine which stream a packet belongs to.

**Sequence Numbering**
  To enable duplicate detection and elimination, each frame in a stream is assigned
  a sequence number at the source. This allows downstream merge points to identify
  and discard duplicate frames.

**Stream Encoding/Decoding**
  Streams are encoded into VLAN tags as they traverse the network. This allows
  switches to forward frames based on standard VLAN forwarding tables while
  maintaining stream identity. At each hop, incoming frames are decoded to
  determine their stream identity, processed, and then encoded again for the
  next hop.

**Stream Splitting (Replication)**
  At replication points, a single input stream is duplicated into multiple output
  streams, each sent on a different path. This creates redundancy - if one path
  fails, the other copies can still reach the destination.

**Stream Merging (Elimination)**
  At merge points, multiple input streams are combined into a single output stream.
  The merger uses sequence numbers to detect and eliminate duplicate frames,
  forwarding only the first copy of each frame that arrives.

**Benefits**
  FRER provides seamless redundancy with zero packet loss during link or node
  failures, making it ideal for critical industrial control systems, automotive
  networks, and other time-sensitive applications.

Network Implementation Strategy
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This showcase implements FRER in a network topology from the IEEE 802.1CB standard,
featuring one source node, one destination node, and five switches arranged to
provide multiple redundant paths.

Network Topology
^^^^^^^^^^^^^^^^

Here is the network:

.. figure:: media/Network.png
   :align: center

The network consists of:

- **source**: Generates a UDP data stream
- **s1**: First-level switch that performs the initial stream split
- **s2a, s2b**: Second-level switches that provide cross-path redundancy
- **s3a, s3b**: Third-level switches that forward streams to the destination
- **destination**: Receives the stream and performs final duplicate elimination

Redundant Path Structure
^^^^^^^^^^^^^^^^^^^^^^^^^

The network topology provides **four redundant paths** from source to destination:

1. **Path 1 (Upper Direct)**: source → s1 → s2a → s3a → destination
2. **Path 2 (Upper-to-Lower Cross)**: source → s1 → s2a → s2b → s3b → destination
3. **Path 3 (Lower Direct)**: source → s1 → s2b → s3b → destination
4. **Path 4 (Lower-to-Upper Cross)**: source → s1 → s2b → s2a → s3a → destination

The key to this redundancy is the connection between s2a and s2b, which
creates the cross-paths (Paths 2 and 4). This allows the network to tolerate
multiple simultaneous link failures.

Stream Hierarchy and Naming
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The streams are named according to their position in the network:

- **s1**: The original stream from the source
- **s2a, s2b**: First-level split streams (created at s1)
- **s2a-s2b**: Stream from s2a crossing to s2b
- **s2b-s2a**: Stream from s2b crossing to s2a  
- **s3a, s3b**: Second-level split streams (created at s2a and s2b)

This hierarchical naming reflects the replication and merging operations:

- At **s1**: Stream s1 is split into s2a and s2b
- At **s2a**: Streams s2a and s2b-s2a are merged, then split into s3a and s2b (cross-stream)
- At **s2b**: Streams s2b and s2a-s2b are merged, then split into s3b and s2a (cross-stream)
- At **destination**: Streams s3a and s3b are merged into a null stream (final elimination)

VLAN Encoding Strategy
^^^^^^^^^^^^^^^^^^^^^^^

The network uses two VLANs to maintain stream separation:

- **VLAN 1**: Used for primary path streams (s1, s2a, s3a, and cross-stream s2b-s2a)
- **VLAN 2**: Used for secondary path streams (s2b, cross-stream s2a-s2b)

Each switch maps streams to VLANs for forwarding, then the next hop decodes the
VLAN back to stream identity for processing.

Link Failure Scenario
^^^^^^^^^^^^^^^^^^^^^

The configuration includes a scenario that simulates two link failures to
demonstrate the network's resilience:

**At t=0.1s**: Link s1-s2a breaks
  - ❌ Path 1 fails (uses s1→s2a)
  - ❌ Path 2 fails (uses s1→s2a)
  - ✅ Path 3 survives (uses s1→s2b)
  - ✅ Path 4 survives (uses s1→s2b)

**At t=0.2s**: Link s2b-s3b breaks
  - ❌ Paths 1, 2 remain failed
  - ❌ Path 3 fails (uses s2b→s3b)
  - ✅ **Path 4 survives** (uses s2a→s3a)

After both failures, Path 4 remains operational: source → s1 → s2b → s2a → s3a → destination

This demonstrates that the mesh topology with FRER can maintain connectivity
even with multiple link failures, as long as at least one complete path exists.

The Model
---------

Configuration Details
~~~~~~~~~~~~~~~~~~~~~

Let's examine each section of the configuration and explain how it implements
the FRER strategy described above.

Basic Configuration
^^^^^^^^^^^^^^^^^^^

.. code-block:: ini

   # disable automatic MAC forwarding table configuration
   *.macForwardingTableConfigurator.typename = ""

Disables automatic MAC table configuration so we can manually configure stream
forwarding rules.

.. code-block:: ini

   # link breaks between switches
   *.scenarioManager.script = xml("<scenario> \
                                     <at t='0.1'> \
                                       <disconnect src-module='s1' dest-module='s2a'/> \
                                     </at> \
                                     <at t='0.2'> \
                                       <disconnect src-module='s2b' dest-module='s3b'/> \
                                     </at> \
                                   </scenario>")

Configures the link failure scenario: breaks the s1-s2a link at 0.1s and the
s2b-s3b link at 0.2s. This tests the network's ability to maintain connectivity
through the remaining redundant paths.

.. code-block:: ini

   # enable frame replication and elimination
   *.*.hasStreamRedundancy = true

Enables FRER functionality in all network nodes, allowing them to perform stream
splitting, merging, encoding, and decoding operations.

.. code-block:: ini

   # source application
   *.source.numApps = 1
   *.source.app[0].typename = "UdpSourceApp"
   *.source.app[0].io.destAddress = "destination"
   *.source.app[0].io.destPort = 1000
   *.source.app[0].source.displayStringTextFormat = "sent %p pk (%l)"
   *.source.app[0].source.packetLength = 1200B
   *.source.app[0].source.productionInterval = truncnormal(100us,50us)

Configures the source application to generate UDP packets with 1200-byte payloads
at intervals following a truncated normal distribution (mean 100μs, std dev 50μs).
The packets are sent to the destination node on port 1000.

.. code-block:: ini

   # destination application
   *.destination.numApps = 1
   *.destination.app[0].typename = "UdpSinkApp"
   *.destination.app[0].io.localPort = 1000

   # all interfaces must have the same address to accept packets from all streams
   *.destination.eth[*].address = "0A-AA-12-34-56-78"

Configures the destination to receive UDP packets on port 1000. Importantly, both
Ethernet interfaces are configured with the same MAC address so they can accept
packets from either path (s3a or s3b).

.. code-block:: ini

   # configure all egress traffic as part of stream s1, start sequence numbering
   *.source.bridging.streamIdentifier.identifier.mapping = [{packetFilter: "*", stream: "s1", sequenceNumbering: true}]
   # encode egress stream s1 to VLAN 1
   *.source.bridging.streamCoder.encoder.mapping = [{stream: "s1", vlan: 1}]

**Stream Identification**: All outgoing traffic is identified as stream "s1" and
sequence numbering is enabled. This is the entry point for FRER - each packet
gets assigned a unique sequence number that will be used for duplicate detection
throughout the network.

**Stream Encoding**: Stream s1 is encoded with VLAN tag 1 before being sent to s1.
This allows the network to route the stream using standard VLAN-based forwarding.

Switch s1 Configuration
^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: ini

   # map destination MAC address and VLAN pairs to network interfaces
   *.s1.macTable.forwardingTable = [{address: "destination", vlan: 1, interface: "eth0"},
                                    {address: "destination", vlan: 2, interface: "eth1"}]
   # allow ingress traffic from VLAN 1
   *.s1.ieee8021q.qTagHeaderChecker.vlanIdFilter = [1]

Sets up MAC forwarding: packets with destination MAC and VLAN 1 go to eth0 (s2a),
packets with VLAN 2 go to eth1 (s2b). Only VLAN 1 traffic is accepted from the source.

.. code-block:: ini

   # enable stream policing in layer 2 bridging
   *.s1.bridging.streamRelay.typename = "StreamRelayLayer"
   *.s1.bridging.streamCoder.typename = "StreamCoderLayer"
   # map eth2 VLAN 1 to stream s1
   *.s1.bridging.streamCoder.decoder.mapping = [{interface: "eth2", vlan: 1, stream: "s1"}]

Enables FRER functionality and decodes incoming VLAN 1 traffic on eth2 as stream s1.

.. code-block:: ini

   # eliminate duplicates from stream s1
   *.s1.bridging.streamRelay.merger.mapping = {s1: "s1"}

Configures the merger to eliminate duplicates in stream s1. Since this is the
first switch, there shouldn't be duplicates yet, but this prepares for any
potential loops or retransmissions.

.. code-block:: ini

   # split stream s1 into s2a and s2b
   *.s1.bridging.streamRelay.splitter.mapping = {s1: ["s2a", "s2b"]}
   # map stream s2a to VLAN 1 and s2b to VLAN 2
   *.s1.bridging.streamCoder.encoder.mapping = [{stream: "s2a", vlan: 1},
                                                {stream: "s2b", vlan: 2}]

**Critical Replication Point**: This is where the initial split occurs. Stream s1
is duplicated into two separate streams (s2a and s2b), creating the first level
of redundancy. Stream s2a is encoded with VLAN 1 and forwarded to s2a, while
stream s2b is encoded with VLAN 2 and forwarded to s2b.

Switch s2a Configuration
^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: ini

   # map destination MAC address and VLAN pairs to network interfaces
   *.s2a.macTable.forwardingTable = [{address: "destination", vlan: 1, interface: "eth0"},
                                     {address: "destination", vlan: 2, interface: "eth1"}]
   # allow ingress traffic from VLAN 1 and 2
   *.s2a.ieee8021q.qTagHeaderChecker.vlanIdFilter = [1, 2]

MAC forwarding for s2a: VLAN 1 traffic goes to eth0 (s3a), VLAN 2 goes to eth1
(s2b for the cross-path). Accepts both VLAN 1 and 2 traffic.

.. code-block:: ini

   # enable stream policing in layer 2 bridging
   *.s2a.bridging.streamRelay.typename = "StreamRelayLayer"
   *.s2a.bridging.streamCoder.typename = "StreamCoderLayer"
   # map eth2 VLAN 1 to stream s2a and eth1 VLAN 2 to stream s2b-s2a
   *.s2a.bridging.streamCoder.decoder.mapping = [{interface: "eth2", vlan: 1, stream: "s2a"},
                                                 {interface: "eth1", vlan: 2, stream: "s2b-s2a"}]

Decodes incoming traffic: eth2 VLAN 1 → stream s2a (from s1), eth1 VLAN 2 →
stream s2b-s2a (cross-path from s2b).

.. code-block:: ini

   # merge streams s2a and s2b-s2a in into s3a
   *.s2a.bridging.streamRelay.merger.mapping = {s2a: "s3a", "s2b-s2a": "s3a"}

**Merge Point**: Combines streams s2a (from s1) and s2b-s2a (cross-path from s2b)
into a single stream s3a. The merger eliminates duplicates using sequence numbers,
ensuring each frame is forwarded only once even if it arrives on both paths.

.. code-block:: ini

   # split stream s2a into s3a and s2b
   *.s2a.bridging.streamRelay.splitter.mapping = {s3a: ["s3a", "s2b"]}
   # map stream s3a to VLAN 1 and s2b to VLAN 2
   *.s2a.bridging.streamCoder.encoder.mapping = [{stream: "s3a", vlan: 1},
                                                 {stream: "s2b", vlan: 2}]

**Second-Level Split**: Creates mesh redundancy by splitting the merged stream s3a
into:
- Stream s3a (VLAN 1) → forwarded to s3a toward destination
- Stream s2b (VLAN 2) → cross-path forwarded to s2b for additional redundancy

This creates Paths 1 and 4 from the source.

Switch s2b Configuration
^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: ini

   # map destination MAC address and VLAN pairs to network interfaces
   *.s2b.macTable.forwardingTable = [{address: "destination", vlan: 1, interface: "eth0"},
                                     {address: "destination", vlan: 2, interface: "eth1"}]
   # allow ingress traffic from VLAN 1 and 2
   *.s2b.ieee8021q.qTagHeaderChecker.vlanIdFilter = [1, 2]

Similar to s2a: VLAN 1 to eth0 (s3b), VLAN 2 to eth1 (s2a cross-path).

.. code-block:: ini

   # enable stream policing in layer 2 bridging
   *.s2b.bridging.streamRelay.typename = "StreamRelayLayer"
   *.s2b.bridging.streamCoder.typename = "StreamCoderLayer"
   # map eth2 VLAN 2 to stream s2b and eth1 VLAN 2 to stream s2a-s2b
   *.s2b.bridging.streamCoder.decoder.mapping = [{interface: "eth2", vlan: 2, stream: "s2b"},
                                                 {interface: "eth1", vlan: 2, stream: "s2a-s2b"}]

Decodes: eth2 VLAN 2 → stream s2b (from s1), eth1 VLAN 2 → stream s2a-s2b
(cross-path from s2a).

.. code-block:: ini

   # merge streams s2b and s2a-s2b in into s3b
   *.s2b.bridging.streamRelay.merger.mapping = {s2b: "s3b", "s2a-s2b": "s3b"}

**Merge Point**: Combines streams s2b (from s1) and s2a-s2b (cross-path from s2a)
into stream s3b, eliminating duplicates.

.. code-block:: ini

   # split stream s2b into s3b and s2a
   *.s2b.bridging.streamRelay.splitter.mapping = {s3b: ["s3b", "s2a"]}
   # stream s3a maps to VLAN 1 and s2a to VLAN 2
   *.s2b.bridging.streamCoder.encoder.mapping = [{stream: "s3b", vlan: 1},
                                                 {stream: "s2a", vlan: 2}]

**Second-Level Split**: Creates the complementary mesh by splitting s3b into:
- Stream s3b (VLAN 1) → forwarded to s3b toward destination
- Stream s2a (VLAN 2) → cross-path forwarded to s2a for additional redundancy

This creates Paths 2 and 3 from the source.

Switches s3a and s3b Configuration
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: ini

   # s3a configuration
   *.s3a.macTable.forwardingTable = [{address: "destination", vlan: 1, interface: "eth0"}]
   *.s3a.bridging.streamCoder.decoder.mapping = [{interface: "eth1", vlan: 1, stream: "s3a"}]
   *.s3a.bridging.streamCoder.encoder.mapping = [{stream: "s3a", vlan: 1}]
   *.s3a.ieee8021q.qTagHeaderChecker.vlanIdFilter = [1]

   # s3b configuration
   *.s3b.macTable.forwardingTable = [{address: "destination", vlan: 1, interface: "eth0"}]
   *.s3b.bridging.streamCoder.decoder.mapping = [{interface: "eth1", vlan: 1, stream: "s3b"}]
   *.s3b.bridging.streamCoder.encoder.mapping = [{stream: "s3b", vlan: 1}]
   *.s3b.ieee8021q.qTagHeaderChecker.vlanIdFilter = [1]

Both s3a and s3b are simple forwarding switches with no splitting or merging.
They decode their respective streams (s3a or s3b) from VLAN 1, and forward them
to the destination with VLAN 1 encoding. These switches simply relay the streams
toward the final destination.

Destination Node FRER Configuration
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: ini

   # allow ingress traffic from VLAN 1
   *.destination.ieee8021q.qTagHeaderChecker.vlanIdFilter = [1]
   # map eth0 VLAN 1 to stream s3a and eth1 VLAN 1 to stream s3b
   *.destination.bridging.streamCoder.decoder.mapping = [{interface: "eth0", vlan: 1, stream: "s3a"},
                                                         {interface: "eth1", vlan: 1, stream: "s3b"}]
   # merge streams s3a and s3b into null stream
   *.destination.bridging.streamRelay.merger.mapping = {s3a: "", s3b: ""}

**Final Elimination Point**: The destination decodes streams s3a and s3b from both
interfaces and merges them into a null stream (empty string). This performs the
final duplicate elimination - only the first copy of each frame (identified by
sequence number) is delivered to the application, while later duplicates are
discarded. This ensures the application receives exactly one copy of each frame,
regardless of which path(s) it arrived on.

Results
-------

Here are the number of received and sent packets:

.. figure:: media/packetsreceivedsent.png
   :align: center
   :width: 100%

Here is the ratio of received and sent packets:

.. figure:: media/packetratio.png
   :align: center

The expected number of successfully received packets relative to the number of
sent packets is verified by the Python script (``compute_frame_replication_success_rate_analytically2()`` function in ``inet/python/inet/tests/validation.py``). The expected result is around 0.657.

This ratio reflects the packet loss that occurs due to the two link failures.
Despite both failures, approximately 65.7% of packets successfully reach the
destination through the remaining redundant Path 4 (source → s1 → s2b → s2a →
s3a → destination). This demonstrates the effectiveness of the FRER mechanism
in maintaining network connectivity even under multiple failure conditions.

.. The following video shows the behavior in Qtenv:

   .. video:: media/behavior.mp4
      :align: center
      :width: 90%

   Here are the simulation results:

   .. .. figure:: media/results.png
      :align: center
      :width: 100%


Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`ManualConfigurationShowcase.ned <../ManualConfigurationShowcase.ned>`


Try It Yourself
---------------

If you already have INET and OMNeT++ installed, start the IDE by typing
``omnetpp``, import the INET project into the IDE, then navigate to the
``inet/showcases/tsn/framereplication/manualconfiguration`` folder in the `Project Explorer`. There, you can view
and edit the showcase files, run simulations, and analyze results.

Otherwise, there is an easy way to install INET and OMNeT++ using `opp_env
<https://omnetpp.org/opp_env>`__, and run the simulation interactively.
Ensure that ``opp_env`` is installed on your system, then execute:

.. code-block:: bash

    $ opp_env run inet-4.5 --init -w inet-workspace --install --build-modes=release --chdir \
       -c 'cd inet-4.5.*/showcases/tsn/framereplication/manualconfiguration && inet'

This command creates an ``inet-workspace`` directory, installs the appropriate
versions of INET and OMNeT++ within it, and launches the ``inet`` command in the
showcase directory for interactive simulation.

Alternatively, for a more hands-on experience, you can first set up the
workspace and then open an interactive shell:

.. code-block:: bash

    $ opp_env install --init -w inet-workspace --build-modes=release inet-4.5
    $ cd inet-workspace
    $ opp_env shell

Inside the shell, start the IDE by typing ``omnetpp``, import the INET project,
then start exploring.

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/789>`__ page in the GitHub issue tracker for commenting on this showcase.

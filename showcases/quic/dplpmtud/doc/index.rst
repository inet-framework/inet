QUIC Path MTU Discovery with DPLPMTUD
======================================

Goals
-----

This showcase demonstrates QUIC's Datagram Packetization Layer Path MTU Discovery
(DPLPMTUD) mechanism, which enables QUIC connections to dynamically discover and
adapt to the maximum transmission unit (MTU) of the network path. Path MTU discovery
is crucial for optimizing network efficiency by using the largest possible packet
size without causing fragmentation.

DPLPMTUD is an evolution of traditional MTU discovery mechanisms (like PMTUD), 
designed specifically for packetized transport protocols. Unlike PMTUD which relies
on ICMP Packet Too Big (PTB) messages, DPLPMTUD can operate with or without ICMP
feedback, making it more robust in networks where ICMP messages may be filtered or
unreliable.

In this simulation, we explore several scenarios that demonstrate key aspects of
DPLPMTUD:

- **Path MTU constraints**: How QUIC discovers when the path MTU equals or differs
  from the maximum probe size
- **ICMP-based vs. ICMP-free discovery**: Comparing discovery with and without
  Packet Too Big messages
- **Dynamic path changes**: How DPLPMTUD adapts when the network path changes,
  resulting in increased or decreased MTU
- **Candidate probe sequences**: Different strategies for probing larger MTU values

The showcase illustrates how DPLPMTUD helps QUIC maintain optimal packet sizes
throughout the connection lifetime, even when network conditions change.

| Verified with INET version: ``4.6.0``
| Source files location: `inet/showcases/quic/dplpmtud <https://github.com/inet-framework/inet/tree/master/showcases/quic/dplpmtud>`__

The Model
---------

The Network
~~~~~~~~~~~

The network topology consists of a sender and receiver connected through multiple
router paths. The design includes redundant paths to enable dynamic path switching,
which allows us to demonstrate MTU changes during an active connection.

.. figure:: media/network.png
   :width: 70%
   :align: center

The network features:

- **Sender**: QUIC-enabled host using ``TrafficgenCompound`` to generate data
- **Receiver**: QUIC-enabled host with ``QuicDiscardServer`` application
- **Multiple routers**: Creating redundant paths with different MTU configurations
- **ScenarioManager**: Controls dynamic network topology changes

The key to this showcase is the ability to configure different MTU values on
different links, creating scenarios where the path MTU may be constrained by
intermediate routers.

DPLPMTUD Configuration
~~~~~~~~~~~~~~~~~~~~~~

QUIC's DPLPMTUD implementation includes several configurable parameters:

.. code-block:: ini

   **.sender.quic.useDplpmtud = true
   **.sender.quic.dplpmtudMinPmtu = 1280         # Minimum IPv6 MTU
   **.sender.quic.dplpmtudCandidateSequence = "OptBinary"
   **.sender.quic.dplpmtudUsePtb = true/false    # Enable/disable ICMP PTB

**Candidate probe sequences** determine how DPLPMTUD searches for larger MTU values:

- **Up**: Incrementally increasing probes
- **Down**: Decreasing probes from maximum
- **Binary**: Binary search approach
- **OptBinary**: Optimized binary search (recommended)
- **DownUp**: Start high, then probe upward if failed
- **Jump**: Large jumps between probe sizes

Scenarios
---------

Scenario 1: Max Equals PMTU
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Configuration: ``[Config MaxEqualsPmtu]``

In this baseline scenario, all links have the same MTU (1300 bytes), so the path
MTU equals the maximum link MTU throughout the path.

.. code-block:: ini

   [Config MaxEqualsPmtu]
   **.ppp[*].ppp.mtu = 1300B

**Expected behavior**: DPLPMTUD should quickly discover that 1300 bytes is the
maximum usable packet size and stabilize at this value.

.. figure:: media/pmtu_max_equals.png
   :width: 100%
   :align: center

   *Placeholder: Chart showing PMTU discovery converging to 1300 bytes*

The sender transmits probe packets of increasing sizes until it confirms that
1300 bytes is the maximum. Once confirmed, regular data transmission uses this
optimal packet size.

Scenario 2: Max Larger Than PMTU
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Configuration: ``[Config MaxLargerPmtu]``

This scenario creates a bottleneck where the path between router1 and router2
has a smaller MTU (1300 bytes) than the other links (1500 bytes).

.. code-block:: ini

   [Config MaxLargerPmtu]
   **.router1.ppp[1].ppp.mtu = 1300B
   **.router2.ppp[0].ppp.mtu = 1300B
   **.ppp[*].ppp.mtu = 1500B

**Expected behavior**: DPLPMTUD discovers the 1300-byte constraint even though
most links support 1500 bytes.

.. figure:: media/pmtu_max_larger.png
   :width: 100%
   :align: center

   *Placeholder: Chart showing PMTU discovery settling at 1300 bytes despite
   1500-byte endpoint MTUs*

With ICMP PTB enabled, the sender receives explicit feedback when packets exceed
the path MTU. Without ICMP (``MaxLargerPmtuWithoutPtb``), discovery relies on
detecting lost probe packets.

Scenario 3: Increasing PMTU During Connection
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Configuration: ``[Config IncreasingPmtu]``

This scenario demonstrates DPLPMTUD's ability to detect and utilize increased
path MTU during an active connection. Initially, the path MTU is 1300 bytes,
but at t=5s, the network switches to an alternative path with 1400-byte MTU.

.. code-block:: ini

   [Config IncreasingPmtu]
   extends = MaxLargerPmtu
   **.router1.ppp[2].ppp.mtu = 1400B
   **.router3.ppp[0].ppp.mtu = 1400B
   *.scenarioManager.script = xml("<x> <at t='5'> 
       <disconnect src-module='router1' dest-module='router2'/> 
   </at> </x>")

**Expected behavior**: The sender initially discovers 1300 bytes as the path MTU.
After the path change at t=5s, DPLPMTUD continues probing and discovers it can
now use 1400-byte packets, increasing throughput efficiency.

.. figure:: media/pmtu_increasing.png
   :width: 100%
   :align: center

   *Placeholder: Timeline chart showing PMTU increasing from 1300 to 1400 bytes
   at t=5s*

This demonstrates DPLPMTUD's proactive nature—it periodically probes for larger
MTU values to take advantage of improved network conditions.

.. figure:: media/throughput_increasing_pmtu.png
   :width: 100%
   :align: center

   *Placeholder: Throughput chart showing efficiency improvement after MTU increase*

Scenario 4: Decreasing PMTU During Connection
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Configuration: ``[Config DecreasingPmtu]``

This scenario shows how DPLPMTUD responds when the path MTU decreases during an
active connection, which can occur due to network path changes or routing updates.

.. code-block:: ini

   [Config DecreasingPmtu]
   extends = MaxLargerPmtu
   **.router1.ppp[2].ppp.mtu = 1280B
   **.router3.ppp[0].ppp.mtu = 1280B
   *.scenarioManager.script = xml("<x> <at t='5'> 
       <disconnect src-module='router1' dest-module='router2'/> 
   </at> </x>")

**Expected behavior**: The sender starts with a 1300-byte PMTU. After the path
change at t=5s redirects traffic through links with 1280-byte MTU, the sender
must detect this reduction and adjust packet sizes accordingly.

.. figure:: media/pmtu_decreasing.png
   :width: 100%
   :align: center

   *Placeholder: Timeline chart showing PMTU decreasing from 1300 to 1280 bytes
   at t=5s*

With ICMP PTB enabled, the sender receives immediate feedback about the MTU
violation. Without ICMP (``DecreasingPmtuWithoutPtb``), the sender detects the
issue through packet loss and probes downward to find the new path MTU.

.. figure:: media/packet_loss_decreasing.png
   :width: 100%
   :align: center

   *Placeholder: Chart showing packet loss spike during MTU reduction, followed
   by recovery*

Results
-------

PMTU Discovery Effectiveness
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The following chart compares PMTU discovery with and without ICMP PTB support
across different scenarios:

.. figure:: media/pmtu_comparison.png
   :width: 100%
   :align: center

   *Placeholder: Comparison chart showing PMTU values over time for different
   configurations*

**Key observations**:

- With ICMP PTB enabled, PMTU discovery is faster and more responsive to changes
- Without ICMP, discovery still succeeds but requires more time and probe packets
- The OptBinary candidate sequence provides efficient convergence in most scenarios

Probe Packet Overhead
~~~~~~~~~~~~~~~~~~~~~

DPLPMTUD involves sending probe packets to test larger MTU values. The following
analysis shows the overhead:

.. figure:: media/probe_overhead.png
   :width: 100%
   :align: center

   *Placeholder: Chart showing number of probe packets vs. total packets for
   different candidate sequences*

The OptBinary sequence minimizes probe overhead while maintaining good discovery
performance.

Impact on Throughput
~~~~~~~~~~~~~~~~~~~~

Using optimal MTU values significantly impacts throughput efficiency. The following
chart compares throughput with DPLPMTUD enabled vs. using a fixed conservative MTU:

.. figure:: media/throughput_comparison.png
   :width: 100%
   :align: center

   *Placeholder: Throughput comparison showing benefits of adaptive MTU discovery*

With DPLPMTUD discovering and using the maximum path MTU, fewer packets are needed
to transmit the same amount of data, reducing protocol overhead and improving
overall efficiency.

Packet Size Distribution
~~~~~~~~~~~~~~~~~~~~~~~~~

.. figure:: media/packet_size_distribution.png
   :width: 100%
   :align: center

   *Placeholder: Histogram showing packet size distribution with DPLPMTUD active*

The distribution shows that most data packets use the discovered optimal MTU, with
occasional smaller packets for control frames and probe packets testing larger sizes.

Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`Dplpmtud.ned <../Dplpmtud.ned>`

Try It Yourself
---------------

If you already have INET and OMNeT++ installed, start the IDE by typing
``omnetpp``, import the INET project into the IDE, then navigate to the
``inet/showcases/quic/dplpmtud`` folder in the `Project Explorer`. There, you can view
and edit the showcase files, run simulations, and analyze results.

Otherwise, there is an easy way to install INET and OMNeT++ using `opp_env
<https://omnetpp.org/opp_env>`__, and run the simulation interactively.
Ensure that ``opp_env`` is installed on your system, then execute:

.. code-block:: bash

    $ opp_env run inet-4.6 --init -w inet-workspace --install --build-modes=release --chdir \
       -c 'cd inet-4.6.*/showcases/quic/dplpmtud && inet'

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

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-showcases/issues/TBD>`__ in
the GitHub issue tracker for commenting on this showcase.

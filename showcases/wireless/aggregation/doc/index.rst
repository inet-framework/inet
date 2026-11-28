IEEE 802.11 Frame Aggregation
=============================

Goals
-----

IEEE 802.11 frame aggregation is a feature that can improve the performance of
wireless networks by reducing the overhead associated with transmitting small
packets. When sending small packets over a wireless network using the 802.11
protocol, the overhead from protocol headers and other control information can
become significant, resulting in reduced performance. Frame aggregation helps to
mitigate this performance hit by allowing multiple data packets to be sent in a
single 802.11 transmission. This reduces the number of contention periods,
interframe spaces, and protocol headers, increasing the overall throughput of
the network.

In this showcase, we will demonstrate frame aggregation in INET's 802.11 model
and examine its effect on performance when sending a large number of small packets.

| Verified with INET version: ``4.7``
| Source files location: `inet/showcases/wireless/aggregation <https://github.com/inet-framework/inet/tree/master/showcases/wireless/aggregation>`__

About Aggregation
-----------------

Frame aggregation increases throughput by sending multiple data frames in a single transmission.
It reduces 802.11 protocol overhead, as multiple packets can be sent with a single PHY and
MAC header, instead of each packet having its own headers. The number of ACKs and interframe
spaces (and contention periods, if not in a TXOP) is also reduced.

There are two kinds of frame aggregation in 802.11:

- MAC Service Data Unit (MSDU) aggregation: the packets received by the MAC from the upper layer are MSDUs. Each packet gets an MSDU subframe header. Two or more subframes are bundled together and put in an 802.11 MAC frame (header + trailer). The resulting frame is an aggregate-MSDU (a-MSDU). The a-MSDUs are transmitted with a single PHY header by the radio.
- MAC Protocol Data Unit (MPDU) aggregation: MPDUs are frames passed from the MAC to the PHY layer. Each MPDU has a MAC header and trailer. Multiple MPDUs are bundled together to create an aggregate MPDU (a-MPDU), which is transmitted with a PHY header by the radio.

.. figure:: media/dataunits3.png
   :width: 80%
   :align: center

When using aggregation, the reduced number of contention periods, interframe spaces and ACKs might have a greater effect
on performance than the shorter headers. In this regard, there might not be much difference between
MSDU and MPDU aggregation.

MSDU aggregation has less overhead than MPDU aggregation. Each packet in an a-MSDU
has an MSDU subframe header, but the aggregate frame has just one MAC header.
In contrast, in a-MPDUs, each data packet has a MAC header.
(The MAC header + trailer is 30B, the MSDU subframe header is 14B.)

Thus, a-MPDUs have more overhead, but each data packet has its own
`Frame check sequence` (FCS) contained in the MAC trailer (in contrast
with a-MSDUs, which have just one FCS for the whole aggregate frame).
This way, the individual data packets sent in the a-MPDU can be
acknowledged with a block acknowledgment frame, making
MPDU aggregation more useful in a high error rate environment
(the performance gain from using a-MSDUs might be negated by
the higher error rate in such an environment).
A standalone QoS Data MPDU carrying an A-MSDU can receive a Normal Ack. This showcase uses that exchange.

Here is an a-MSDU of three subframes displayed in qtenv's packet view:

.. figure:: media/subframe.png
   :width: 100%
   :align: center

In this showcase, MSDU aggregation is
controlled by the aggregation policy in :ned:`Ieee80211Mac`. The aggregation
policy is a submodule of the MAC's :ned:`Hcf` submodule. The aggregation policy
submodule is located at ``mac.hcf.originatorMacDataService.msduAggregationPolicy``
in the module hierarchy. By default,
the :ned:`Hcf` module contains a :ned:`BasicMsduAggregationPolicy` submodule.

Aggregation can be controlled by the following parameters of :ned:`BasicMsduAggregationPolicy`:

- :par:`subframeNumThreshold`: Specifies the minimum number of frames needed to create an a-MSDU. By default, the number of frames is not checked. Thus, the MAC creates an a-MSDU from any number of packets present in the queue when the MAC decides to transmit.
- :par:`aggregationLengthThreshold`: Specifies the minimum length for an aggregated payload needed to create an a-MSDU. By default, the length is not checked.
- :par:`maxAMsduSize`: Specifies the maximum size for an a-MSDU. By default, it's 4065 bytes.

When the MAC creates an a-MSDU, the resulting a-MSDU complies with all three of these
parameters (if it didn't comply with any of them, then no a-MSDU would be created,
and the frames would be sent without aggregation).

IEEE Std 802.11-2024 clause 10.11 requires a station operating in this band to implement HT
and to have received an HT Capabilities element from its peer before sending an A-MSDU.
A legacy ERP station can use QoS channel access and transmit multiple frames in a TXOP.
Enabling the aggregation policy alone leaves its A-MSDU count at zero.

The Model
---------

The network has two hosts. In the ERP control they communicate in ad hoc mode.
For the HT cases, host1 associates with host2, which runs access point management
and the UDP sink. Association establishes peer HT capabilities before the traffic starts.

.. figure:: media/network.png
   :width: 60%
   :align: center

Configurations
~~~~~~~~~~~~~~

All four configurations send 100-byte UDP payloads from host1 to host2 every
72.7 microseconds, starting at 0.2 s in a 10 s simulation. They use the same
configured 54 Mbit/s bitrate. The table identifies the comparison each run supports.

.. list-table::
   :header-rows: 1

   * - Configuration
     - Operating mode
     - A-MSDU policy
     - Access category
   * - ErpAggregationControl
     - 802.11g ad hoc
     - Enabled, 1,500 B limit
     - Best effort
   * - NoAggregation
     - 802.11n infrastructure
     - Disabled
     - Best effort
   * - Aggregation
     - 802.11n infrastructure
     - Enabled, 1,500 B limit
     - Best effort
   * - VoicePriorityAggregation
     - 802.11n infrastructure
     - Enabled, 1,500 B limit
     - Voice

Compare ``NoAggregation`` and ``Aggregation`` to measure the effect of A-MSDU formation
with the same association, offered load, access category and PHY settings.
Compare ``Aggregation`` and ``VoicePriorityAggregation`` to examine the effect of
classifying the same traffic into the voice access category. The ERP run checks capability eligibility.

Results
-------

For run 0, read ``packetAggregated:count`` from the sender's HCF originator data service,
``packetDeaggregated:count`` from the receiver's HCF recipient data service, and
``packetReceived:count`` from the receiver's UDP sink. Each aggregate counter counts
complete A-MSDUs. The sink counts the individual application datagrams they deliver.

All four senders offered 134,801 datagrams in run 0. The two HT best-effort runs
delivered 28,135 datagrams with aggregation disabled and 134,794 with aggregation enabled.

.. list-table:: Run 0, 10 s simulation
   :header-rows: 1

   * - Configuration
     - A-MSDUs formed
     - A-MSDUs deaggregated
     - UDP datagrams received
   * - ErpAggregationControl
     - 0
     - 0
     - 48,014
   * - NoAggregation
     - 0
     - 0
     - 28,135
   * - Aggregation
     - 20,281
     - 20,281
     - 134,794
   * - VoicePriorityAggregation
     - 35,705
     - 35,704
     - 134,797

Open ``Aggregation.anf`` after running all four configurations to plot sink deliveries,
throughput and end-to-end delay. The delay chart shows arrivals between 2.39 and 2.40 s.
Packets carried in one A-MSDU arrive together even though their send times differ.

Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`AggregationShowcase.ned <../AggregationShowcase.ned>`


Try It Yourself
---------------

If you already have INET and OMNeT++ installed, start the IDE by typing
``omnetpp``, import the INET project into the IDE, then navigate to the
``inet/showcases/wireless/aggregation`` folder in the `Project Explorer`. There, you can view
and edit the showcase files, run simulations, and analyze results.

Otherwise, there is an easy way to install INET and OMNeT++ using `opp_env
<https://omnetpp.org/opp_env>`__, and run the simulation interactively.
Ensure that ``opp_env`` is installed on your system, then execute:

.. code-block:: bash

    $ opp_env run inet-4.6 --init -w inet-workspace --install --build-modes=release --chdir \
       -c 'cd inet-4.6.*/showcases/wireless/aggregation && inet'

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

Use `this <https://github.com/inet-framework/inet-showcases/issues/44>`__ page in the GitHub issue tracker for commenting on this showcase.

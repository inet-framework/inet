.. _ug:cha:collecting-results:

Collecting Results
==================

The following sections introduce the INET specific concepts and features for
collecting simulation results. For more information in general on collecting
statistics, please refer to the OMNeT++ manual.

.. _ug:sec:results:recording-statistics:

Recording Statistics
--------------------

Most INET modules are already equipped with the collection of various statistics,
defined using the ``@statistic`` properties in the corresponding NED files.
By default, many of them are already configured to be automatically recorded
as either scalars, vectors, or histograms depending on the typical usefulness
of the statistic. Note that the default recording mode can be changed
from INI files as described in the OMNeT++ manual.

If the default statistics provided by the INET modules are not sufficient,
new modules can be derived using only NED files and new statistics
can be added to them based on the signals emitted by the module. The emitted signals
can be found as ``@signal`` properties declared in the corresponding NED files.

If even declaring new statistics isn't sufficient, new
C++ classes can be derived from the module implementations and new signals and/or
new statistic collection code can be added to them. This is the most cumbersome way to collect
new results, but it's also the most expressive, allowing to collect any kind of
statistic.

Measuring along Packet Flows
----------------------------

By default, INET statistics are only capable of collecting results based on the
data that individual protocol modules and applications can access. Statistics are collected
independently by each module in the network, making them less useful in complex scenarios.
For example, a TCP protocol module that communicates with multiple other TCP modules cannot distinguish
between the packets based on the path they took. To overcome this issue, INET
introduces the notion of packet flows.

A packet flow is a logical classification of packets, identified by its name,
over the whole network and over the duration of the whole simulation. Basically,
at any given moment, any packet that is present anywhere in the network can be
part of any number of packet flows. Packets may enter a packet flow and leave
it multiple times. Different packet flows can overlap both in time and also
along the network topology. Note that a packet flow doesn't necessarily have
a single entry and a single exit point.

A packet flow is usually defined by active modules that classify certain packets
(e.g., matching a filter) to be entering the flow, and similarly, other modules
that decide when packets leave the flow. Both kinds of modules are inserted into
the network for this specific purpose, usually at the network interface level.
While a packet is being part of any number of packet flows, certain INET modules
(e.g., queues) are going to automatically record certain events that happen with
the packets (e.g., queueing).

So far, the notion of packet flows was introduced on the packet level. That is,
at any given time, a packet is either completely part of a packet flow or not.
In fact, this is a simplification for easier understanding. This approach is
clearly not sufficient in the general case because packets can be fragmented
and aggregated throughout the network, and they can traverse many different
paths. Therefore, the packet flow membership is actually specified on a per-bit
basis. The implementation takes care of efficiently representing this data, so
the coherent parts of a packet that belong to the same packet flow are marked
together.

Using the packet flow mechanism, one can easily measure the timing of various
things that happen to a packet (or to a part of it). The following quantities
are automatically measured (if requested) along packet flows:

-  total elapsed time measured from entering the packet flow
-  total time spent in queues (e.g., transmission queue)
-  total delay spent in various non-queue modules (e.g., interframe gap)
-  total time spent in various packet processing modules (e.g., packet server)
-  total transmission time of transmitters
-  total propagation time spent on the transmission medium

The collected timing data is attached to coherent regions of the packets while
the packets are in the packet flow. The actual measurement, that is, collecting
the statistical results, is usually done when the packets leave the packet flow.

If the timing statistics are not sufficient, it's also possible to collect all
packet events that happen to packets in the packet flow. The following packet
events can be automatically collected along packet flows:

-  packet is enqueued in a packet queue (e.g., transmission queue)
-  packet is delayed (e.g., interframe gap)
-  packet is processed (e.g., packet server)
-  packet is transmitted by a wired or wireless transmitter
-  packet is propagated on a wired or wireless transmission medium

The collected packet event data structure is also attached to coherent regions
of the packet while the packets are in the packet flow. To actually make a
measurement, a new measurement module must be implemented which processes the
collected data.

In order to configure one of the above packet flow measurements, the following modules can be used:

-  :ned:`FlowMeasurementStarter`: classifies packets to enter packet flows and
   starts timing measurements or packet event collection.
-  :ned:`FlowMeasurementRecorder`: completes timing measurements, collects statistics,
   and makes packets leave the packet flow.
-  :ned:`MeasurementLayer`: can be added to network nodes and network interfaces
   for optional packet flow measurements.

.. _ug:sec:results:recording-pcap-traces:

Recording PCAP Traces
---------------------

The easiest way to understand the behavior of a communication network on the
network level is to look at the actual packets that are exchanged. INET supports
recording such packet traces in the widely used PCAP and its more recent sibling,
the PCAPng file formats. These file formats allow analyzing the network traffic
using the well-known Wireshark packet analyzer.

All network nodes and network interfaces support the recording of incoming and
outgoing traffic into PCAP files via optional :ned:`PcapRecorder` modules. By
default, this module records all packets emitted with configured signals from
its parent module. For example, a recorder module put in the network interface
module records all traffic specific to that network interface, and similarly a
recorder put in the network node records all traffic from the given node. It's
also possible to put a PCAP recorder module on the network level to produce a
PCAP file that contains all network traffic. If the traffic of more than one
network interface is recorded into a single file, then the newer PCAPng file
format must be used to also record the data of the corresponding network interfaces.

Recording PCAP traces also support using packet filters, which in turn allows
one to produce multiple files for the same network interface containing different
kinds of traffic.

The :ned:`PcapRecorder` parameters select the observation and representation in
separate steps. :par:`moduleNamePatterns` selects the sibling modules whose
signals are observed. :par:`dumpProtocols` is a space-separated list of INET
protocol names and selects the protocol representation written to the capture;
the recorder dissects a packet until it finds a listed protocol and writes that
portion with the corresponding PCAP link type. Its default is
``"ethernetmac ppp ieee80211mac"``. For example, selecting ``ipv4`` can produce
an IP-level capture from packets observed at an interface. This parameter does
not select the observation point and is not a general content filter; use
:par:`packetFilter` to select packets by headers or fields.

PCAP stores bytes, so every checksum or FCS included in the selected
representation must have an actual serializable value. Many protocol and link
layer modules default to ``"declared"`` checksum or FCS handling: the model
records whether the field is correct without computing its numeric value. Set
the applicable :par:`checksumMode` and :par:`fcsMode` parameters to
``"computed"`` when the capture includes those fields. These modes perform
protocol work and verification rather than merely changing recorder output, so
they are part of the simulation configuration and should be held constant when
comparing runs.

For a frame that a radio receives, the recorder writes the copy of the
receiver, with the receptionEnded signal of the radio. If the receiver could not
decode the frame, the copy has the bit error flag. :par:`dumpBadFrames` selects
whether the file contains such frames; the default is ``true``. A PCAPng record
of a frame with bit errors has the "CRC error" flag, so Wireshark shows the
frames that the receiver could not decode. Packet-level radios do not change
the bytes of such a frame, so its FCS in the file is still correct.

By default, an IEEE 802.11 record holds only the MAC frame (link type 105). The
helper ``inet::physicallayer::Ieee80211RadiotapPcapRecorderHelper`` writes
Radiotap records (link type 127) instead, the format of Linux monitor mode:

.. code-block:: ini

   *.host1.pcapRecorder[0].helpers = "inet::physicallayer::Ieee80211RadiotapPcapRecorderHelper"

The Radiotap header of a record has these physical layer values of the frame:

- the data rate of a legacy mode, or the MCS, the bandwidth and the guard
  interval of an HT mode, or the MCS, the number of spatial streams, the
  bandwidth and the guard interval of a VHT mode;
- the center frequency of the channel;
- the transmit power of a sent frame;
- the signal power and the start time of a received frame, and the noise. The
  noise is the signal power divided by the minimum SNIR of the reception, so it
  includes the interference. INET has no TSF timer, so the start time is the
  simulation time of the first bit, in microseconds;
- the FCS flags: "FCS at end", and "bad FCS" for a frame with bit errors.

The Radiotap header exists only in the file, not in the simulated frame, and it
does not change the simulation. A PCAP file holds the records of only one link
type, and the recorder stops with an error at the first record of a second link
type. To record a node that has Ethernet and Wi-Fi interfaces into one file
with Radiotap records, use the PCAPng format, or use one recorder for each
interface.

.. _ug:sec:results:recording-routing-tables:

Recording Routing Tables
------------------------

Understanding the behavior of routing protocols, especially with respect to the
dynamic state of all routing tables, in a complex communication network is a
difficult thing to do. In order to ease this task, INET provides a special
:ned:`RoutingTableRecorder` module which is capable of recording all network
interface data along with all routes in all routing tables into a single log
file.

The log file uses a simple text-based format. It contains one line for each
network interface added, changed, or deleted, and it also contains one line
for each route added, changed, or deleted in all network nodes. The resulting
log file can be used, for example, to verify that certain changes don't affect
the way routes are discovered in the network.

.. _ug:sec:results:eventlog-recording:

.. Eventlog Recording
   ------------------

   TODO: how to record packet content

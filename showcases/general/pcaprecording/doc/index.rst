PCAP Recording
==============

Goals
-----

INET has support for recording PCAP traces from simulations. The
recording process produces PCAP files that are similar to real-world
PCAP traces, so one can use the same tools and techniques for analyzing
simulated traffic as used on real traffic, such as Wireshark and
tcpdump. Knowledge of PCAP can be reused in the context of simulations.

This showcase contains an example simulation, which generates and
records PCAP traces of TCP, UDP, and ICMP traffic, using various
physical layer protocols like Ethernet and 802.11.

| INET version: ``4.0``
| Source files location: `inet/showcases/general/pcaprecording <https://github.com/inet-framework/inet/tree/master/showcases/general/pcaprecording>`__

The model
---------

To record PCAP traces in a node, a :ned:`PcapRecorder` module
needs to be included in it. Pcap recorder modules can be easily included
in hosts and routers by specifying their :par:`numPcapRecorders` parameter
(available in modules that extend :ned:`LinkLayerNodeBase`, such as
:ned:`StandardHost` and derivatives, and router modules.)

The PCAP recorder module records packets sent to and from modules that
are in the same host as the PCAP recorder module. By default, it records
L2 (link layer) frames (frames going in and out of the L2 layer.) It can
also be set to record L3 frames. It writes traces in a PCAP file, which
has to be specified by the :par:`pcapFile` parameter. This parameter acts
as the main switch for recording, thus specifying this parameter enables
packet capture. The PCAP recorder writes traces in the original PCAP
format, not the next generation one (PcapNg.) There can be packets with
only one link-layer header type in the PCAP file (this is a limitation
of the original PCAP file format.) The PCAP file's link-layer header
type needs to be set with the :par:`pcapNetwork` parameter, so that PCAP
programs interpret the traces correctly. Some widely used type codes are
the following:

-  Ethernet: 1
-  802.11: 105
-  PPP: 204

The complete list of link-layer header type codes can be found
`here <http://www.tcpdump.org/linktypes.html>`__.

The :par:`moduleNamePatterns` parameter specifies which modules' traffic
should be recorded. It takes a space separated list of module names. For
selecting a module vector, ``[*]`` can be used. The recorded modules are
on the same level in the hierarchy as the PCAP recorder module. The
default value for the :par:`moduleNamePatterns` parameter is
``wlan[*] eth[*] ppp[*]``, so it records the most commonly
present L2 interfaces. The :par:`dumpProtocols` parameter is a filter and
selects which protocols to include in the capture. It matches packets
which are of the specified protocol type at the level of capture, but
not the protocol type of encapsulated packets. The parameter takes
protocol names registered in INET (see
`/inet/src/inet/common/Protocol.cc <https://github.com/inet-framework/inet/blob/master/src/inet/common/Protocol.cc>`__)
The parameter's default value is ``"ethernetmac ppp ieee80211mac"``.

By default, the PCAP recorder module records L2 frames, but setting the
``moduleNamePatterns`` to ``ipv4``, for example, lets one record L3
frames (note that the parameter's value is lowercase because it refers
to the actual ``ipv4`` module in the host, not the module type.)
To record IP frames, the :par:`pcapNetwork` parameter also needs to
be set to the proper link-layer header type (101 - raw IP), and the
:par:`dumpProtocols` parameter to ``ipv4`` (see the configuration section
below.)

When a node connects to the network via just one kind of interface,
specifying the link-layer header type is sufficient for capturing a
proper trace. However, if there are multiple kinds of interfaces the
node connects with, the set of captured interfaces or physical layer
protocols should be narrowed to the ones with the link-layer header type
specified by the :par:`pcapNetwork` parameter. It is needed because traffic
for all interfaces is included in the trace by default. Multiple PCAP
recorder modules need to be included in the network to record packets
with different link-layer headers. One PCAP recorder module should only
record traces with one link-layer header type because the packets with
the other header types would not be recognized by PCAP programs.

Additionally, there are two packet filtering parameters. The
:par:`packetFilter` parameter can filter for the packet class and its
properties, most commonly the packet's name. The :par:`packetDataFilter`
parameter can filter for the packet's contents, i.e. the contained
chunks. Specifically, it can filter for class names of protocol headers,
and their fields. The expression given as the parameter's value is
evaluated for all chunks, and the set of recorded packets are narrowed
to the ones that match the expression. The default value of both
parameters is ``*``, thus no packets are filtered. See the following
sections for examples.

To summarize: the :par:`moduleNamePatterns` parameter specifies which
modules' outputs should be captured. The :par:`pcapNetwork` parameter sets
the link-layer header type according to the captured module outputs, so
PCAP programs can interpret the PCAP file correctly. The
:par:`dumpProtocols` parameter can narrow the set of recorded protocols at
the level of capture. The :par:`packetFilter` and :par:`packetDataFilter`
parameters can further narrow the set of captured packets.

The configuration
~~~~~~~~~~~~~~~~~

The example simulation for this showcase contains wired hosts, wireless
hosts, and routers. The hosts are configured to generate TCP, UDP and
ICMP traffic. The hosts connect to routers via Ethernet; the connection
between the routers is PPP. The wireless hosts communicate via 802.11.
The simulation can be run by choosing the ``PcapRecording``
configuration from the ini file. The simulation uses the following
network:

.. image:: media/network2.png
   :width: 100%

The network contains two ``adhocHost``\ s named ``host1`` and ``host2``,
and two :ned:`StandardHost`\ s named ``ethHost1`` and ``ethHost2``. There
are two :ned:`Router` modules (``router1`` and ``router2``), which are
connected by PPP. Each wired host is connected to one of
the routers via Ethernet. The network also contains an
:ned:`Ipv4NetworkConfigurator`, an :ned:`Ieee80211ScalarRadioMedium`, and an
:ned:`IntegratedMultiVisualizer` module.

Traffic generation is set up the following way: ``host1`` is configured
to send a UDP stream to ``host2`` (via 802.11), ``ethHost1`` is
configured to open a TCP connection to ``ethHost2``, and send a 1Mbyte
file (via Ethernet). Additionally, ``ethHost1`` is configured to ping
``ethHost2``.

.. todo::

   <!-- V1

   There are :ned:`PcapRecorder` modules added to `host1`, `host2`, `ethHost1`,
   `router1`, and `router2`. The keys in the ini file pertaining to PCAP recording
   configuration are the following:

   .. literalinclude:: ../omnetpp.ini
      :language: ini
      :start-at: host1.numPcapRecorders
      :end-before: verbose

   We configure `host1`'s PCAP recorder to use the 802.11 link-layer headers,
   and `ethHost1`'s PCAP recorder to use Ethernet link-layer headers.
   There are two PCAP recorder modules in `router1`, with one of them recording
   Ethernet traffic on `eth0` and the other PPP traffic on `ppp0`.
   Additionally, there is a PCAP recorder in `ethHost2`, which is set to record
   traffic of the `ipv4` module. -->

   <!-- V2 -->

We set up multiple PCAP recorder modules in various hosts in the
network:

In ``host1``, we'll record 802.11 traffic on the ``wlan0`` interface. We
configure ``host1``'s PCAP recorder to use the 802.11 link-layer
headers:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: host1.numPcapRecorders
   :end-at: /host1.pcap

In ``host2``, we'll record only the ARP packets from the 802.11 traffic
on ``wlan0``. The link-layer header type is set to 802.11, and the
:par:`packetDataFilter` is set to record only to packets of ``ArpPacket``
class:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: host2.numPcapRecorders
   :end-at: packetDataFilter

The :par:`packetDataFilter` parameter's value ``*ArpPacket`` matches the
``inet::ArpPacket`` class. There is an ARP request packet open in the
packet inspector on the following image:

.. image:: media/arppacketclass.png
   :width: 60%
   :align: center

In ``ethHost1``, we'll record Ethernet traffic on the ``eth0``
interface. We set the link-layer header type to ethernet:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: ethHost1.numPcapRecorders
   :end-at: /ethHost1.pcap

There are two PCAP recorder modules in ``router1``, with one of them
recording Ethernet traffic on ``eth0`` and the other PPP traffic on
``ppp0``. The :par:`moduleNamePatters` parameter needs to be set for both
PCAP recorder modules, because ``router1`` has two interfaces.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: router1.numPcapRecorders
   :end-before: router2.numPcapRecorders

In ``router2``, we'll record only packets carrying TCP data on the
``eth0`` interface. ``router2`` has two interfaces, so the
:par:`moduleNamePatters` parameter needs to be set. The link-layer header
type is set to ethernet, and the packet data filter is set to match
packets containing an ``Ipv4Header``, and where the ``totalLengthField``
field's value is 576 (the size of TCP data packets with IP
encapsulation):

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: router2.numPcapRecorders
   :end-at: totalLengthField

In ``ethHost2``, we'll record traffic of the ``ipv4`` module. The
:par:`dumpProtocols` parameter's default is
``ethernetmac ppp ieee80211mac``, so it has to be set to ``ipv4``.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: ethHost2.numPcapRecorders
   :end-at: dumpProtocols

By default, modules like :ned:`Ipv4` and :ned:`EthernetInterface` don't
compute CRC and FCS frames, but assumes they are correct ("declared
correct" mode.) To include the CRC and FCS values in the
capture file, L2 and L3 modules need to be set to compute CRC and FCS:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: crcMode
   :end-at: fcsMode

Note that these settings are required, otherwise an error is returned.

The :par:`alwaysFlush` parameter controls whether to write packets to the
PCAP file as they are recorded or after the simulation has concluded.
It is ``false`` by default, but it's set to ``true`` in all PCAP
recorders to make sure there are recorded packets even if the simulation
crashes:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: alwaysFlush
   :end-at: alwaysFlush

Results
-------

The following video shows the traffic in the network:

.. video:: media/pcap1.mp4
   :width: 100%

   <!--internal video recording, playback speed 1, no animation speed, run until first sendTimer (t=0.002), step, stop at about 10.5 seconds simulation time-->

The following images show the same packets viewed in Qtenv's packet mode
inspector panel and the PCAP trace opened with Wireshark. Both
display the same data about the same packet (with the same data,
sequence number, CRC, etc. Click to zoom.)

TCP data, in ``ethHost1`` (sent from ``ethHost1`` to ``ethHost2``):

.. image:: media/ethHost9.png
   :width: 100%

Ping request, in ``router1``'s eth interface (sent from ``ethHost1`` to
``router1``):

.. image:: media/routerEth2_2.png
   :width: 100%

TCP ACK, in ``router1``'s ppp interface (sent from ``ethHost1`` to
``ethHost2``):

.. image:: media/routerPPP3.png
   :width: 100%

UDP data packet, in ``host1``'s wlan interface (sent from ``host1`` to
``host2``):

.. image:: media/wifi5.png
   :width: 100%

The following screenshot shows ``ethHost1.pcap`` opened with TCPDump:

.. image:: media/tcpdump.png
   :width: 100%

TCP data packet, in ``ethHost2``, recorded at the IPv4 module (sent from
``ethHost1`` to ``ethHost2``):

.. image:: media/rawip.png
   :width: 100%

The following images are the PCAP traces from ``host2`` and ``router2``,
where the set of recorded packets were narrowed with
:par:`packetDataFilter`.

The ARP packets from ``host2``:

.. image:: media/arp.png
   :width: 100%

The TCP data packets from ``router2``:

.. image:: media/tcpdata.png
   :width: 100%

Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`PcapRecordingShowcase.ned <../PcapRecordingShowcase.ned>`

Discussion
----------

Use `this
page <https://github.com/inet-framework/inet-showcases/issues/22>`__ in
the GitHub issue tracker for commenting on this showcase.

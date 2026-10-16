=============
Model Catalog
=============

The following model components (protocols, applications and other models) are available for the INET Framework:

.. raw:: html

   <table class="model-catalog">
   <thead>
     <tr>
       <th>Protocol</th>
       <th>Project</th>
       <th>License<sup>1</sup></th>
       <th>Comments<sup>2</sup></th>
     </tr>
   </thead>
   <tbody>
     <tr>
       <td colspan='4' class='section'>Application</td>
     </tr>
     <tr>
       <td>CBR/VBR</td>
       <td><a href='http://github.com/inet-framework/inet'>INET</a></td>
       <td>LGPL</td>
       <td>Traffic generator modules in the <code>inet.applications.*</code> packages (like <code>UDPBasicApp</code>, <code>TCPBasicClientApp</code>, <code>IPTrafGen</code>, <code>EtherAppCli</code>, EtherAppSrv, etc.) can generate both constant bit-rate and variable bit-rate traffic, depending on the setting of the <code>packetInterval</code> or equivalent parameter (constant or random variable like <code>exponential(1s)</code>)</td>
     </tr>
     <tr>
       <td>HTTP</td>
       <td><a href='http://github.com/inet-framework/inet'>INET</a></td>
       <td>GPL</td>
       <td>HTTP traffic generation (merged from the <a href='http://code.google.com/p/omnet-httptools'>HttpTools</a> project).</td>
     </tr>
     <tr>
       <td>File Transfer</td>
       <td><a href='http://github.com/inet-framework/inet'>INET</a></td>
       <td>LGPL</td>
       <td>Modules in the <code>inet.applications.tcp</code> package, e.g. <code>TCPSessionApp</code> can be used to model file transfers.</td>
     </tr>
     <tr>
       <td>DHCP</td>
       <td><a href='http://github.com/inet-framework/inet'>INET</a></td>
       <td>LGPL</td>
       <td>DHCP protocol from <a href='http://github.com/jmaureir/DHCP/'>link</a></td>
     </tr>
     <tr>
       <td>Video (basic)</td>
       <td><a href='http://github.com/inet-framework/inet'>INET</a></td>
       <td>LGPL</td>
       <td>The <code>UDPVideoStreamCli</code> and <code>UDPVideoStreamSvr</code> modules in the <code>inet.applications.udp</code> package provide a simple video streaming model; video traffic is modeled as CBR stream.</td>
     </tr>
     <tr>
       <td>Video (detailed)</td>
       <td>n/a</td>
       <td>n/a</td>
       <td>The VideoInterface model in the OMNeT++ download area (<a href='http://www.omnetpp.org/omnetpp/doc_details/2128-videointerface'>link</a>), originally from <a href='http://trace.eas.asu.edu/tracesim/tracesim.html'>http://trace.eas.asu.edu/tracesim/tracesim.html</a> implements a trace-based model of video traffic. The code needs to be revised and ported into the INET Framework.</td>
     </tr>
     <tr>
       <td>Voice (basic)</td>
       <td><a href='http://github.com/inet-framework/inet'>INET</a></td>
       <td>LGPL</td>
       <td>If modeling voice traffic as a CBR stream suffices, see the <em>CBR/VBR</em> entry in this table.</td>
     </tr>
     <tr>
       <td>Voice (detailed)</td>
       <td><a href='http://github.com/inet-framework/inet'>INET</a></td>
       <td>LGPL</td>
       <td>Can generate realistic VoIP packet streams thanks to the utilization of real audio data and an existing VoIP standard codec. Moreover, by applying ITU-T's perceptual evaluation of speech quality (PESQ) approach at the sink, the perceived quality of a transmitted VoIP stream can be determined. (from the <a href='https://github.com/inet-framework/voiptool'>VoIPTool</a> project)</td>
     </tr>
     <tr>
       <td>Peer-to-peer (Chord, Kademlia, Pastry, GIA, etc.)</td>
       <td><a href='http://www.oversim.org'>OverSim</a></td>
       <td>GPL</td>
       <td>OverSim is an open-source INET-based overlay and peer-to-peer network simulation framework. The simulator contains several models for structured (e.g. Chord, Kademlia, Pastry) and unstructured (e.g. GIA) P2P systems and overlay protocols. The project has its own mailing list, bug tracker, documentation, etc., hosted at the web site.</td>
     </tr>
     <tr>
       <td>BitTorrent</td>
       <td><a href='https://github.com/pedromanoel/EbitSim'>EbitSim</a></td>
       <td>CCPL</td>
       <td>BitTorrent simulation with the following features: multiple concurrent swarm, multiple trackers, timeslice processing model</td>
     </tr>
     <tr>
       <td>P2P Video Streaming</td>
       <td><a href='http://ece.modares.ac.ir/~denacast/'>DenaCast</a></td>
       <td>GPL</td>
       <td>DenaCast is an open-source peer-to-peer video streaming framework based on the OverSim framework.</td>
     </tr>
     <tr>
       <td>Network Attack</td>
       <td><a href='http://nesg.ugr.es/index.php/en/results'>NETA</a></td>
       <td>GPL</td>
       <td>A framework devised to simulate attacks in heterogeneous networks. NETA is aimed to be a useful tool in the network security field.</td>
     </tr>
     <tr>
       <td colspan='4' class='section'>Transport</td>
     </tr>
     <tr>
       <td>TCP (INET)</td>
       <td><a href='http://github.com/inet-framework/inet'>INET</a></td>
       <td>LGPL</td>
       <td>INET's own TCP implementation. Contains TCP Tahoe, Reno and NewReno (new algorithms can be added by implementing and registering a C++ class interface); supports SACK. Data transfer simulation modes: byte count, C++ packet object (TODO: byte array). Supports both IPv4 and IPv6. Support for application-level flow control (simulation of READ syscalls) is in work as of Aug 2010.</td>
     </tr>
     <tr>
       <td>TCP (lwIP)</td>
       <td><a href='http://github.com/inet-framework/inet'>INET</a></td>
       <td>LGPL+BSD</td>
       <td>OMNeT++ module that wraps <a href='http://www.sics.se/~adam/lwip'>lwIP</a>. Less overhead (less frequent timer calls) than the NSC-based TCP implementation. Data transfer simulation modes: byte count, C++ packet object (TODO: byte array). Supports both IPv4 and IPv6. Not yet released, snapshot available from INET repo branch; support for application-level flow control (simulation of READ syscalls) is in work as of Aug 2010.</td>
     </tr>
     <tr>
       <td>TCP (NSC)</td>
       <td><a href='http://github.com/inet-framework/inet'>INET</a></td>
       <td>LGPL+GPL</td>
       <td>OMNeT++ module that wraps Sam Jensen's <a href='http://research.wand.net.nz/software/nsc.php'>Network Simulation Cradle</a> (NSC). NSC needs to be downloaded and installed into INET separately. Contains Linux 2.6, OpenBSD and FreeBSD stacks. Data transfer simulation modes: byte count (TODO: byte array, C++ packet object). Supports both IPv4 and IPv6. No support for application-level flow control (simulation of READ syscalls).</td>
     </tr>
     <tr>
       <td>UDP</td>
       <td><a href='http://github.com/inet-framework/inet'>INET</a></td>
       <td>LGPL</td>
       <td>Implements UDP.</td>
     </tr>
     <tr>
       <td>SCTP</td>
       <td><a href='http://github.com/inet-framework/inet'>INET</a></td>
       <td>LGPL</td>
       <td>Implements SCTP.</td>
     </tr>
     <tr>
       <td>RTP, RTCP</td>
       <td><a href='http://github.com/inet-framework/inet'>INET</a></td>
       <td>LGPL</td>
       <td>Extensible with profiles; currently contains the "RTP Profile for Audio and Video Conferences with Minimal Control" profile, and MPEG video payload. Needs to be reviewed and tested.</td>
     </tr>
     <tr>
       <td colspan='4' class='section'>Network</td>
     </tr>
     <tr>
       <td>IPv4</td>
       <td><a href='http://github.com/inet-framework/inet'>INET</a></td>
       <td>LGPL</td>
       <td>Implements IPv4. Supports autoconfigured static routing, manual static routing via config files, and routing protocols. Implementation consists of the <code>IPv4</code> and <code>RoutingTable</code> modules in the <code>inet.networklayer.ip</code> package; see also <code>FlatNetworkConfigurator</code>.</td>
     </tr>
     <tr>
       <td>ICMPv4</td>
       <td><a href='http://github.com/inet-framework/inet'>INET</a></td>
       <td>LGPL</td>
       <td>Implements ICMPv4.</td>
     </tr>
     <tr>
       <td>ARP</td>
       <td><a href='http://github.com/inet-framework/inet'>INET</a></td>
       <td>LGPL</td>
       <td>Implements ARP over IEEE 802 networks.</td>
     </tr>
     <tr>
       <td>IGMPv2</td>
       <td><a href='http://github.com/inet-framework/inet'>INET</a></td>
       <td>LGPL</td>
       <td>Implements IGMPv2 (RFC 2236). Limitations: no interoperability with IGMPv1 hosts.</td>
     </tr>
     <tr>
       <td>IGMPv3</td>
       <td><a href='http://github.com/inet-framework/inet'>INET</a></td>
       <td>LGPL</td>
       <td>IGMPv3 implementation based on the <a href='http://nes.fit.vutbr.cz/ansa/'>ANSA</a> project.</td>
     </tr>
     <tr>
       <td>IPv6</td>
       <td><a href='http://github.com/inet-framework/inet'>INET</a></td>
       <td>LGPL</td>
       <td>TODO</td>
     </tr>
     <tr>
       <td>ICMPv6</td>
       <td><a href='http://github.com/inet-framework/inet'>INET</a></td>
       <td>LGPL</td>
       <td>TODO</td>
     </tr>
     <tr>
       <td>MIPv6</td>
       <td><a href='http://github.com/inet-framework/inet'>INET</a></td>
       <td>LGPL</td>
       <td>(from the <a href='http://www.kn.e-technik.tu-dortmund.de/de/forschung/ausstattung/xmipv6.html'>xMIPv6</a> project)</td>
     </tr>
     <tr>
       <td>MCoA</td>
       <td><a href='http://mcoa.dei.uc.pt/'>MCoA</a></td>
       <td>LGPL</td>
       <td>Multiple Care of Address Registration for Mobile IPv6</td>
     </tr>
     <tr>
       <td>HIP</td>
       <td><a href='http://www.ict-optimix.eu/index.php/HIPSim'>HIPSim</a></td>
       <td>LGPL</td>
       <td>Implementation of the Host Identity Protocol for INET and xMIPv6.</td>
     </tr>
     <tr>
       <td colspan='4' class='section'>Routing</td>
     </tr>
     <tr>
       <td>link-state routing</td>
       <td><a href='http://github.com/inet-framework/inet'>INET</a></td>
       <td>LGPL</td>
       <td>The <code>LinkStateRouting</code> module implements a minimalistic link state routing protocol. Apart from the basic topology information, the current link usage is distributed to all participants in the network (by means of flooding). Was primarily implemented as an aid for MPLS simulations. Needs more review and testing.</td>
     </tr>
     <tr>
       <td>OSPFv2 (1)</td>
       <td><a href='http://github.com/inet-framework/inet'>INET</a></td>
       <td>LGPL</td>
       <td>The INET implementation of the OSPFv2 protocol. Needs more documentation, review and testing.</td>
     </tr>
     <tr>
       <td>OSPF (2)</td>
       <td><a href='http://github.com/inet-framework/inet-quagga'>INET/Quagga</a></td>
       <td>LGPL</td>
       <td>OMNeT++ modules that wrap the code of the <a href='http://www.quagga.net'>Quagga routing suite</a>. Supports OSPFv2. An INET Quagga router can be configured with the same config files as a real Quagga daemon. Needs more review and testing.</td>
     </tr>
     <tr>
       <td>BGPv4 (1)</td>
       <td><a href='http://github.com/inet-framework/inet'>INET</a></td>
       <td>LGPL</td>
       <td>The INET implementation of the BGPv4 protocol. Needs review and testing.</td>
     </tr>
     <tr>
       <td>BGP (2)</td>
       <td><a href='http://github.com/inet-framework/inet-quagga'>INET/Quagga</a></td>
       <td>LGPL</td>
       <td>OMNeT++ modules that wrap the code of the <a href='http://www.quagga.net'>Quagga routing suite</a>. Supports BGPv4. An INET Quagga router can be configured with the same config files as a real Quagga daemon. Needs more review and testing.</td>
     </tr>
     <tr>
       <td>RIP (1)</td>
       <td><a href='http://github.com/inet-framework/inet'>INET</a></td>
       <td>LGPL</td>
       <td>The INET implementation of the RIP protocol. Needs more review and testing.</td>
     </tr>
     <tr>
       <td>RIP (2)</td>
       <td><a href='http://github.com/inet-framework/inet-quagga'>INET/Quagga</a></td>
       <td>LGPL</td>
       <td>OMNeT++ modules that wrap the code of the <a href='http://www.quagga.net'>Quagga routing suite</a>. Supports RIP. An INET Quagga router can be configured with the same config files as a real Quagga daemon. Needs more review and testing.</td>
     </tr>
     <tr>
       <td>PIM</td>
       <td><a href='http://github.com/inet-framework/inet'>INET</a></td>
       <td>LGPL</td>
       <td>PIM implementation based on the <a href='http://nes.fit.vutbr.cz/ansa/'>ANSA</a> project.</td>
     </tr>
     <tr>
       <td colspan='4' class='section'>MANET Routing</td>
     </tr>
     <tr>
       <td>AODV (1)</td>
       <td><a href='http://github.com/inet-framework/inet'>INET</a></td>
       <td>LGPL</td>
       <td>RFC 3561 protocol implementation for INET.</td>
     </tr>
     <tr>
       <td>AODV (2)</td>
       <td><a href='http://github.com/inet-framework/inet'>INET</a></td>
       <td>GPL</td>
       <td>This is a port of the <a href='http://sourceforge.net/projects/aodvuu'>AODV-UU</a> implementation into INET (from the <a href='http://github.com/inetmanet/inetmanet'>INETMANET</a> project).</td>
     </tr>
     <tr>
       <td>DYMO (1)</td>
       <td><a href='http://github.com/inet-framework/inet'>INET</a></td>
       <td>LGPL</td>
       <td>Implementation based on draft-ietf-manet-dymo-24</td>
     </tr>
     <tr>
       <td>DYMO (2)</td>
       <td><a href='http://github.com/inet-framework/inet'>INET</a></td>
       <td>GPL</td>
       <td>Implements draft-ietf-manet-dymo-05 (from the <a href='http://github.com/inetmanet/inetmanet'>INETMANET</a> project, based on DYMO-UM).</td>
     </tr>
     <tr>
       <td>DYMO (3)</td>
       <td><a href='http://github.com/inet-framework/inet'>INET</a></td>
       <td>GPL</td>
       <td>Imported from <a href='http://www7.informatik.uni-erlangen.de/~sommer/omnet/dymo/'>DYMO-FAU</a>. Implements draft-ietf-manet-dymo-10 (from the <a href='http://github.com/inetmanet/inetmanet'>INETMANET</a> project).</td>
     </tr>
     <tr>
       <td>GPSR</td>
       <td><a href='http://github.com/inet-framework/inet'>INET</a></td>
       <td>LGPL</td>
       <td>Greedy Perimeter Stateless Routing for Wireless Networks. Supports
     <p>both GG and RNG planarization algorithms.
     </p></td>
     </tr>
     <tr>
       <td>DSDV</td>
       <td><a href='http://github.com/inet-framework/inet'>INET</a></td>
       <td>BSD?</td>
       <td>This is a port of <a href='http://dsdv.8rf.com'>DSDV</a> (from the <a href='http://github.com/inetmanet/inetmanet'>INETMANET</a> project).</td>
     </tr>
     <tr>
       <td>DSR</td>
       <td><a href='http://github.com/inet-framework/inet'>INET</a></td>
       <td>GPL</td>
       <td>Implements draft-ietf-manet-dsr-10.txt (from the <a href='http://github.com/inetmanet/inetmanet'>INETMANET</a> project).</td>
     </tr>
     <tr>
       <td>OLSR</td>
       <td><a href='http://github.com/inet-framework/inet'>INET</a></td>
       <td>GPL</td>
       <td>This is a port of <a href='http://masimum.dif.um.es/?Software:UM-OLSR'>UM_OLSR</a>. Complies with RFC 3626 and supports all core functionalities of OLSR plus the link-layer feedback option. (from the <a href='http://github.com/inetmanet/inetmanet'>INETMANET</a> project)</td>
     </tr>
     <tr>
       <td colspan='4' class='section'>MPLS</td>
     </tr>
     <tr>
       <td>MPLS</td>
       <td><a href='http://github.com/inet-framework/inet'>INET</a></td>
       <td>LGPL</td>
       <td>MPLS simulations in INET are made possible by a set of modules, including <code>MPLS</code>, <code>LIBTable</code> and <code>TED</code>. Node failure and recovery are currently not supported. Needs more review and testing.</td>
     </tr>
     <tr>
       <td>LDP</td>
       <td><a href='http://github.com/inet-framework/inet'>INET</a></td>
       <td>LGPL</td>
       <td>The <code>LDP</code> module in INET implements the LDP control protocol for MPLS networks. This is the base LDP (RFC 5036), not CR-LDP; however, note that as of February 2003 the IETF MPLS working group deprecated CR-LDP and decided to focus purely on RSVP-TE. Node failure and recovery are currently not supported; needs more review and testing.</td>
     </tr>
     <tr>
       <td>RSVP-TE</td>
       <td><a href='http://github.com/inet-framework/inet'>INET</a></td>
       <td>LGPL</td>
       <td>The <code>RSVP</code> module in INET implements the RSVP-TE control protocol for MPLS networks. Node failure and recovery are currently not supported. Needs more review and testing.</td>
     </tr>
     <tr>
       <td colspan='4' class='section'>Wired</td>
     </tr>
     <tr>
       <td>PPP</td>
       <td><a href='http://github.com/inet-framework/inet'>INET</a></td>
       <td>LGPL</td>
       <td>The <code>PPP</code> module in INET can be used for simulating PPP links. The model only performs simple encapsulation/decapsulation and queuing; the link configuration and maintenance part of the PPP protocol are not covered.</td>
     </tr>
     <tr>
       <td>Ethernet</td>
       <td><a href='http://github.com/inet-framework/inet'>INET</a></td>
       <td>LGPL</td>
       <td>INET contains a set of modules for simulating Ethernet networks. Supports classic Ethernet (10Mbps), Fast Ethernet (100Mbps), Gigabit Ethernet (1000Mbps). There are two MAC implementations: one with the full CSMA/CD protocol, and a simplified one without CSMA/CD that can be used for full-duplex links. Supports raw Ethernet, Ethernet-II and Ethernet SNAP frames. There is a hub and a switch model; the switch relay unit has several flavours which differ in the performance models. Ethernet MAC currently expects the data rate in a module parameter instead of the link <code>datarate</code> parameter; this is currently being changed (new code is available in an INET repo branch).</td>
     </tr>
     <tr>
       <td>STP</td>
       <td><a href='http://github.com/inet-framework/inet'>INET</a></td>
       <td>LGPL</td>
       <td>Spanning Tree Protocol</td>
     </tr>
     <tr>
       <td>RSTP</td>
       <td><a href='http://github.com/inet-framework/inet'>INET</a></td>
       <td>LGPL</td>
       <td>Rapid Spanning Tree Protocol</td>
     </tr>
     <tr>
       <td>TTE</td>
       <td><a href='http://tte4inet.realmv6.org/'>CoRE4INET</a></td>
       <td>LGPL</td>
       <td>CoRE4INET contains a Time-Triggered Ethernet protocol implementation.</td>
     </tr>
     <tr>
       <td>802.1avb</td>
       <td><a href='http://tte4inet.realmv6.org/'>CoRE4INET</a></td>
       <td>LGPL</td>
       <td>CoRE4INET is an extension to the INET-Framework for the event-based simulation of real-time Ethernet.</td>
     </tr>
     <tr>
       <td>EPON</td>
       <td><a href='http://sourceforge.net/projects/omneteponmodule/'>EPON</a></td>
       <td>LGPL</td>
       <td>A basic implementation of (1G) Ethernet Passive Optical Network (EPON). OLT and ONU modules + MPCP protocol.</td>
     </tr>
     <tr>
       <td>TDM/WDM-PON</td>
       <td><a href='http://kyeongsoo.github.io/inet-hnrl/'>INET-HNRL</a></td>
       <td>LGPL</td>
       <td>Extension for optical, wireless, and their hybrid networks and protocols.</td>
     </tr>
     <tr>
       <td colspan='4' class='section'>Wireless</td>
     </tr>
     <tr>
       <td>802.11</td>
       <td><a href='http://github.com/inet-framework/inet'>INET</a></td>
       <td>LGPL</td>
       <td>A set of modules for 802.11 simulations in INET. Supports 802.11b and g, both ad-hoc and infrastructure mode. Unimplemented protocol features: fragmentation, power management, and polling (PCF). Model accuracy has been experimentally verified (<em>"On The Accuracy of IEEE 802.11g Wireless LAN Simulations Using OMNeT++"</em>, Michael Bredel and Martin Bergner, OMNeT++ 2009 Workshop; <a href='http://www.omnet-workshop.org/2009/docs/michael-bredel.pdf'>slides</a>)</td>
     </tr>
     <tr>
       <td>802.11p</td>
       <td><a href='http://veins.car2x.org/'>VEINS</a></td>
       <td>GPL</td>
       <td>Veins is an open source framework for running vehicular network simulations. The 802.11p implementation is based on MiXiM.</td>
     </tr>
     <tr>
       <td>802.1e</td>
       <td><a href='http://github.com/inet-framework/inet'>INET</a></td>
       <td>LGPL</td>
       <td>Quality of Service Extension to 802.11 (from the <a href='http://github.com/inetmanet/inetmanet'>INETMANET</a> project).</td>
     </tr>
     <tr>
       <td>802.15.4 (1)</td>
       <td><a href='http://github.com/inet-framework/inet'>INET</a></td>
       <td>LGPL</td>
       <td>UWB, NarrowBand and CSMA implementation inspired by the model implemented in MiXiM. INET 3.0+</td>
     </tr>
     <tr>
       <td>802.15.4 (2)</td>
       <td><a href='http://github.com/inetmanet/inetmanet'>INETMANET</a></td>
       <td>???</td>
       <td>Implementation of IEEE 802.15.4-2006 (low-rate wireless personal area network). <a href='http://www7.informatik.uni-erlangen.de/~fengchen/omnet/802154/'>Original project</a></td>
     </tr>
     <tr>
       <td>802.16e (WiMAX) (1)</td>
       <td><a href='http://github.com/inetmanet/inetmanet'>INETMANET</a></td>
       <td>???</td>
       <td>Incomplete implementation of Mobile Broadband Wireless Access System.</td>
     </tr>
     <tr>
       <td>802.16e (WiMAX) (2)</td>
       <td><a href='http://klub.com.pl/numbat/'>numbat</a>
     <p><a href='https://github.com/urban-1/numbat'>numbat-inet</a>
     </p></td>
       <td>???</td>
       <td>Implementation of Mobile Broadband Wireless Access System. INET compatible version is based on original numbat project. VLANs are used for CS Layer, but a pure Ethernet model shouldn't require many modifications. UGS, RTPS, BE flows are supported and a basic scheduler is provided.</td>
     </tr>
     <tr>
       <td>LTE (User-Plane)</td>
       <td><a href='http://www.simulte.com/'>SimuLTE</a></td>
       <td>LGPL</td>
       <td>User Plane LTE Simulator. PHY: 2x2 MU-MIMO, S-MUX, transmit diversity using SINR curves; User Terminals: Mobility, Battery consumption, Interference, all types of traffic; Built-in applications: VoIP, gaming, VoD, web; E-NodeB: Macro, micro, pico eNodeBs,Inter-eNB Coordination through X2 interface, Relay nodes, Distributed antennas,SU and MU-MIMO, Scheduling algorithms, Max C/I, Proportional Fair, Round Robin; MAC: Buffering, PDU concatenation, CQI reception, transport format selection and resource allocation, Coding designed to facilitate cross-layer analysis; RLC: UM and AM segmentation and reassembly retransmissions</td>
     </tr>
     <tr>
       <td>LTE (Control-Plane)</td>
       <td><a href='https://github.com/4gsim/4Gsim'>4GSim</a> <a href='https://github.com/aarizaq/inetmanet-2.0/tree/inetmanet-2.2/'>INETMANET</a></td>
       <td>LGPL</td>
       <td>Control Plane LTE Protocols.</td>
     </tr>
     <tr>
       <td colspan='4' class='section'>Mobility / Environment</td>
     </tr>
     <tr>
       <td>Satellite Mobility</td>
       <td><a href='http://www-os3.kn.e-technik.tu-dortmund.de/'>OS³</a></td>
       <td>LGPL</td>
       <td>Allows to evaluate satellite orbits and communication performance pertaining to SNR, bit error rate, packet loss, round trip time, jitter, reachability, and other measures.</td>
     </tr>
     <tr>
       <td>Vehicular Mobility</td>
       <td><a href='http://veins.car2x.org/'>VEINS</a></td>
       <td>GPL</td>
       <td>Veins is an open source framework for running vehicular network simulations. It is based on two well-established simulators: OMNeT++, an event-based network simulator, and SUMO, a road traffic simulator. It extends these to offer a comprehensive suite of models for IVC simulation.</td>
     </tr>
     <tr>
       <td>Various Mobility Models</td>
       <td><a href='http://github.com/inet-framework/inet'>INET</a></td>
       <td>LGPL</td>
       <td>INET contains various mobility models including TraCI (Veins).</td>
     </tr>
   </tbody>
   </table>

Notes:

1. License: should ideally be LGPL; note that GPL is generally incompatible with the OMNeT++ license
2. Comments: describes implementation status, completeness, future plans, "todo" items, maintainers, validation status, etc.

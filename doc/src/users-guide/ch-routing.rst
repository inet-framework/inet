.. _ug:cha:routing:

Internet Routing
================

.. _ug:sec:routing:overview:

Overview
--------

INET Framework has models for several internet routing protocols,
including RIP, OSPF and BGP.

The easiest way to add routing to a network is to use the :ned:`Router`
NED type for routers. :ned:`Router` contains a conditional instance for
each of the above protocols. These submodules can be enabled by setting
the :par:`hasRip`, :par:`hasOspf` and/or :par:`hasBgp` parameters to
``true``.

Example:

.. code-block:: ini

   **.hasRip = true

There are also NED types called :ned:`RipRouter`, :ned:`OspfRouter`,
:ned:`BgpRouter`, which are all :ned:`Router`’s with appropriate routing
protocol enabled.

.. _ug:sec:routing:rip:

RIP
---

RIP (Routing Information Protocol) is a distance-vector routing protocol
which employs the hop count as a routing metric. RIP prevents routing
loops by implementing a limit on the number of hops allowed in a path
from source to destination. RIP uses the *split horizon with poison
reverse* technique to work around the “count-to-infinity” problem.

The :ned:`Rip` module implements distance vector routing as specified in
RFC 2453 (RIPv2) and RFC 2080 (RIPng). The behavior can be selected by
setting the :par:`mode` parameter to either ``"RIPv2"`` or
``"RIPng"``. Protocol configuration such as link metrics and
per-interface operation mode (such as whether RIP is enabled on the
interface, and whether to use split horizon) can be specified in XML
using the :par:`ripConfig` parameter.

The following example configures a :ned:`Router` module to use RIPv2:

.. code-block:: ini

   **.hasRip = true
   **.mode = "RIPv2"
   **.ripConfig = xmldoc("RIPConfig.xml")

The configuration file specifies the per interface parameters. Each
``<interface>`` element configures one or more interfaces; the
``hosts``, ``names``, ``towards``, ``among`` attributes
select the configured interfaces (in a similar way as with
:ned:`Ipv4NetworkConfigurator`). See the :doc:`ch-network-autoconfig` chapter
for further information.

Additional attributes:

-  ``metric``: metric assigned to the link, default value is 1. This
   value is added to the metric of a learned route, received on this
   interface. It must be an integer in the [1,15] interval.

-  ``mode``: mode of the interface.

The mode attribute can be one of the following:

-  ``NoRIP``: no RIP messages are sent or received on this
   interface.

-  ``NoSplitHorizon``: no split horizon filtering; send all routes
   to neighbors.

-  ``SplitHorizon``: do not sent routes whose next hop is the
   neighbor.

-  ``SplitHorizonPoisenedReverse`` (default): if the next hop is
   the neighbor, then set the metric of the route to infinity.

The following example sets the link metric between router ``R1`` and
``RB`` to 2, while all other links will have a metric of 1.

.. code-block:: xml

   <RIPConfig>
     <interface among="R1 RB" metric="2"/>
     <interface among="R? R?" metric="1"/>
   </RIPConfig>

.. _ug:sec:routing:ospf:

OSPF
----

OSPF (Open Shortest Path First) is a routing protocol for IP networks.
It uses a link state routing (LSR) algorithm and falls into the group of
interior gateway protocols (IGPs), operating within a single autonomous
system (AS).

:ned:`OspfRouter` is a :ned:`Router` with the OSPF protocol enabled.

The :ned:`Ospfv2` module implements OSPF protocol version 2. Areas and
routers can be configured using an XML file specified by the
:par:`ospfConfig` parameter. Various parameters for the network
interfaces can be specified also in the XML file or as a parameter of
the :ned:`Ospfv2` module.

.. code-block:: ini

   **.ospf.ospfConfig = xmldoc("ASConfig.xml")
   **.ospf.helloInterval = 12s
   **.ospf.retransmissionInterval = 6s

The ``<OSPFASConfig>`` root element may contain ``<Area>`` and
``<Router>`` elements with various attributes specifying the
parameters for the network interfaces. Most importantly ``<Area>``
contains ``<AddressRange>`` elements enumerating the network
addresses that should be advertized by the protocol. Also
``<Router>`` elements may contain data for configuring various
pont-to-point or broadcast interfaces.

.. code-block:: xml

   <?xml version="1.0"?>
   <OSPFASConfig xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="OSPF.xsd">
     <!-- Areas -->
     <Area id="0.0.0.0">
       <AddressRange address="H1" mask="H1" status="Advertise" />
       <AddressRange address="H2" mask="H2" status="Advertise" />
       <AddressRange address="R1>R2" mask="R1>R2" status="Advertise" />
       <AddressRange address="R2>R1" mask="R2>R1" status="Advertise" />
     </Area>
     <!-- Routers -->
     <Router name="R1" RFC1583Compatible="true">
       <BroadcastInterface ifName="eth0" areaID="0.0.0.0" interfaceOutputCost="1" routerPriority="1" />
       <PointToPointInterface ifName="eth1" areaID="0.0.0.0" interfaceOutputCost="2" />
     </Router>
     <Router name="R2" RFC1583Compatible="true">
       <PointToPointInterface ifName="eth0" areaID="0.0.0.0" interfaceOutputCost="2" />
       <BroadcastInterface ifName="eth1" areaID="0.0.0.0" interfaceOutputCost="1" routerPriority="2" />
     </Router>
   </OSPFASConfig>

.. _ug:sec:routing:bgp:

BGP
---

BGP (Border Gateway Protocol) is a standardized exterior gateway
protocol designed to exchange routing and reachability information among
autonomous systems (AS) on the Internet.

:ned:`BgpRouter` is a :ned:`Router` with the BGP protocol enabled.

The :ned:`Bgp` module implements BGP Version 4. The model implements RFC
4271, with some limitations. Autonomous Systems and rules can be
configured in an XML file that can be specified in the :par:`bgpConfig`
parameter.

.. code-block:: ini

   **.bgpConfig = xmldoc("BGPConfig.xml")

The configuration file may contain ``<TimerParams>``, ``<AS>`` and
``Session`` elements at the top level.

-  ``<TimerParams>``: allows specifying various timing parameters for
   the routers.

-  ``<AS>``: defines Autonomous Systems, routers and rules to be
   applied.

-  ``<Session>``: specifies open sessions between edge routers. It
   must contain exactly two ``<Router exterAddr="x.x.x.x"/>``
   elements.

.. code-block:: xml

   <BGPConfig xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
     xsi:schemaLocation="BGP.xsd">

     <TimerParams>
       <connectRetryTime> 120 </connectRetryTime>
       <holdTime> 180 </holdTime>
       <keepAliveTime> 60 </keepAliveTime>
       <startDelay> 15 </startDelay>
     </TimerParams>

     <AS id="60111">
       <Router interAddr="172.1.10.255"/> <!--Router A1-->
       <Router interAddr="172.1.20.255"/> <!--Router A2-->
     </AS>

     <AS id="60222">
       <Router interAddr="172.10.4.255"/> <!--Router B-->
     </AS>

     <AS id="60333">
       <Router interAddr="172.13.1.255"/> <!--Router C1-->
       <Router interAddr="172.13.2.255"/> <!--Router C2-->
       <Router interAddr="172.13.3.255"/> <!--Router C3-->
       <Router interAddr="172.13.4.255"/> <!--Router C4-->
       <DenyRouteOUT Address="172.10.8.0" Netmask="255.255.255.0"/>
       <DenyASOUT> 60111 </DenyASOUT>
     </AS>

     <Session id="1">
       <Router exterAddr="10.10.10.1" > </Router> <!--Router A1-->
       <Router exterAddr="10.10.10.2" > </Router> <!--Router C1-->
     </Session>

     <Session id="2">
       <Router exterAddr="10.10.20.1" > </Router> <!--Router A2-->
       <Router exterAddr="10.10.20.2" > </Router> <!--Router B-->
     </Session>

     <Session id="3">
       <Router exterAddr="10.10.30.1" > </Router> <!--Router B-->
       <Router exterAddr="10.10.30.2" > </Router> <!--Router C2-->
     </Session>
   </BGPConfig>

Inside ``<AS>`` elements various rules can be sepecified:

-  DenyRoute: deny route in IN and OUT traffic (``Address`` and
   ``Netmask`` attributes must be specified.)

-  DenyRouteIN : deny route in IN traffic (``Address`` and
   ``Netmask`` attributes must be specified.)

-  DenyRouteOUT: deny route in OUT traffic (``Address`` and
   ``Netmask`` attributes must be specified.)

-  DenyAS: deny routes learned by AS in IN and OUT traffic (AS id must
   be specified as the body of the element.)

-  DenyASIN : deny routes learned by AS in IN traffic (AS id must be
   specified as the body of the element.)

-  DenyASOUT: deny routes learned by AS in OUT traffic (AS id must be
   specified as the body of the element.)

/**
@mainpage Wireless Tutorial for the INET framework

In this tutorial we show you how to build wireless simulations in the INET
framework. It contains a series of simulation models numbered from 1 through 19.
The models are of increasing complexity -- they start from the basics and
introduce  new INET features and concepts related to wireless communication
networks.

This is an advanced tutorial, and it assumes that you are familiar with creating
and running simulations in @opp and  INET. If you are not yet, you can check out
the <a href="https://omnetpp.org/doc/omnetpp/tictoc-tutorial/"
target="_blank">TicToc Tutorial</a> to get started with using @opp. The <a
href="../../../doc/walkthrough/tutorial.html" target="_blank">ARP Tutorial</a>
is an introduction to INET and how to work with protocols.

If you need more information at any time, feel free to refer to the @opp and
INET documentation:

- 	<a href="https://omnetpp.org/doc/omnetpp/manual/usman.html" target="_blank">@opp User Manual</a>
-	<a href="https://omnetpp.org/doc/omnetpp/api/index.html" target="_blank">@opp API Reference</a>
- 	<a href="https://omnetpp.org/doc/inet/api-current/inet-manual-draft.pdf" target="_blank">INET Manual draft</a>
- 	<a href="https://omnetpp.org/doc/inet/api-current/neddoc/index.html" target="_blank">INET Reference</a>

The tutorial starts off with a basic simulation model at step 1, and gradually
makes it more complex and realistic in subsequent steps -- so you can learn
about various INET wireless features and what can be achieved with them.

In the tutorial, each step is a separate configuration in the same omnetpp.ini file,
and consecutive steps mostly share the same networks, defined in NED.

@section contents Contents

-	@ref step1
-	@ref step2
-	@ref step3
-	@ref step4
-	@ref step5
-	@ref step6
-	@ref step7
-	@ref step8
-	@ref step9
-	@ref step10
-	@ref step11

NEXT: @ref step1

*/--------------------------------------------------------------------------/**


@page step1 Step 1 - Two hosts communicating wirelessly
UP: @ref step1

In the first scenario, we set up two hosts, with one host sending a UDP data
stream wirelessly to the other. We don't care if the wireless
exchange is realistic or not, just want the hosts to transfer data between each
other. There are no collisions, and other physical effects -- like attenuation
and interference -- are ignored. The network topology is defined in the
.ned files -- in this case WirelessA.ned.

<img src="wireless-step1-v2.png">

WirelessA, the network defined in the above NED file, defines a playground of
the size 500x500 meters:

@dontinclude WirelessA.ned
@skip network WirelessA
@until @display

It contains two hosts spaced 400 meters apart:

@dontinclude WirelessA.ned
@skip hostA: <hostType>
@until @display("p=450,250");
@skipline }

In @opp, the <tt>StandardHost</tt> NED type is generally used to represent hosts.
However, later steps will use other NED types for hosts, so the submodule type
is parameterizable. (<tt>INetworkNode</tt> is the interface for host types.)

All wireless simulations in INET need a radio medium module. This module represents
the shared physical medium where communication takes place. It is responsible for
taking signal propagation, attenuation, interference, and other physical phenomena
into account.

INET is capable of modeling wireless communication at various levels of detail,
realized with different radio medium modules. In this step, we use
the simplest model, a variation of unit disc radio, which is implemented by
the <tt>IdealRadioMedium</tt> NED type.

@dontinclude WirelessA.ned
@skip radioMedium: <mediumType>
@until @display

In hosts, network interface cards are represented by NIC modules. Radio is part of
wireless NIC modules. There are various radio modules, and one must always
use one that is compatible with the medium module. In this step, hosts contain
<tt>IdealRadio</tt> as part of IdealWirelessNic.

@dontinclude omnetpp.ini
@skipline .host*.wlan[*].typename = "IdealWirelessNic"

The most important parameter of <tt>IdealRadio</tt> is <i>communication range</i>.
<tt>IdealRadio</tt> receives a transmission correctly within communication
range, unless there is an interfering transmission.

@note This might seem overly simplified because such radios and signal propagation do not work
like that in real life. However, it can be very useful in modeling scenarios where
details of radio propagation is not of interest.

In this model, the communication range is set to 500m.

@dontinclude omnetpp.ini
@skipline *.host*.wlan[*].radio.transmitter.maxCommunicationRange = 500m

Interference (in this case, loss of packets due to collision) is also
modeled by <tt>IdealRadio</tt>. In this step, interference is turned off,
resulting in pairwise independent duplex communication channels.

@dontinclude omnetpp.ini
@skipline *.host*.wlan[*].radio.receiver.ignoreInterference = true

The radio data rates are set to 1 Mbps.

@dontinclude omnetpp.ini
@skipline **.bitrate = 1Mbps

Hosts in the network need IP addresses. IP address assignment in this model
is handled by an instance of <tt>IPv4NetworkConfigurator</tt>. This module
also sets up static routing between the hosts.

@dontinclude WirelessA.ned
@skip configurator:
@until @display
@skipline }

The hosts have to know each other's MAC addresses to communicate, which is
taken care of by using <i>GlobalARP</i>:

@dontinclude omnetpp.ini
@skipline **.arpType = "GlobalARP"

In the model, host A generates UDP packets which are received by host B.
To this end, host A contains a UDPBasicApp module, which generates 1000-byte UDP
messages at random intervals with exponential distribution, the mean of which is 10ms.
Therefore the app is going to generate 100 kbyte/s (800 kbps) UDP traffic (not counting protocol overhead).

@dontinclude omnetpp.ini
@skip *.hostA.numUdpApps = 1
@until *.hostA.udpApp[0].sendInterval = exponential(10ms)

Host B contains a <tt>UDPSink</tt> application that just discards received packets.

@dontinclude omnetpp.ini
@skip *.hostB.numUdpApps = 1
@until *.hostB.udpApp[0].localPort = 5000

The model also contains a gauge to display the number of packets received by Host B:

@dontinclude WirelessA.ned
@skipline @figure
@skipline moduleName

Here is an animation showing the hosts communicating:

<img src="step1_v4.gif">

When the simulation concludes at t=25s, the throughput instrument indicates that
around 2400 packets were sent. A packet with overhead is 1028 bytes, which means
the transmission rate was around 800 kbps.

Sources: @ref omnetpp.ini, @ref WirelessA.ned

NEXT: @ref step2

*/--------------------------------------------------------------------------/**

@page step2 Step 2 - Setting up some animations

UP: @ref step1

We would like to visualize radio waves as they propage through space. For that
we can use the @opp Canvas API.

Note that the configuration for Step 2 -- <i>Wireless02</i> -- is created by
extending <i>Wireless01</i> in omnetpp.ini: @dontinclude omnetpp.ini @skip
[Config Wireless02] @until extends

This way we can easily base subsequent steps on the previous ones by adding a
few lines to the .ini file. Actually, this is true for all 19 steps -- each is
an extension of the previous one.

Let's turn on visualization of transmissions by editing the ini file:

@dontinclude omnetpp.ini
@skipline visualizer

Packet transfers are visualized by the animation of radio transmissions, so we
don't need the default packet animations any more. Let's turn them off in qtenv
(uncheck <i>animate messages</i> in preferences).

In order to get a smooth animation, we need to enable canvas updates and set an
update interval:

@dontinclude omnetpp.ini
@skipline Propagation

Also turn on communication trails, so we can get a fading blue line on
successfull communication paths:

@dontinclude omnetpp.ini
@skipline displayCommunicationTrail

The result is that we have nice bubble animations representing radio
transmissions, and blue lines indicating communication paths:

<img src="step2_2_v3.gif">

Sources: @ref omnetpp.ini, @ref WirelessA.ned

NEXT: @ref step3


*/--------------------------------------------------------------------------/**


@page step3 Step 3 - Adding more nodes and decreasing the communication range
to 250m

UP: @ref step2

In this scenario, we add 3 more hosts by extending WirelessA.ned into WirelessB.ned:

@dontinclude WirelessB.ned
@skip network
@until hostR3
@skipline }
@skipline }

We decrease the communication range to 250m, so host A and B are out of range,
and cannot communicate directly.

@dontinclude omnetpp.ini
@skipline *.host*.wlan[*].radio.transmitter.maxCommunicationRange = 250m

The other hosts are there to relay the data between them, but routing is not yet
 configured. The result is that Host A and B cannot communicate.

<img src="wireless-step3-v2.png">

Sources: @ref omnetpp.ini, @ref WirelessB.ned

NEXT: @ref step4


*/--------------------------------------------------------------------------/**


@page step4 Step 4 - Setting up static routing

UP: @ref step3

The recently added hosts will have to behave like routers, and forward packets
from Host A to Host B. We have to set forwarding:

@dontinclude omnetpp.ini
@skipline *.host*.forwarding = true

We need to set up static routes. We could do that manually, but that's tedious,
 so we let the configurator take care of it.

@dontinclude omnetpp.ini
@skipline *.configurator.config = xml("<config><interface hosts='**' address='10.0.0.x' netmask='255.255.255.0'/><autoroute metric='errorRate'/></config>")

We tell the configurator to assign IP addresses in the 10.0.0.x range, and to
create routes based on the estimated error rate of links between the nodes. In
the case of the <tt>IdealRadio</tt> model, the error rate is 1 for nodes that
are out of range, and 1e-3 for ones in range. The result will be that nodes that
are out of range of each other will send packets to intermediate nodes that can
forward them.

Turning off routing table optimizaton. This way there will be a distinct entry
for reaching each host, so the table is easier to understand.

@dontinclude omnetpp.ini
@skipline *.configurator.optimizeRoutes = false

Also disable routing table entries generated from the netmask and remove default
 routes (because it doesn't make sense in this adhoc network):

@dontinclude omnetpp.ini
@skipline **.routingTable.netmaskRoutes = ""

Turn on route visualization, so we can see colored arrows indicating the routes
that packets take:

@dontinclude omnetpp.ini
@skipline *.visualizer.routeVisualizer.packetNameFilter = "*"

The arrows indicate routes in the network layer -- so now we should see a route
going from Host A to B -- while we still have the blue lines that visualize
communication paths in the physical layer. Here we want to display all kinds of
packets, hence the "*".

Now the two nodes can communicate -- you can see that Host R1 relays data to
Host B.

The data rate is the same as before (800 kbps) -- even though multiple hosts are
 transmitting at the same time -- because we're still ignoring interference.

Note that there are blue lines leading to Host R2 and R3 even though they don't
transmit. This is because they receive the transmissions at the physical layer,
but they discard the packets at the link layer because it is not addressed to
them.

<img src="wireless-step4-v2.png">

Sources: @ref omnetpp.ini, @ref WirelessB.ned,

NEXT: @ref step5


*/--------------------------------------------------------------------------/**


@page step5 Step 5 - Taking into account the interference between different hosts

UP: @ref step4

We refine our model by enabling the simulation of interference:

@dontinclude omnetpp.ini
@skipline *.host*.wlan[*].radio.receiver.ignoreInterference = false

Set maximum interference range to the double of the communication range, 500m:

@dontinclude omnetpp.ini
@skipline *.host*.wlan[*].radio.transmitter.maxInterferenceRange = 500m

This means that Host A cannot communicate with Host B because it is out of
range, but its transmission will cause interference with other transmissions at Host B.

<img src="wireless-step5-v2.png">

Host A starts sending a lot of packets, at nearly the capacity of the medium.
R1 is constantly in receiving state -- nothing is controlling who can transmit
and when. R1's queue is filling up. It is supposed to relay the packets to B,
but can't as long as it is receiving A's transmissions. When A's random send
interval is a bit longer, R1 has the chance to send its queued packets to B.
Most of the time however, A starts transmitting again and its transmission
interferes with R1's at B. Packets only get through to B when the send interval
at A is greater than the time it takes for R1 to send a packet. The result is
that a very low number of packets get to B successfully. This is extemely low
throughput -- 40 packets arrive at B out of around 2500 (about 12 kbps out of
the 1 Mbps bandwidth).

@note If you lower the exponential send interval (for example, to 5ms).
In this case, A's transmission rate maxes out the radio bandwidth.
You will see that no packet arrives to B at all. The opposite happens if you
increase the interval beyond 10ms -- more packets get through to B.

What happens here is Host A starts sending packets at random intervals, and Host
R1 is supposed to relay them to Host B. However, Host R1 is almost constantly in
receiving state. R1 gets to transmit when A's random interval between
transmissions is by chance greater, but most of the time its transmission do not
make it Host B without Host A's transmissions interfering. Meanwhile, R1's send
queue is filling up, as it doesn't get the chance to transmit. The result is
that only a handful of packets make it to Host B. The throughput is minial -- 40
packets make it out of about 2500, which is about 12 kbps (out of 1 Mbps
possible bandwidth).

When you run the simulation, you will see that it's mostly Host A that is
transmitting -- Host R1 should be relaying the packets to Host B, but it can't
transmit while receiving from Host A. As Host A is generating packets in random
intervals, sometimes the interval is great enough for Host R1 to transmit a
packet. Most of the time, these packets are not delivered successfully because
Host A starts to transmit before Host R1 finished transmitting. So they are cut
by interference. Only a handful of packets arrive at Host B. <!rewrite>

To minimize interference, we need some kind of media access protocol to govern
which host can transmit and when.

Sources: @ref omnetpp.ini, @ref WirelessB.ned

NEXT: @ref step6

*/--------------------------------------------------------------------------/**

@page step6 Step 6 - Using CSMA to better utilize the medium

UP: @ref step5

With CSMA, hosts will sniff into the medium to see if there are ongoing
transmissions in their interference range. This is in contrast to the previous
step, where they didn't care if someone else was transmitting. This should
improve throughput, as the medium will be utilized better.

We need to switch the <tt>IdealWirelessNic</tt> to <tt>WirelessNic</tt>, which can use CSMA:

@dontinclude omnetpp.ini
@skipline *.host*.wlan[*].typename = "WirelessNic"

<tt>WirelessNic</tt> has <tt>Ieee80211</tt> radio by default, but we still
want to use <tt>IdealRadio</tt>:

@dontinclude omnetpp.ini
@skipline *.host*.wlan[*].radioType = "IdealRadio"

Set mac protocol to CSMA:

@dontinclude omnetpp.ini
@skipline *.host*.wlan[*].macType = "CSMA"

We need to turn on mac acknowledgements so hosts can detect if a transmission
needs resending:

@dontinclude omnetpp.ini
@skipline *.host*.wlan[*].mac.useMACAcks = true

<img src="wireless-step6-v2.png">

We can see that throughput is about 380 kbps, so it is increased over the
previous step thanks to CSMA -- altough less than in step 4 because of the
interference.

Sources: @ref omnetpp.ini, @ref WirelessB.ned

NEXT: @ref step7

*/--------------------------------------------------------------------------/**

@page step7 Step 7 - Configuring node movements

UP: @ref step6

Let's configure the intermediate nodes (R1-3) to move around. We set them to
move upwards at a speed of 12 miles per hour:

@dontinclude omnetpp.ini
@skip mobility
@until mobility.angle

<!more on linearmobility>
<!do we need more on mobility? should be clear from the code above>

You should run the simulation in fast mode to better see the nodes moving,
because they move very slowly if run in normal mode.

We see that data exchange works just like in the previous step until R1 moves
out of range of A. Traffic could be routed through R2 and R3, but the routing
tables are static, and configured according to the starting positions of the
nodes. Throughput is about 260 kbps, which is less than in the previous step,
because at around 18 seconds, R1 moves out of range of A thus severing the
connection to B.

<img src="step7_2_v3.gif">

A dynamic routing mechanism is needed to reconfigure the routes as nodes move out of range.

Sources: @ref omnetpp.ini, @ref WirelessB.ned

NEXT: @ref step8

*/--------------------------------------------------------------------------/**


@page step8 Step 8 - Configuring adhoc routing (AODV)

UP: @ref step7

Let's configure ad-hoc routing with AODV.

We need the <tt>IPv4NetworkConfigurator</tt> to only assign the IP addresses. We
turn all other functions off:

@dontinclude omnetpp.ini
@skip *.configurator.addStaticRoutes = false
@until Subnet

Replace <tt>INetworkNode</tt>s with <tt>AODVRouter</tt>s:

@dontinclude omnetpp.ini
@skipline *.hostType = "AODVRouter"

<tt>AODVRouter</tt> is basically an <tt>INetworkNode</tt> extended with the
<tt>AODVRouting</tt> submodule. Each node works like a router -- they manage
their own routing tables and adapt to changes in the network topology.

This time when R1 gets out of range, the routes are reconfigured and packets
keep flowing to B. Throughput is about the same as in step 6 -- even though the
connection is not broken here, the AODV protocol adds some overhead to the
communication.

<img src="wireless-step8-v2.png">

Sources: @ref omnetpp.ini, @ref WirelessB.ned

NEXT: @ref step9

*/--------------------------------------------------------------------------/**

@page step9 Step 9 - Installing energy management into the nodes

UP: @ref step8

The nodes behave like mobile devices, and we can model their energy consumption
and storage.

First set up energy consumption in the node radios:

@dontinclude omnetpp.ini
@skipline **.energyConsumerType = "StateBasedEnergyConsumer"

The <tt>StateBasedEnergyConsumer</tt> module models radio power consumption
based on states like radio mode, transmitter and receiver state. Each state has
a constant power consumption that can be set by a parameter. Energy use depends
on how much time the radio spends in a particular state.

Set up energy storage in the nodes -- basically modelling the batteries:

@dontinclude omnetpp.ini
@skipline *.host*.energyStorageType = "IdealEnergyStorage"

<tt>IdealEnergyStorage</tt> provides an infinite ammount of energy, can't be
fully charged or depleted. We use this because we want to concentrate on the
power consumption, not the storage.

The energyBalance variable indicates the energy consumption
(host*.energyStorage.energyBalance). You can use the residualCapacity signal
to display energy consumption over time.

<img src="wireless-step9-energy.png">

Sources: @ref omnetpp.ini, @ref WirelessB.ned

NEXT: @ref step10

*/--------------------------------------------------------------------------/**

@page step10 Step 10 - Adding obstacles to the environment

UP: @ref step9

Up until now, the nodes were operating in free space. In the real world,
however, there are usually obstacles that decrease signal strength, absorb or
reflect radio waves.

Let's add a concrete wall to the model that sits between Host A and R1, and see
what happens to the transmissions.

We have to extend WirelessB.ned to include an <tt>environment</tt> module:

@dontinclude WirelessC.ned
@skip network WirelessC extends WirelessB
@until @display
@skipline }
@skipline }

The physical environment module implements the objects that interact with
transmissions -- various shapes can be created. <!rewrite>

Objects can be defined in .xml files (see details in the
<a href="https://omnetpp.org/doc/inet/api-current/inet-manual-draft.pdf" target="_blank">INET manual</a>).
Our wall is defined in walls.xml.

@dontinclude walls.xml
@skip environment
@until /environment

We need to configure the environment in omnetpp.ini:

@dontinclude omnetpp.ini
@skipline *.physicalEnvironment.config = xmldoc("walls.xml")

To calculate interactions with obstacles, we need an obstacle loss model:
@dontinclude omnetpp.ini
@skipline obstacleLoss

<tt>TracingObstacleLoss/tt> models signal loss along a line connecting the
<transmitter and the receiver, calculating the loss where it intersects
<obstackles.!rewrite>

Unfortunatelly, the wall has no effect on the transmissions -- the number of
received packets is exactly the same as in the previous step -- because our
simple radio model doesn't interact with obstacles. We need a more realistic
radio model.

<img src="wireless-step10-v2.png">

Sources: omnetpp.ini, WirelessC.ned, walls.xml

NEXT: @ref step11

*/--------------------------------------------------------------------------/**

@page step11 Step 11 - Enhancing the accuracy of the radio model

UP: @ref step10

We will have to replace <tt>IdealRadio</tt> with APSKScalarRadio, which is more
realistic. It implements a radio that uses APSK modulation, but it is not using
other techniques like forward error correction, interleaving or spreading.
Nevertheless, it models attenuation and obstacle loss.

So let's switch <tt>IdealRadioMedium</tt> with <tt>APSKScalarRadioMedium</tt>:

@dontinclude omnetpp.ini
@skipline *.mediumType = "APSKScalarRadioMedium"

Set up some background noise:

@dontinclude omnetpp.ini
@skipline *.radioMedium.backgroundNoise.power = -110dBm

<tt>APSKRadioMedium</tt> uses <tt>IsotropicScalarBackgroundNoise</tt> by
default. This is basically white noise that is constant in space, time and
frequency.<!white noise already means every frequency>

<!frequency 2 ghz>

Set up <tt>APSKScalarRadio</tt>'s in the nodes and configure each radio:

@dontinclude omnetpp.ini
@skip *.host*.wlan[*].radioType = "APSKScalarRadio"
@until *.host*.wlan[*].radio.receiver.snirThreshold = 4dB

@note Each <tt>radioMedium</tt> model has to be used with the corresponding
radio model -- this case <tt>APSKScalarRadioMedium</tt> with
<tt>APSKScalarRadio</tt>. The same was true with <tt>IdealRadio</tt>.
<!do we need this - is it correct - do we need this here and not at idealradio -
last 3 lines - preambleduration?>

<!img>
<!results>
<!throughput>

Now our model takes the objects into account when calculating attenuation.
The wall is blocking the transmission between Host A and R1 when R1 gets
behind it.<!rewrite>

Sources: @ref omnetpp.ini, @ref WirelessC.ned

NEXT: @ref step12

*/--------------------------------------------------------------------------/**

@page step12 Step 12 - Configuring a more accurate pathloss model

UP: @ref step11

*/

*/

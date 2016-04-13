/**
@mainpage Wireless Tutorial for the INET framework

In this tutorial we show you how build wireless simulations in the INET framework. It contains a series of simulation
models numbered from 1 through 19. The models are of increasing complexity -- they start from the basics and introduce 
new INET features and concepts related to wireless communication networks.

This is an advanced tutorial, and it assumes that you are familiar with creating and running simulations in @opp and 
INET. If you are yet not, you can check out the <a href="https://omnetpp.org/doc/omnetpp/tictoc-tutorial/" target="_blank">TicToc Tutorial</a> to get started with using @opp. The <a href="../../../doc/walkthrough/tutorial.html" target="_blank">ARP Tutorial</a> is an introduction to INET and how to work with protocols.

If you need more information at any time, feel free to refer to the @opp and INET documentation:

- 	<a href="https://omnetpp.org/doc/omnetpp/manual/usman.html" target="_blank">@opp User Manual</a>
-	<a href="https://omnetpp.org/doc/omnetpp/api/index.html" target="_blank">@opp API Reference</a>
- 	<a href="https://omnetpp.org/doc/inet/api-current/inet-manual-draft.pdf" target="_blank">INET Manual draft</a>
- 	<a href="https://omnetpp.org/doc/inet/api-current/neddoc/index.html" target="_blank">INET Reference</a>

The tutorial starts off with a basic simulation model at step 1, and gradually make it more complex and realistic in subsequent steps -- so you can learn about various INET wireless features and what can be achieved with them.

Feel free to try the steps as you progress with the tutorial -- all simulation models are defined in omnetpp.ini

@section contents Contents

-	@ref step1
-	@ref step2
-	@ref step3
-	@ref step4

NEXT: @ref step1
*/
-----------------------------------------------------------------------------------------------------------------------
/**

@page step1 Step 1 - Two nodes communicating via UDP
UP: @ref overview

In the first scenario, we set up two hosts, with one host sending data wirelessly to the other via UDP. Right now, we don't care if the wireless exchange is realistic or not, just want the hosts to transfer data between each other. There are no collisions, and other physical effects -- like attenuation and multipath propagation -- are ignored. The network topology is defined in the .ned files -- in this case WirelessA.ned.

<img src="wireless-step1.png">

First, we create the network environment -- this is where the simulation will take place -- and specify its size to 500x500:

@dontinclude WirelessA.ned
@skip network WirelessA
@until @display

Then we add the two nodes 400 meters apart:

@dontinclude WirelessA.ned
@skip hostA: <hostType>
@until @display("p=450,250");
@skipline }

We could have just used <tt>INetworkNode</tt>, but we're using parametrized types, so their types and properties can be easily modified from the .ini file. This way we can create the increasingly complex simulations by extending the one used in the previous step. For example, we can switch ideal components to more realistic ones.

The two nodes want to communicate wirelessly, and for that we need a radio medium module. In this case we add <tt>IdealRadioMedium</tt>. 

@dontinclude WirelessA.ned
@skip radioMedium: <mediumType>
@until @display

The radio medium in general is responsible for coordinating the radio transmissions in the model. Hosts do not send radio packets to each other, but hand it to the radioMedium, which computes which hosts will receive the transmission and when, based on their positions and distance, taking other physical effects like attenuation and noise into account. It also computes when collisions happen. This way hosts don't have anything to do with who gets their transmission -- the radioMedium handles that. In the animation, hosts are shown to be sending messages directly to each other for clarity.

<tt>IdealRadioMedium</tt> is a simple model of radio transmission -- the success of reception only depends on the distance of the two nodes -- whether or not they are in communication range. In-range packets are always received and out-of-range ones are never.

We can configure this range in the wireless NIC of the host. Let's add a simple wireless NIC to the hosts in the .ini file:

@dontinclude omnetpp.ini
@skipline .host*.wlan[*].typename = "IdealWirelessNic"

Now set the communication range to 500m:

@dontinclude omnetpp.ini
@skipline *.host*.wlan[*].radio.transmitter.maxCommunicationRange = 500m

Because we want a simple model, we don't care about interference. The two hosts can transmit simultaneously, but it doesn't affect packet traffic. We turn interference off in the NIC card by specifying the parameter in the .ini file:

@dontinclude omnetpp.ini
@skipline *.host*.wlan[*].radio.receiver.ignoreInterference = true

Finally, we set the transmission bandwidth to 1 Mbps of all radios in the model:

@dontinclude omnetpp.ini
@skipline **.bitrate = 1Mbps

The radio part is done.

Now let's assign IP addresses to the nodes. We could do that manually, but now we really don't care about what IP address they are getting -- we just want to concentrate on the wireless communication. We let <tt>IPv4Configurator</tt> handle the assignment.

@dontinclude WirelessA.ned
@skip submodules
@until @display
@skipline }

The configurator assigns the IP addresses and sets up static routing between the nodes. The configurator has no gates and does not connect to anything, only stores the routing information. Nodes contain an <tt>IPv4NodeConfigurator</tt>  module that configures hosts' routing tables based on the information stored in the configurator (the <tt>IPv4NodeConfigurator</tt> is included in the <tt>INetworkNode</tt> module by default).

The hosts have to know each other's MAC addresses to communicate, which is handled by the ARP protocol. Since we want to concentrate on the UPD exchange, we can set the MAC addresses even before the simulation begins to cut the ARP resolution messages, by using <i>GlobarARP</i>:

@dontinclude omnetpp.ini
@skipline **.arpType = "GlobalARP"

We need to set up UDP applications in the nodes in omnetpp.ini so they can exchange traffic:

@dontinclude omnetpp.ini
@skip *.hostA.numUdpApps = 1
@until *.hostA.udpApp[0].sendInterval = exponential(10ms)

The UDPBasicApp generates 1000 byte messages at random intervals with exponential distribution, the mean of which is 10ms. Therefore the app is going to generate 100 kbyte/s (800 kbps) UDP traffic (not counting protocol overhead).

The <tt>UDPSink</tt> at the other node just discards received packets:

@dontinclude omnetpp.ini
@skip *.hostB.numUdpApps = 1
@until *.hostB.udpApp[0].localPort = 5000

We also add a gauge to display the number of packets received by Host B:

@dontinclude WirelessA.ned
@skipline @figure[thruputInstrument](type=gauge; pos=370,90; size=120,120; maxValue=2500; tickSize=500; colorStrip=green 0.75 yellow 0.9 red;label=Number of packets received;

Let's run the simulation. We can see the nodes exchanging UDP packets:

<img src="step1_v4.gif">

The simulation concludes at t=25s, and the throughput instrument indicates that around 2400 packets were sent. A packet with overhead is 1028 bytes, which means the transmission rate was around 800 kbps.

Sources: @ref omnetpp.ini, @ref WirelessA.ned

NEXT: @ref step2
*/
-----------------------------------------------------------------------------------------------------------------------
/**
@page step2 Step 2 - Set up some animations

UP: @ref step1

We would like to visualize radio waves as they propage through space. For that we can use the @opp Canvas API.

Note that the configuration for Step 2 -- <i>Wireless02</i> -- is created by extending <i>Wireless01</i> in omnetpp.ini:
@dontinclude omnetpp.ini
@skip [Config Wireless02]
@until extends

This way we can easily base subsequent steps on the previous ones by adding a few lines to the .ini file. Actually, this is true for all 19 steps -- each is an extension of the previous one.

Let's turn on visualization of transmissions by editing the ini file:

@dontinclude omnetpp.ini
@skipline *.radioMedium.mediumVisualizer.displayCommunication = true

Packet transfers are visualized by the animation of radio transmissions, so we don't need the default packet anymations any more. Let's turn them off in qtenv (uncheck animate messages).

In order to get a smooth animation, we need to enable canvas updates and set an update interval:

@dontinclude omnetpp.ini
@skipline *.radioMedium.mediumVisualizer.updateCanvasInterval = 100ns

Also turn on communication trails, so we can get a fading blue line on successfull communication paths:

@dontinclude omnetpp.ini
@skipline *.radioMedium.mediumVisualizer.leaveCommunicationTrail = true

The result is that we have nice bubble animations representing radio transmissions, and blue lines indicating communication paths:

<img src="step2_v2.gif">

Sources: @ref omnetpp.ini, @ref WirelessA.ned

NEXT: @ref step3

*/
-----------------------------------------------------------------------------------------------------------------------
/**

@page step3 Step 3 - Add more nodes and decrease the communication range to 250m

UP: @ref step2

In this scenario, we add 3 more hosts by extending WirelessA.ned into WirelessB.ned:

@dontinclude WirelessB.ned
@skip network
@until hostR3
@skipline }
@skipline }

We decrease the communication range to 250m, so host A and B are out of range, and cannot communicate directly.

@dontinclude omnetpp.ini
@skipline *.host*.wlan[*].radio.transmitter.maxCommunicationRange = 250m

The other hosts are there to relay the data between them, but routing is not yet configured. The result is that Host A and B cannot communicate.

<img src="wireless-step3.png">

Sources: @ref omnetpp.ini, @ref WirelessB.ned

NEXT: @ref step4

*/
-----------------------------------------------------------------------------------------------------------------------
/**

@page step4 Step 4 - Set up static routing

UP: @ref step3

The recently added hosts will have to behave like routers, and forward packets from Host A to Host B (and vice-versa). We have to set forwarding:

@dontinclude omnetpp.ini
@skipline *.host*.forwarding = true

We need to set up static routes. We could do that manually, but that's tedious, so we let the configurator take care of it. 

@dontinclude omnetpp.ini
@skipline *.configurator.config = xml("<config><interface hosts='**' address='10.0.0.x' netmask='255.255.255.0'/><autoroute metric='errorRate'/></config>")

We tell the configurator to assign IP addresses in the 10.0.0.x range, and to create routes based on the error rate of links between the nodes. In the case of the <tt>IdealRadio</tt> model, this is 1 for nodes that are out of range, and 1e-3 for ones in range. The result will be that nodes that are out of range of each other will send packets to intermediate nodes that can forward them.

Turning off routing table optimizaton. This way there will be a distinct entry for reaching each host, so the table is easier to understand.

@dontinclude omnetpp.ini
@skipline *.configurator.optimizeRoutes = false

Also disable routing table entries generated from the netmask and remove default routes (because it doesn't make sense in this adhoc network):

@dontinclude omnetpp.ini
@skipline **.routingTable.netmaskRoutes = ""

Now the two nodes can communicate -- you can see that Host R1 relays data to Host B.

The data rate is the same as before -- even though multiple hosts are transmitting at the same time -- because we're still ignoring interference.

<img src="wireless-step4.png">

Sources: @ref omnetpp.ini, @ref WirelessB.ned

*/
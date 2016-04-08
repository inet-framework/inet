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

@section contents Contents

-	@ref overview
-	@ref step1
-	@ref step2
-	@ref step3
-	@ref step4

NEXT: @ref overview
*/
-----------------------------------------------------------------------------------------------------------------------

/**

@page overview Overview
UP: @ref contents

The tutorial consists of 19 simulation models -- steps 1 to 19. They are based on 3 network topologies, which are defined in the 3 .ned files (WirelessA.ned, WirelessB.ned, WirelessC.ned). They use parametrized types, so their types and properties can be changed from the .ini file. This is what we do here to create the 19 simulation models. Some of them are derived from a previous model.

There are 3 network topologies -- defined in 3 .ned files -- that are increasing in complexity, and an extension of the previous one. They share a single .ini file, where the 19 simulations are created...

what do you want to say?

there are 3 ned files, which contain the topology, and they extend each other. The ini file defines the 19 simulation models that increase in complexity. The ned files make use of parametrized types so the settings can be changed from the ini file. they are derived from the previous one as well.

you are welcome to try them, but this tutorial should be enough without that.

The purpose of this tutorial is to show what features of wireless simulations in INET have and what can be built and simulated.

<! write introduction later ><!do we need this at all>
NEXT: @ref step1
*/
-----------------------------------------------------------------------------------------------------------------------
/**

@page step1 Step 1 - Two nodes communicating via UDP
UP: @ref overview

In the first scenario, we set up two hosts, with one host sending data wirelessly to the other via UDP. The network topology is defined in the .ned files -- in this case WirelessA.ned.

<img src="wireless-step1.png">

First, we create the network environment -- this is where the simulation will take place -- and specify its size to 500x500:

@dontinclude WirelessA.ned
@skip network WirelessA
@until @display

Then we add the two nodes:

@dontinclude WirelessA.ned
@skip hostA: <hostType>
@until @display("p=450,250");
@skipline }

We could have just used <tt>INetworkNode</tt>, but we're using parametrized types, so their types and properties can be easily modified from the .ini file. This way we can create the increasingly complex simulations by extending the one used in the previous step. For example, we can switch ideal components to more realistic ones.

The two nodes want to communicate wirelessly, and for that we need a radio medium module. In this case we add <tt>IdealRadioMedium</tt>. 

@dontinclude WirelessA.ned
@skip radioMedium: <mediumType>
@until @display

This is a simple model of radio transmission -- the success of reception only depends on the distance of the two nodes -- whether or not they are in communication range. In-range packets are always received and out-of-range ones are never.

We can configure this range in the wireless NIC of the host. Let's add a simple wireless NIC to the hosts in the .ini file:

@dontinclude omnetpp.ini
@skipline .host*.wlan[*].typename = "IdealWirelessNic"

Now set the communication range to 500m:

@dontinclude omnetpp.ini
@skipline *.host*.wlan[*].radio.transmitter.maxCommunicationRange = 500m

Because we want a simple model, we don't care about interference. The two hosts can transmit simultaneously, but it doesnt affect packet traffic. We turn interference off in the NIC card by specifying the parameter in the .ini file:

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

The hosts have to know each other's MAC addresses to communicate, which is handled by the ARP protocol. Since we want to concentrate on the UPD exchange, we can set the MAC addresses even before the simulation begins, to cut the ARP resolution messages by using <i>GlobarARP</i>:

@dontinclude omnetpp.ini
@skipline **.arpType = "GlobalARP"

We need to set up UDP applications in the nodes in omnetpp.ini so they can exchange traffic:

@dontinclude omnetpp.ini
@skip *.hostA.numUdpApps = 1
@until *.hostA.udpApp[0].sendInterval = exponential(10ms)

The UDPBasicApp generates 1000 byte messages at random intervals, the mean of which is 10ms. Therefore the app is going to generate 100 kbyte/s (800 kbps) UDP traffic (not counting protocol overhead).

The <tt>UDPSink</tt> at the other node just discards received packets:

@dontinclude omnetpp.ini
@skip *.hostB.numUdpApps = 1
@until *.hostB.udpApp[0].localPort = 5000

We also add a gauge to display the number of packets received by Host B:

@dontinclude WirelessA.ned
@skipline @figure[thruputInstrument](type=gauge; pos=370,90; size=120,120; maxValue=2500; tickSize=500; colorStrip=green 0.75 yellow 0.9 red;label=Number of packets received;
*/
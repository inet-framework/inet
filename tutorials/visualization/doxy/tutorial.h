/**
@mainpage Visualization Tutorial for the INET Framework
In this tutorial we show you, how to use the visualization module in the INET Framework.
The tutorial contains a series of simulation models, numbered from 1 through 22.
The models are of increasing complexity – they start from the basics and in each step, they introduce new INET features and concepts related to visualization module.
Our goal is show the working of the visualization module and its submodules in INET.

This is an advanced tutorial, and it assumes that you are familiar with creating and running simulations in @opp and  INET.
If you aren't, you can check out the <a href="https://omnetpp.org/doc/omnetpp/tictoc-tutorial/" target="_blank"> TicToc Tutorial</a> to get started with using @opp. The <a href="../../../doc/walkthrough/tutorial.html" target="_blank"> INET Walkthrough</a> is an introduction to INET and working with protocols.

If you need more information at any time, feel free to refer to the @opp and INET documentation:

- <a href="https://omnetpp.org/doc/omnetpp/manual/usman.html" target="_blank">@opp User Manual</a>
- <a href="https://omnetpp.org/doc/omnetpp/api/index.html" target="_blank">@opp API Reference</a>
- <a href="https://omnetpp.org/doc/inet/api-current/inet-manual-draft.pdf" target="_blank">INET Manual draft</a>
- <a href="https://omnetpp.org/doc/inet/api-current/neddoc/index.html" target="_blank">INET Reference</a>

In the tutorial, each step is a separate configuration in the same omnetpp.ini file.
Steps build on each other, they extend the configuration of the previous step by adding a few new lines.
Consecutive steps mostly share the same network, defined in NED.

@section contents Contents

- @ref step1
- @ref step2
- @ref step3
- @ref step4
- @ref step5
- @ref step6
- @ref step7
- @ref step8
- @ref step9
- @ref step10
- @ref step11
- @ref step12
- @ref step13
- @ref step14
- @ref step15
- @ref step16
- @ref step17
- @ref step18
- @ref step19
- @ref step20
- @ref step21
- @ref step22
- @ref step23
- @ref step24
- @ref conclusion

@nav{index,step1}
@fixupini

<!------------------------------------------------------------------------>

@page step1 Step 1 - Enabling visualization

@nav{index,step2}

@section s1goals Goals
The default visualization of OMNeT++ already displays message sends, methods calls, etc.
These are rather low level details compared to the domain specific visualization of INET.
The complex state and behavior of communication protocols provide much more opportunity
for visualizations. In the first step we create a model with INET visualizations enabled.

@section s1model The model

@section s1results Results

Sources: @ref omnetpp.ini, @ref VisualizationNetworks.ned

@nav{index,step2}
@fixupini

<!------------------------------------------------------------------------>

@page step2 Step 2 - Showing Boston downtown

@nav{step1,step3}

@section s2goals Goals
In network simulation it's often desirable to have a real world context. Although displaying a map
doesn't directly affect the outcome of the simulation, nevertheless it still facilitates understanding.
In this step we display a street map of Boston downtown.

@section s2model The model
<!--
Here is the NED file of the network:
@dontinclude VisualizationNetworks.ned
@skip network VisualizationA
@until ####

This is a very simple model, contains an <tt>IntegratedVisualizer</tt> and an <tt>OsgGeographicCoordinateSystem</tt> submodule. The visualizer is responsible all of phenomenon, that we can see on the playground.
We can change their parameters in the <tt>omnetpp.ini</tt> file.
The ini file contains the parameters of these submodules.

@dontinclude omnetpp.ini
@skipline [Config Visualization01]
@until ####

In this part of the file, there are some coordinate system and the scene visualizer parameters. 
<i>Coordinatesystem</i> parameters set the longitude, latitude and altitude coordinate of the playground origin and the heading of it.
<tt>SceneVisualizer</tt> parameters set the visualizer type. 
We add a map to the simulation with the <tt>mapFile</tt> parameter. 
Besides these options we turn off the playground shading and configure the opacity and the color of the playground. 
We need to make the playground transparent, because it's over the map, and later if we place a node, we can't see the map under of the area from the origin to the node. 
With the <tt>axisLength</tt> parameter we can change the axis' size on the map.
-->
@section s2results Results

<img src="step2_map_without_axis.png" width="850">
<!--
When we start the simulation we can see what we expected. 
There's the map of Boston downtown with axis.

Using the mouse, we can move and rotate the camera. 
If we hold down the left mouse button, we can navigate on the map. 
Holding down the mouse wheel or both mouse button at the same time we can rotate the camera, and if we scroll up and down we can zoom out and in. 
If we click with the right mouse button, we can change between camera modes.
In the top right corner of the playground, we can change between 3D Scene view mode and Module view.
-->

Sources: @ref omnetpp.ini, @ref VisualizationNetworks.ned

@nav{step1,step3}
@fixupini

<!------------------------------------------------------------------------>

@page step3 Step 3 - Showing the playground

@nav{step2,step4}

@section s3goals Goals
Even though it's possible to express the position of network nodes, the movement of mobile nodes,
the geometry of physical objects using geographic locations, internally INET uses a Cartesian
coordinate system called the playground. Such coordinates may appear in the runtime GUI or the
simulation log or even in the debugger. To help you deal with that, in this step we show the
playground along with its coordinate axes.

@section s3model The model

@section s3results Results

<img src="step1_map_with_axes.png" width="850">

Sources: @ref omnetpp.ini, @ref VisualizationNetworks.ned

@nav{step2,step4}
@fixupini

<!------------------------------------------------------------------------>

@page step4 Step 4 - Adding static 3D objects to the scene (WIP)

@nav{step3,step5}

@section s4goals Goals
In tis step we extend the scene with a 3D object model. 

@section s4model The model

@section s4results Results

Sources: @ref omnetpp.ini, @ref VisualizationNetworks.ned

@nav{step3,step5}
@fixupini

<!------------------------------------------------------------------------>

@page step5 Step 5 - Displaying communication/interference range

@nav{step4, step6}

@section s5goals Goals
For wireless networks to work, communicating devices have to be
within each other's communication range.  In this step we display
these areas to help putting nodes to the right place on the map.

<!--
A wireless hálózatok működéséhez szükséges, hogy az eszközök egymás communication range-ben legyenek.
Ebben a lépésben megjelenítjük ezeket a range-ket, hogy megfelelően el tudjuk
helyezni az eszközöket a térképen.
-->

@section s5model The model
<!--
In this step we display the communication and interfaces range of wireless nodes in the network.

Later we want to see communication between these network nodes, so we have to place them in each other's communication range.
In this step we want to visualize these areas to place devices .

TODO: Now we extend our model with two pedestrians, and an Access Point.


This is our extended network file:

@dontinclude VisualizationNetworks.ned
@skip network VisualizationB
@until ####

To achieve our goal, we need to add two <tt>WirelessHost</tt>s, and an <tt>AccessPoint</tt> to the network.
To communicate with each other, we need an <tt>IPv4NetworkConfigurator</tt> and an <tt>Ieee80211ScalarRadioMedium</tt> submodule.
The configurator prepares the network nodes to the communication, the radioMedium manages the media.

In the <tt>omnetpp.ini</tt> file, we adjust the transmission power of the network nodes.
We have to do that, because by using the default transmission power parameter, the ranges will be too big.
We set the transmission power of the <i>accessPoint0</i> bigger, than the <i>pedestrian0</i> and <i>pedestrian1</i> transmission power.
It's possible to modify the color of the ranges with the <tt>communicationRangeColor</tt> and <tt>interferenceRangeColor</tt> parameters.
Now we leave them on the default value: the communication range is blue and the interference range color is grey.
Below, there is the appropriate part of the ini file:

@dontinclude omnetpp.ini
@skipline [Config Visualization03]
@until ####
-->
@section s5results Results

<img src="step3_result1.png">
<img src="step3_result2.png">
<!--
If we run the simulation in the 3D Scene view mode, we can see the three nodes and circles around them.
Each node is in the center of a circle, that circle is the node's communication range.

We configured the visualization of interference ranges too.
These are also on the map, but they're very big, so we have to zoom out or move to any direction to see these ranges.
The communication and interference ranges seen in the Module view mode too.

When we run the simulation, the pedestrians associate with the access point.
In Module view mode there's a bubble message when its happens.
-->
Sources: @ref omnetpp.ini, @ref VisualizationNetworks.ned

@nav{step4, step6}
@fixupini

<!------------------------------------------------------------------------>

@page step6 Step 6 - Using 3D models for network nodes

@nav{step5, step7}

@section s6goals Goals
ad1:
However it doesn't directly affect the outcome of the simulation, it is 
more understandable if the appearance of nodes is similar as in real world.
The default icon for the pedestrians is a cellphone. In this step we replace 
the default icon with an 3D animated model.

<!--
ad2:
The simulation is more understandable if the appearance of nodes is similar as in real world.
The default icon for the pedestrians is a cellphone. In this step we replace 
the default icon with an 3D animated model. It doesn't directly affect the 
outcome of the simulation.
-->
<!--
A szimuláció sokkal érthetőbb, ha a node-ok hasonlóan néznek ki, mint ahogy a valóságban.
A pedestrian-ok default ikonja egy mobil. Ebben a lépésben kicseréljük egy 
3D animált modellre. Ennek semmi közvetlen hatása nincs a szimuláció eredményére.
-->

@section s6model The model
<!--
<i>Pedestrian0</i> and <i>pedestrian1</i> are <tt>WirelessHost</tt> node type, so by default their icon is a cellphone.
We want to show, how to change network nodes' default icon.
In INET Framework it's possible to change device appearance to an external 3D osg model.
It's really simple.
We have to change only the network node's <tt>osgModel</tt> attribute.
We set that to <tt>boxman.osgb</tt>.
That's the file name of the 3D model.
In addition we can set the size and the rotation of the model.
The <tt>(0.06).scale</tt> means the model size is 6% of the original.
The three numbers are in for the rot keyword mean the rotation of the 3D model around x, y and z axis.

@dontinclude omnetpp.ini
@skipline [Config Visualization04]
@until ####
-->
@section s6results Results

<img src="step4_result1.gif">
<!--
In Module view mode there's no difference compared to the simulation before this.
But in 3D Scene view mode instead of phones we see walker boxmans.
-->

Sources: @ref omnetpp.ini, @ref VisualizationNetworks.ned

@nav{step5, step7}
@fixupini

<!------------------------------------------------------------------------>


@page step7 Step 7 - Displaying recent movement

@nav{step6, step8}

@section s7goals Goals

When wireless nodes roam in a network, it's often difficult to follow their movement. 
In this step we visualize the recent movement trajectory of network nodes.

@section s7model The model

<img src="step7_without_movement_trails.gif">
<!--
The network nodes movement handled by the <tt>Mobility</tt> submodule.
To visualize that, we need to use <tt>MobilityVisualizer</tt>.

Here is the configuration:

@dontinclude omnetpp.ini
@skipline [Config Visualization05]
@until ####

We have to adjust the nodes mobility settings. <i>Pedestrian0</i> and <i>Pedestrian1</i> movement is calculated using "MassMobility".
This is a random mobility model for a mobile host with a mass.
We set their initial position, and a border, because we don't want to let them go out from the <i>accessPoint0</i>'s communication range.
We have more ways to set the nodes initial position.
We can set that in meter or we can add that in degree.
The pedestrians' movement based on three parameters.
The <tt>changeInterval</tt> is the frequency of changing speed and angle, the <tt>changeAngleBy</tt> change the direction of the movement, and the <tt>speed</tt> means the movement speed.

After that we need to add some visualizer parameters.
We display a trail, that shows the passed route, and we visualize an arrow, that represents the velocity of the pedestrians.
-->
@section s7results Results

<img src="step07_moving_2d.gif">
<img src="step5_result3.gif" width="850">
<!--
It is advisable to run the simulation in Fast mode, because the nodes move very slowly if viewed in Normal mode.

It can be seen in the animation below <i>pedestrian0</i> and <i>pedestrian1</i> roam in the park between invisible borders that we adjust to them.

Here's that in Module view mode:


And here's that in 3D Scene view mode:
-->

Sources: @ref omnetpp.ini, @ref VisualizationNetworks.ned

@nav{step6, step8}
@fixupini

<!------------------------------------------------------------------------>

@page step8 Step 8 - Showing IP adresses

@nav{step7,step9}

@section s8goals Goals

Understanding network traffic often requires indentifying nodes based on their IP addresses.
In this step we show the IP addresses assigned by the network configurator. 

@section s8model The model
<!--
If we want to see the network nodes' IP addresses we have to set the parameters of the <tt>InterfaceTableVisualizer</tt>.
Here is the configuration:

@dontinclude omnetpp.ini
@skipline [Config Visualization06]
@until ####

With the <tt>nodeFilter</tt> parameter we can define the nodes list, that are considered.
By default that list is empty, so we must set it.
Then we can set an interface list.
That specifies which interfaces are considered at each node.
Besides of these parameters we can change the font color, the background color, and the opacity of the text.
These settings are optional, that may make the IP addresses clearly visible.

With <tt>InterfaceTableVisualizer</tt> we can display not only the IP address of a NIC, but the MAC address too.
To that we need to change the <tt>content</tt> parameter to <tt>macAddress</tt>.
-->
@section s8results Results

<img src="step08_ipaddress_2d.gif">
<img src="step6_result4.gif"  width="850">
<img src="step6_result3.png" width="850">
<!--
If we run the simulation, we can see a yellow bubble above each pedestrian with its wlan NIC IP address.

In 3D view mode the text is a little bit fainter.

If we set the content parameter to <tt>macAddress</tt>, we can see the given NIC MAC (layer 2) address.
-->

Sources: @ref omnetpp.ini, @ref VisualizationNetworks.ned

@nav{step7,step9}
@fixupini

<!------------------------------------------------------------------------>

@page step9 Step 9 - Showing wifi association

@nav{step8,step10}

@section s9goals Goals

In infrastructure mode wireless nodes have to associate with access points to be able to communicate.
The association may change over time, or at any given time there might be several access points for a
wireless node to associate with. In general, it's difficult to tell which nodes are associated with which access points.
In this step we enable displaying the association between wireless nodes and access points.

@section s9model The model
<!--
The pedestrians are in the access point's communication range, so they can associate with that.

In the ini file we need to set only the <tt>Ieee80211Visualizer</tt>'s parameters.
This visualizer will display us information about the association.
We can set which nodes and which interfaces are considered, like at <tt>InterfaceTableVisualizer</tt>.

Here is the appropriate configuration:

@dontinclude omnetpp.ini
@skipline [Config Visualization07]
@until ####
-->
@section s9results Results

<img src="step09_wifi_assoc_2d.gif">
<img src="step7_result2.gif">
<!--
In Module view mode we can monitor the association process, we see all messages between the nodes.
When a pedestrian send an <tt>Assoc</tt> message, the access point in its communication range receive that, and a signal sign appear above the access point.
In response to this <tt>Assoc</tt> message, the access point reply with an <tt>AssocResp-</tt> message.
If the association is successful it's <tt>AssocResp-OK</tt> and a signal sign appear above that pedestrian who wants to associate with the access point.

In 3D view mode as a result of the association process the signal sing appears above the appropriate network node.
-->

Sources: @ref omnetpp.ini, @ref VisualizationNetworks.ned

@nav{step8,step10}
@fixupini

<!------------------------------------------------------------------------>

@page step10 Step 10 - Displaying transmissions and receptions

@nav{step9,step11}

@section s10goals Goals

In this step we display transmissions and receptions to identify the participants of communication.
Data communication refers to the exchange of data between a source and a receiver. 
The meanings of source and receiver are very simple. The device that transmits the data 
is known as source and the device that receives the transmitted data is known as receiver.

<!--
---
-->

@section s10model The model
<!--
After the successful association process we can start the VoIP application between the pedestrians, so we need to add one udp application to them.
The pedestrian0 will be the sender and the pedestrian1 will be the receiver.
They communicate with udp over port 5000. 
The application starts at 1 second.
We leave all other options at their default values at the sender side.
At the receiver side we need to set the port to 5000, and we turn on the adaptive playout setting.
It will be used later.

We need to turn on some mediumVisualizer parameters.
We set true the displayTransmissions and the displayReceptions options.
We have to set an image to these options to display them.

Configuration:

@dontinclude omnetpp.ini
@skipline [Config Visualization08]
@until ####
-->
@section s10results Results

<img src="step8_result1.gif">
<!--
If we start the simulation, we can see clearly, who is the transmitter, and who are the receivers.
The signs appear, when a signal arrives or leaves the wlan NIC.
-->

Sources: @ref omnetpp.ini, @ref VisualizationNetworks.ned

@nav{step9,step11}
@fixupini

<!------------------------------------------------------------------------>

@page step11 Step 11 - Showing propagating signals

@nav{step10,step12}

@section s11goals Goals

Understanding network communication, we need to know that how the nodes 
communicate with each other on the wireless channel. To this we should see propagation 
signals. In this step we display propagating signals between wireless nodes.

In this step we display propagating signals. When we turn on this visualizer we can 
see the communication through wireless channel. It helps to monitoring network 
communication.

<!--
A hálózati kommunikáció megértéséhez szükségünk van arra, hogy tudjuk, hogy a network node-ok
hogyan kommunikálnak egymás között a vezeték nélküli csatornán. Ehhez látnunk kell 
a vezeték nélküli jeleket. Ebben a lépésben megmutatjuk, hogy a wireless node-ok milyen üzeneteket
továbbítanak egymás között a hálózaton.

Ebben a lépésben a jelek terjedését mutatjuk meg. Ha bekapcsoljuk ezt a visualizert, akkor 
láthatjuk az egyes packet-ek terjedését a vezeték nélküli közegben. Ez nagy segítség a hálózati 
kommunikáció monitorozásában.
-->

@section s11model The model
<!--
The communication works, we can see who is the sender and the receiver, but we don't see the signal.
In this step we want to display the signal propagation.

in the previous step we configured the communication already, so now we only have to show that.
To this we need to turn on the mediumVisualizer's displaySignals parameter,
and set a propagation interval. It's possible to set the signal shape.
We have three options: ring, sphere and both.
If we set the shape ring the signals propagate on the ground in 2 dimension.
The Module view mode can display signals only this way.
The sphere is a 3D displaying mode, the signals propagate as a sphere under and over the ground.
If we set both, we can see a ring on the ground, and a sphere in the air. This is the default option.

@dontinclude omnetpp.ini
@skipline [Config Visualization09]
@until ####
-->
@section s11results Results

<img src="step9_result1.gif">
<img src="step9_result2.gif">
<img src="step9_result3.gif">
<img src="step9_result4.gif">
<!--
When we start the simulation in Module view mode, we can see how the signals propagate. We can see each signal has a beginning and an end.
Next to the ring, there is a label, that shows the message type.

There's an animation of a VoIP message, that goes from the pedestrian0 to the pedestrian1 through the accessPoint0:

And there's a similar message in 3D view mode, with both option:

Ring signal propagation:

Sphere signal propagation:

-->
Sources: @ref omnetpp.ini, @ref VisualizationNetworks.ned

@nav{step10,step12}
@fixupini

<!------------------------------------------------------------------------>

@page step12 Step 12 - Showing active physical links

@nav{step11,step13}

@section s12goals Goals

In this step we show active physical links in VoIP communication.
These visualize which node receives VoIP packets from the sender device. 
With this visualizer, we verify whether there's physical connection 
between two devices.

<!--
Ebben a lépésben megmutatjuk a VoIP kommunikáció aktív fizikai linkjeit.
Ezek azt mutatják meg, hogy mely eszköz fogad VoIP csomagot a küldő eszköztől. 
Ezzel a visualizer-rel ellenőrizzük, hogy van-e fizikai kapcsolat két eszköz között.
-->

<!--
In this step we want to show the active physical links of the VoIP communication.
Physical links appear, when a node receives VoIP message. With this visualizer
we can monitor which network node transmits VoIP message and which node receives that.
-->

@section s12model The model

<!--
The communication is given, we need to handle the visualizer only.
We can turn off some visualizer, that can confuse us.
Now we have to use the physicalLinkVisualizer. We need to filter
which packets are considered to determine active links, in this case we are curious about
VoIP messages. This visualizer display an arrow from the sender to the receiver.
We can change the color, the width and the style of the arrow, and we can adjust how quickly
inactive links fade away. With the fadeOutMode we can set the base
of the elapsed time.

Here is the configuration:

@dontinclude omnetpp.ini
@skipline [Config Visualization10]
@until ####
-->

@section s12results Results

<img src="step12_phys_link_3d.gif">
<img src="step12_phys_link_2d.gif">
<!--
The VoIP application starts at 1s. Then the pedestrian0 sends the first VoIP message. Because only
the accessPoint0 is in its communication range, only between them appears an arrow. But when the sender
is the accessPoint0, and the destination is the pedestrian1, an array turns up towards
the pedestrian0 too. This happens, because the pedestrian0 is in the accessPoint0's communication
range too, so its wlan NIC also can receive the VoIP packet.The array always points
to the receiver.
-->

Sources: @ref omnetpp.ini, @ref VisualizationNetworks.ned

@nav{step11,step13}
@fixupini

<!------------------------------------------------------------------------>

@page step13 Step 13 - Showing active data links

@nav{step12,step14}

@section s13goals Goals

In this step we display the VoIP communication's data links. Visualizing 
data links we check that the VoIP packet arrived correctly at the receiver.

<!--
Ebben a lépésben a VoIP kommunikáció data linkjeit mutatjuk meg. A data linkek 
vizualizálásával ellenőrizzük, hogy a VoIP csomag hibátlanul megérkezett-e a receiverhez.
-->

@section s13model The model

<!--
The configuration is similar as the physical link visualizer's settings.
We can adjust the same parameters, such as lineColor, lineWidth, lineStyle,
the only difference is, we set the dataLinkVisualizer submodule now.

@note This is because all link visualizer have a common parent class.

Configuration:
@dontinclude omnetpp.ini
@skipline [Config Visualization11]
@until ####
-->

@section s13results Results

<img src="step13_data_link_2d.gif">
<img src="step13_data_link_3d.gif">
<!--
We hide the physicalLinkVisualizer, because it's confusing. When the VoIP
communications starts, and a packet reach the destination an arrow appears
from the sender, towards the receiver. It fades away the same way as physical
links.
-->

Sources: @ref omnetpp.ini, @ref VisualizationNetworks.ned

@nav{step12,step14}
@fixupini

<!------------------------------------------------------------------------>

@page step14 Step 14 - Displaying statistic

@nav{step13,step15}

@section s14goals Goals

We need precise data to monitor working of simulation. These can
be obtained from statistics. In this step we make statistic about VoIP playout delay,
this shows information about VoIP communication.

@note When a router receives a Real-Time Protocol (RTP) audio stream for Voice over IP (VoIP),
it must compensate for the jitter that is encountered. The mechanism that handles this function
is the playout delay buffer. The playout delay buffer must buffer these packets and then play
them out in a steady stream to the digital signal processors (DSPs) to be converted back
to an analog audio stream. The playout delay buffer is also sometimes referred to as the de-jitter buffer.

<!--
A hálózat működésének javításához/monitorozásához pontos adatokra van szükség.
Ezeket statisztikákból tudjuk kinyerni. Ebben a lépésben a playout delayről készítünk
statisztikát, amiből információt kapunk a VoIP kommunikációról.
-->

@section s14model The model

<!--
Communication is still the same. Pedestrian0 sends VoIP stream to pedestrian1 through accessPoint0.
We need to configure only the statisticVisualizer, because we set the adaptive playout true,
when we adjust the VoIP application in step 8. StatisticVisualizer keeps track of the last value of a statistic
for multiple network nodes. <br>
We can look, what kind of signals contain the VoIP application, accurately the SimpleVoIPReceiver,
because the SimpleVoIPSender doesn't contain any signal, so we have to set the
pedestrian1's udp application to the source path. Now we select the playout delay.
It has a signal name and a statistic name. The statisticVisualizer needs these data.
Optional we can set a prefix, that display a string as the prefix of the value. We can add other unit
to the statistic, it is also optional. Because the milliseconds represents better the delay, rather than
the seconds, we add that. We can change the text color, the background and the opacity. They are optional
settings too.

The configuration:
@dontinclude omnetpp.ini
@skipline [Config Visualization12]
@until ####
-->

@section s14results Results

<img src="step14_statistic_3d.gif">
<!--
When we start the simulation here's what happens:

After 1 second the VoIP application starts. After each talk spurt SimpleVoIPReceiver recalculate the playout
because of the adaptive playout setting. After that, the visualizer display the statistic above the pedestrian1
with that font and background color, that we set.
-->

Sources: @ref omnetpp.ini, @ref VisualizationNetworks.ned

@nav{step13,step15}
@fixupini

<!------------------------------------------------------------------------>

@page step15 Step 15 - Showing configured routing tables

@nav{step14,step16}

@section s15goals Goals

In packet switching networks, routing is the higher-level decision making that 
directs network packets from their source towards their destination through 
intermediate network nodes. 
The routing process usually directs forwarding on the basis of routing tables, 
which maintain a record of the routes to various network destinations. 
In this step we show routes from all nodes towards videoStreamServer.

<!--
Csomagkapcsolt hálózatokban a routing határozza meg a csomagtovábbítást, azaz az IP címmel ellátott
csomagok átvitelét a forrás irányából a cél irányába, köztes node-okon keresztül. 
A routing process általában a routing tábla alapján továbbítja a célok felé a csomagokat.
A routing tábla az egyes route-okról tartalmaz információt. 
-->

<!--
After so many steps we extend our network. We add a pedestrian, who watch a video stream in the park.
To this we need a Server to another network. To find ways between video stream server and the
pedestrian, who want to watch video stream, we need a router.<br>
We want to see how can the devices reach the server. To this, in this step we will show routing table entries.
-->

@section s15model The model

<!--
The video streamed by the videoStreamserver, that connects to the router0 through switch0.
We need the switch, because later we want to add more nodes to that subnetwork.

Here is the ned file, that contains the changes:
@dontinclude VisualizationNetworks.ned
@skip network VisualizationC
@until ####

We need to place the new network nodes on the map. To this we use the usual mobility parameters.
Then we adjust the video stream server and client application. On the server side we have to
set the length of the video packets and the full stream, the port to listen on, and the
interval between sending packets.<br>
On the client side we need to add a local port, and the server port and address. Optionally we
can set when the application starts.<br>
The video stream application works as follows:
the client send a request to the given port of the server. Then the server starts the
stream to the client's address. The client's serverPort parameter and the server's localPort parameter
must match.

After setting up communication, we need to configure the visualizer. We have to add
the module path where the visualizer subscribes for routing table signals, and the
route destination(s) in the destinationFilter parameter. In this case we want to
see the routes towards the videoStreamServer. In addition we can change the default
 line style, width and color to make the routes more visible.

The configuration:
@dontinclude omnetpp.ini
@skipline [Config Visualization13]
@until ####
-->

@section s15results Results

<!--
When we start the simulation we can see that, the routingTableVisualizer draw arrows
to represent the routes. This is because by default netmask routes, default routes
and static routes added to routing table. Later we change that.<br>
[img: routes]

After 1 second the VoIP application starts and VoIP data links appear,
because dataLinkVisualizer is on. After 5 seconds the videoPedestrian sends
request to the videoStreamServer and the application starts.
In the Module view mode we can follow the progress. The client send the request,
and in response the server starts the video stream.
[gif: video stream start]
-->

Sources: @ref omnetpp.ini, @ref VisualizationNetworks.ned

@nav{step14,step16}
@fixupini

<!------------------------------------------------------------------------>

@page step16 Step 16 - Displaying 802.11 channel access state

@nav{step15,step17}

@section s16goals Goals

In this step we display how wireless nodes gain access to the channel using
CSMA/CA access method. Visualizing this, we get a clear picture of the state 
of nodes.

<!--
Ebben a lépésben azt mutatjuk meg, hogy hogyan történik a vezeték nélküli
csatorna lefoglalása a node-ok részéről. Ennek a vizualizálásával 
pontos képet kaphatunk arról, hogy melyik node foglalja a csatornát, 
melyik várakozik, hogy adhasson és melyik az, amelyik csak veszi az adást.

Az eszközök CSMA/CA közeghozzáférési módot alkalmaznak, 
aminek az a célja, hogy node-ok ne küldjenek egyszerre csomagokat, ezzel 
ütközést okozva a hálózatban.
A node először "belehallgat" a csatornába és ha úgy érzékeli, hogy éppen 
nincs adás, akkor elkezdi küldeni a saját üzenetét. Ha éppen ad valamelyik 
eszköz a csatornán, akkor véletlenszerű időtartam után
-->
<!--
CSMA/CA in computer networking, is a network multiple access method
in which carrier sensing is used, but nodes attempt to avoid collisions
by transmitting only when the channel is sensed to be <i>"idle"</i>. That operates in
data link layer (Layer 2).

Network nodes can be in different channel access states like <i>idle</i>,
<i>owning</i>, <i>ifs+back off</i>, that show, who transmit on the channel
and who listen. We want to display that in this step.
-->

@section s16model The model

<!--
Firstly we hide some visualizers, because they are distracting.
The communication is the same as in the previous step, we have to configure only the visualizer.
To display the channel access states, we use infoVisualizer. <br>
Here is the configuration:
@dontinclude omnetpp.ini
@skipline [Config Visualization14]
@until ####

The module parameter specifies the submodules of network nodes, and the content
determines what is displayed on network nodes. In addition we can adjust the
background color, the font color, and the opacity. These are optional settings.
-->

@section s16results Results

<img src="step16_channel_access_2d.gif">
<!--
Here's what happens, when the simulation is running:
[gif simulation is running]

We see, the nodes wait until the channel is sensed to be idle. If the medium is clear,
instead of immediately transmitting network nodes are waiting a predefined amount of time.
This waiting period is called the interframe spacing (IFS).
It depends on the priority of the packet.
In addition to having a different IFS, a station will add a "random backoff"
to its waiting period, to reduce the collision probability.
After that the the network node starts transmitting the data, and it's owning the channel.
-->

Sources: @ref omnetpp.ini, @ref VisualizationNetworks.ned

@nav{step15,step17}
@fixupini

<!------------------------------------------------------------------------>

@page step17 Step 17 - Showing active network routes

@nav{step16,step18}

@section s17goals Goals

In this step we enable displaying active network routes for video stream. 
In network devices, applications handle the UDP based traffic, for example video stream. 
When a video stream packet leaves the network layer in the destination node, 
network route becomes active between the source node and the destination. 
With this visualizer we check, whether the videostream packet arrived at the 
destination's client application.

<!--
Ebben a lépésben az aktív hálózati útvonalakat jelenítjük meg.
Az eszközökben alkalmazások kezelik a TCP, illetve UDP alapú forgalmat, mint
pl a videoStream-et vagy a VoIP-ot. Amikor a video vagy VoIP csomag elhagyja 
a fogadó eszközben a hálózati réteget, a két eszköz között aktívvá válik 
a hálózati útvonal. Ezzel a visualizer-rel megnézhetjük, hogy a csomag 
eljut-e a célállomás megfelelő alkalmazásához.
-->

<!--
In this step we want to show active network routes. It's similar to
showing active physical links, and active data links, but network routes
active when the packet pass the destination' network layer.

In furthermore we make the model more interesting by adding more nodes,
change the routing protocol to RIP, and assign address via DHCP protocol
to the wireless nodes.
-->
@section s17model The model

<!--
Firstly we have to edit the configurator. We make an xml file (in this case configurationD.xml),
to set the static ip addresses. Static addresses are the routers' interfaces and
the videoStreamServer's IP address.

@dontinclude configurationD.xml
@skip config
@until /config

The routers assign addresses to wireless nodes via DHCP.
To that we have to turn on the hasDHCP parameter. Then we adjust the
other settings of that service. We have to set which interface assign the addresses.
In our simulation it's the "eth0" on both router. MaxNumClients parameter adjusts
maximum number of clients (IPs) allowed to be leased (in our simulation we set to 10)
and numReservedAddresses define number of addresses to skip
at the start of the network's address range. To gateway we add that interface's IP address,
that run the DHCP service. Finally we have to add the lease time.
We can adjust the start time, but usually we want that, DHCP service run the
beginning, so we leave it on 0s.

If we want RIP routing protocol work, we have to set true the routers' hasRIP parameter,
and set to false the configurator.optimizeRoutes parameter.

The configuration:
@dontinclude omnetpp.ini
@skipline [Config Visualization15]
@until ####
-->

@section s17results Results



Sources: @ref omnetpp.ini, @ref VisualizationNetworks.ned

@nav{step16,step18}
@fixupini

<!------------------------------------------------------------------------>

@page step18 Step 18 - Displaying physical environment

@nav{step17,step19}

@section s18goals Goals

In the real world the objects on the map, such as buildings or trees constitute 
3 dimensional barriers which affects the wireless communication. In this step 
we add 3 dimensional obstacles to our simulation.

<!--
A valós környezetben a tárgyak a térképen (épületek, fák) 3 dimenziós   
akadályokat képeznek, amiknek hatása van a vezeték nélküli kommunikációra. 
Ebben a lépésben akadályokat adunk hozzá a szimulációhoz.
-->

@section s18model The model

@section s18results Results

Sources: @ref omnetpp.ini, @ref VisualizationNetworks.ned

@nav{step17,step19}
@fixupini

<!------------------------------------------------------------------------>

@page step19 Step 19 - Displaying obstacle loss

@nav{step18,step20}

@section s19goals Goals

Impediments shield the signals of wireless communication. In this step we 
show the obstacle loss.
<!--
Az akadályok árnyékolják a vezeték nélküli kommunikáció jeleit.
Ebben a lépésben az akadályokon eső veszteségeket mutatjuk meg.
-->

@section s19model The model

@section s19results Results

Sources: @ref omnetpp.ini, @ref VisualizationNetworks.ned

@nav{step18,step20}
@fixupini

<!------------------------------------------------------------------------>

@page step20 Step 20 - Showing packet drops

@nav{step19,step21}

@section s20goals Goals

In this step we visualize packet drops. Packet drops occurs when one or more 
packets of data travelling across a computer network fail to reach their destination. 
Packet loss is typically caused by network congestion but it can be caused by 
a number of other factors such as radio signals that are too weak due to distance, 
natural or artifical obstacles in the environment or faulty networking hardware. 
It helps to put wireless access points to the right place, if we see where the packets dropped.

<!--
Ebben a lépésben a packet dropot vizualizáljuk.
A packet drop akkor történik, amikor egy vagy több csomag nem ér oda a célhoz. 
Általában hálózati torlódás okozza, de gyenge rádiójel vagy hibás hw is okozhatja. 
Ha természeti vagy mesterséges akadályok vannak a környezetben, akkor segítheti a
wireless AP-k megfelelő helyre telepítését, ha látjuk, hogy hol történik csomag vesztés.
-->

@section s20model The model

@section s20results Results

Sources: @ref omnetpp.ini, @ref VisualizationNetworks.ned

@nav{step19,step21}
@fixupini

<!------------------------------------------------------------------------>

@page step21 Step 21 - Displaying transport connections

@nav{step20,step22}

@section s21goals Goals

In this step we show transport connections. To establish a link, TCP uses 
three-way handshake. It's open until one of the participants initiate closing the 
connection. Displaying these, it will be easy to understand which hosts are the 
participants of a transport connection.

<!--
Ebben a lépésben a nyitott hálózati kapcsolatokat jelenítjük meg.
A TCP kapcsolatok háromfázisú kézfogással épülnek ki. Ezután amíg az egyik fél 
nem kezdeményezi a kapcsolat lezárását, addig nyitva marad. A megjelenítésükkel 
könnyen átlátható lesz, hogy melyik nyitott kapcsolatnak melyik node-ok a résztvevői.
-->

@section s21model The model

@section s21results Results

Sources: @ref omnetpp.ini, @ref VisualizationNetworks.ned

@nav{step20,step22}
@fixupini

<!------------------------------------------------------------------------>

@page step22 Step 22 - Showing link breaks

@nav{step21,step23}

@section s22goals Goals

In this step we display link breaks between network devices. When a mobile node 
exits from an access point's communication range the link breaks up and the devices 
can't communicate with each other. This may results packet loss when the mobile 
node receives any stream.

<!--
Ebben a lépésben a link break-eket mutatjuk meg. 
Ha egy mozgó node kimegy egy access point communication range-éből, akkor a kapcsolat 
megszakad az eszközök között és nem lesznek képesek a kommunikációra egymással. Ennek 
csomagvesztés lehet a következménye, ha pl. a mozgó eszköz valamilyen stream-et fogad.
-->

@section s22model The model

@section s22results Results

Sources: @ref omnetpp.ini, @ref VisualizationNetworks.ned

@nav{step21,step23}
@fixupini

<!------------------------------------------------------------------------>

@page step23 Step 23 - Visualizing handovers

@nav{step22,step24}

@section s23goals Goals



<!--
Ebben a lépésben a wireless handover-t (hand off-ot) fogjuk szimulálni. A vezeték 
nélküli hálózatokban gyakran fedik egymást az AP-k communication range-ei, hogy a 
mozgó endpoint-ok minél nagyobb területen használhassák a hálózatot.
A hálózat működésének megértését könnyíti, ha látjuk az átkapcsolásokat az access 
pointok között.

@note Ez a jelenség akkor történik, amikor egy mozgó wireless eszköz access point-ok egymást fedő 
communication range-ében van. Ha annak az AP-nek amelyikhez a node csatlakozva van 
a jele túl gyenge a kommunikációhoz és egy másik AP-é elég erős, akkor az eszköz 
át fog csatlakozni a másik AP-re.
-->

@section s23model The model

@section s23results Results

Sources: @ref omnetpp.ini, @ref VisualizationNetworks.ned

@nav{step22,step24}
@fixupini

<!------------------------------------------------------------------------>

@page step24 Step 24 - Displaying changing routes

@nav{step23,conclusion}

@section s24goals Goals



<!--
Amikor az eszközök mozognak, gyakran változhat két node között a csomagok útja. 
A változások megjelenítésével bármelyik pillanatban meg tudjuk állapítani, hogy az 
eszközök milyen útvonalat használnak a kommunikációra.
-->

@section s24model The model

@section s24results Results

Sources: @ref omnetpp.ini, @ref VisualizationNetworks.ned

@nav{step23,conclusion}
@fixupini

<!------------------------------------------------------------------------>

@page conclusion Conclusion

@nav{step24,index}

Sources: @ref omnetpp.ini, @ref VisualizationNetworks.ned

@nav{step24,index}
@fixupini

<!------------------------------------------------------------------------>*/

/**
@mainpage Visualization Tutorial for the INET Framework -- Part 1
In this tutorial we show you, how to use the visualization module in the
INET Framework. The tutorial contains a series of simulation models,
numbered from 1 through 22. The models are of increasing complexity –
they start from the basics and in each step,
they introduce new INET features and concepts related to visualization module.
Our goal is show the working of the visualization module and its submodules in INET.

This is an advanced tutorial, and it assumes that you are familiar with creating
and running simulations in @opp and  INET. If you aren't, you can check out
the <a href="https://omnetpp.org/doc/omnetpp/tictoc-tutorial/" target="_blank">
TicToc Tutorial</a> to get started with using @opp.
The <a href="../../../doc/walkthrough/tutorial.html" target="_blank">
INET Walkthrough</a> is an introduction to INET and working with protocols.

If you need more information at any time, feel free to refer to the @opp
and INET documentation:

- <a href="https://omnetpp.org/doc/omnetpp/manual/usman.html" target="_blank">@opp User Manual</a>
- <a href="https://omnetpp.org/doc/omnetpp/api/index.html" target="_blank">@opp API Reference</a>
- <a href="https://omnetpp.org/doc/inet/api-current/inet-manual-draft.pdf" target="_blank">INET Manual draft</a>
- <a href="https://omnetpp.org/doc/inet/api-current/neddoc/index.html" target="_blank">INET Reference</a>

In the tutorial, each step is a separate configuration in the same omnetpp.ini file.
Steps build on each other, they extend the configuration of the previous step
by adding a few new lines. Consecutive steps mostly share the same network, defined in NED.

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
- @ref conclusion

@nav{index,step1}
@fixupini

<!------------------------------------------------------------------------>

@page step1 Step 1 -Showing Boston downtown

@nav{index,step2}

@section s1goals Goals
In the first step we want to show how the scene visualizer works.
We'll display a real map of Boston. This will be the basis of all subsequent steps.

@section s1model The model

Here is the NED file of the network:
@dontinclude VisualizationNetworks.ned
@skip network VisualizationA
@until ####

This is a very simple model, contains an IntegratedVisualizer and an
OsgGeographicCoordinateSystem submodule.
The visualizer is responsible all of phenomenon, that we can see on the playground.
We can change their parameters in the `omnetpp.ini` file.
The ini file contains the parameters of these submodules.

@dontinclude omnetpp.ini
@skipline [Config Visualization01]
@until ####

In this part of the file, there are some coordinate system
and the scene visualizer parameters.
Coordinate system parameters set the longitude, latitude and altitude coordinate
of the playground origin and the heading of it.
Scene visualizer parameters set the visualizer type. We add a map to the simulation
with the mapFile parameter.
Besides these options we turn off the playground shading and configure the opacity
and the color of the playground. We need to make the playground transparent,
because it's over the map, and later if we place a node, we can't see the map
under of the area from the origin to the node.
With the axisLength we can change the axis' size on the map.

@section s1results Results

When we start the simulation we can see what we expected. There's the map of Boston downtown with the axis in that size, that we configure.
<img src="step1_result1.png" width="850">

Using the mouse, we can move and rotate the camera. If we hold down the left mouse button, we can navigate on the map. Holding down the mouse wheel or both mouse button at the same time we can rotate the camera,
and if we scroll up and down we can zoom out and in. If we click with the right mouse button, we can change between camera modes.
In the top right corner of the playground, we can change between 3D Scene view mode and Module view.

Sources: @ref omnetpp.ini, @ref VisualizationNetworks.ned

@nav{index,step2}
@fixupini

<!------------------------------------------------------------------------>

@page step2 Step 2 - Adding static 3D objects to the scene (WIP)

@nav{step1,step3}

@section s2goals Goals

@section s2model The model

@section s2results Results

Sources: @ref omnetpp.ini, @ref VisualizationNetworks.ned

@nav{step1,step3}
@fixupini

<!------------------------------------------------------------------------>

@page step3 Step 3 - Displaying communication/interference range

@nav{step2,step4}

@section s3goals Goals

In this step we extend our model with two pedestrians, and an access point.
Later we want to see communication between them,
so we have to place them in each other's communication range.
Our goal is the visualization of these ranges to make the placing easier.

@section s3model The model

This is our extended network file:

@dontinclude VisualizationNetworks.ned
@skip network VisualizationB
@until ####

To achieve our goal, we need to add two pedestrians, and an access point to the network.
To communicate with each other, we need an IPv4NetworkConfigurator and
an Ieee80211ScalarRadioMedium submodule. The configurator prepares the network nodes
to the communication, the radioMedium manages the media.

In the ini file, we adjust the transmission power of the network nodes.
We have to do that, because by using the default transmission power parameter,
the ranges will be too big. The access point's transmission power is bigger.
We can modify the ranges' color with the communicationRangeColor and interferenceRangeColor parameters.
Now we leave them on the default value: the communication range is blue
and the interference range color is grey.
Below, there is the appropriate part of the ini file:

@dontinclude omnetpp.ini
@skipline [Config Visualization03]
@until ####

@section s3results Results

If we run the simulation in the 3D Scene view mode, we can see the three nodes
and three circles around them. Each node is in the center of a circle,
that circle is the node's communication range.
<img src="step3_result1.png">

We configured the visualization of interference ranges too. They're also on the map,
but they're very big, so we have to zoom out
or move to any direction to see these ranges.
The communication and interference ranges seen in the Module view mode too.
<img src="step3_result2.png">

When we run the simulation, the pedestrians associate with the access point.
In Module view mode there's a bubble message when it happens.

Sources: @ref omnetpp.ini, @ref VisualizationNetworks.ned

@nav{step2,step4}
@fixupini

<!------------------------------------------------------------------------>

@page step4 Step 4 - Using 3D models for network nodes

@nav{step3,step5}

@section s4goals Goals

Pedestrians are WirelessHost node type, so by default their icon is a cellphone.
We want to show, how to change network nodes' default icon.

@section s4model The model

In INET Framework it's possible to change device appearance to an external 3D osg model.
It's really simple. We have to change only the network node's
osgModel attribute. We set that option to boxman.osgb.
It's the 3D model name. In addition we can set the size and the rotation of the model.
The "(0.06).scale" means the model size is 6% of the original.
The three numbers are in for the rot keyword mean the rotation of the 3D model around x, y and z axis.

@dontinclude omnetpp.ini
@skipline [Config Visualization04]
@until ####

@section s4results Results

In Module view mode there's no difference compared to the simulation before this.
But in 3D view mode instead of phones we see walker boxmans.
<img src="step4_result1.gif">

Sources: @ref omnetpp.ini, @ref VisualizationNetworks.ned

@nav{step3,step5}
@fixupini

<!------------------------------------------------------------------------>


@page step5 Step 5 - Displaying recent movement

@nav{step4,step6}

@section s5goals Goals

In this step we want to show how can visualize mobile network nodes' movement.
The pedestrians' movement handled by the mobility submodule.
To visualize that, we need to use the mobility visualizer.

@section s5model The model

Here is the configuration:

@dontinclude omnetpp.ini
@skipline [Config Visualization05]
@until ####

We need to adjust the nodes' mobility settings. The pedestrians movement
is calculated using "MassMobility". This is a random mobility model for a mobile host with a mass.
We set their initial position, and a border, because we don't want to let them go out
from the access point's communication range.
We have more ways to set the nodes initial position.
We can set that in meter or we can add that in degree.
The pedestrians' movement based on three parameters. The changeInterval is the frequency
of changing speed and angle, the changeAngleBy change the direction of the movement,
and the speed means the movement speed.

After that we need to add some visualizer parameters. We can display the movement of the pedestrians.
We display a trail, that shows where the pedestrians came from, and we show an arrow,
that shows the velocity of the pedestrians, but it seems that in Module view mode only.

@section s5results Results

If we run the simulation, here's what happens. The pedestrians roam in the park between
invisible borders that we adjust to them.

Here's that in Module view mode:
<img src="step5_result1.gif">

That's what we can see in 3D Scene view mode:
<img src="step5_result3.gif" width="850">

These animations created in fast run mode, because the normal speed is too slow to see the pedestrians movement.

Sources: @ref omnetpp.ini, @ref VisualizationNetworks.ned

@nav{step4,step6}
@fixupini

<!------------------------------------------------------------------------>

@page step6 Step 6 - Showing IP adresses

@nav{step5,step7}

@section s6goals Goals

In this step we want to show how to print out data of any given host's
any given network interface card (NIC).

@section s6model The model

If we want to see the network nodes' IP addresses we have to set
the parameters of the interfaceTableVisualizer. Here is the configuration:

@dontinclude omnetpp.ini
@skipline [Config Visualization06]
@until ####

With the nodeFilter parameter we can define the nodes list, that are considered.
By default that list is empty, so we need to set it.
Then we can set an interface list. That specifies which interfaces are considered
at each node. Besides of these parameters we can change the font color,
the background color, and the opacity of the text. These settings are optional,
that may make the IP addresses clearly visible.

With interfaceTableVisualizer we can display not only the IP address of an interface,
but the MAC address too. To that we need to change the content parameter to "macAddress".

@section s6results Results

If we run the simulation, we can see a yellow bubble above each pedestrian
with its wlan NIC IP address.
<img src="step6_result5.gif">

In 3D view mode the text is a little bit fainter.
<img src="step6_result4.gif"  width="850">

If we set the content parameter to "macAddress", we can see
the given NIC MAC (layer 2) address.
<img src="step6_result3.png" width="850">

Sources: @ref omnetpp.ini, @ref VisualizationNetworks.ned

@nav{step5,step7}
@fixupini

<!------------------------------------------------------------------------>

@page step7 Step 7 - Showing wifi association

@nav{step6,step8}

@section s7goals Goals

The pedestrians are in the access point's communication range,
so they can associate with that.

@section s7model The model

In the ini file we need to set only the ieee80211Visualizer's parameters.
This visualizer will display us information about the association.
We can set which node's and which interfaces are considered, like at interfaceTableVisualizer.

Here is the appropriate configuration:

@dontinclude omnetpp.ini
@skipline [Config Visualization07]
@until ####

@section s7results Results

In the Module view mode we can monitor the association process,
we can see all messages between the nodes. When a pedestrian send an Assoc message,
the access point in its communication range receive that,
and a signal sign appear above the access point. In response to this Assoc message,
the access point reply with an AssocResp- message. If the association is successful
it's AssocResp-OK and a signal sign appear above that pedestrian
who wants to associate with the access point.
<img src="step7_result1.gif">

In 3D view mode as a result of the association process the signal sing appears
above the appropriate network node.
<img src="step7_result2.gif">


Sources: @ref omnetpp.ini, @ref VisualizationNetworks.ned

@nav{step6,step8}
@fixupini

<!------------------------------------------------------------------------>

@page step8 Step 8 - Displaying transmissions and receptions

@nav{step7,step9}

@section s8goals Goals

We know that, pedestrians are associate with the access point.
We want to simulate a VoIP communication between them,
but in 3D view mode, we can't see any communication.
We want to show, how can display, who transmits, and who receives signals at a given moment.

@section s8model The model

After the successful association process we can start the VoIP application between the pedestrians,
so we need to add one udp application to them.
The pedestrian0 will be the sender and the pedestrian1 will be the receiver.
They communicate with udp over port 5000. The application starts at 1 second.
We leave all other options at their default values at the sender side. At the receiver side
we need to set the port to 5000, and we turn on the adaptive playout setting. It will be used later.

We need to turn on some mediumVisualizer parameters.
We set true the displayTransmissions and the displayReceptions options.
We have to set an image to these options to display them.

Configuration:

@dontinclude omnetpp.ini
@skipline [Config Visualization08]
@until ####

@section s8results Results

If we start the simulation, we can see clearly, who is the transmitter, and who are the receivers.
The signs appear, when a signal arrives or leaves the wlan NIC.

<img src="step8_result1.gif">

Sources: @ref omnetpp.ini, @ref VisualizationNetworks.ned

@nav{step7,step9}
@fixupini

<!------------------------------------------------------------------------>

@page step9 Step 9 - Showing propagating signals

@nav{step8,step10}

@section s9goals Goals

The communication works, we can see who is the sender and the receiver, but we don't see the signal.
In this step we want to display the signal propagation.

@section s9model The model

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

@section s9results Results

When we start the simulation in Module view mode, we can see how the signals propagate. We can see each signal has a beginning and an end.
Next to the ring, there is a label, that shows the message type.

There's an animation of a VoIP message, that goes from the pedestrian0 to the pedestrian1 through the accessPoint0:
<img src="step9_result1.gif">

And there's a similar message in 3D view mode, with <i>both</i> option:
<img src="step9_result2.gif">

<i>Ring</i> signal propagation:
<img src="step9_result3.gif">

<i>Sphere</i> signal propagation:
<img src="step9_result4.gif">

Sources: @ref omnetpp.ini, @ref VisualizationNetworks.ned

@nav{step8,step10}
@fixupini

<!------------------------------------------------------------------------>

@page step10 Step 10 - Showing active physical links

@nav{step9,step11}

@section s10goals Goals

In this step we want to show the active physical links of the VoIP communication.
Physical links appear, when a node receives VoIP message. With this visualizer
we can monitor which network node transmits VoIP message and which node receives that.

@section s10model The model

The communication is given, we need to handle the visualizer only.
We can turn off some visualizer, that can confuse us.
Now we have to use the physicalLinkVisualizer. We need to filter
which packets are considered to determine active links, in this case we are curious about
VoIP messages. This visualizer display an arrow from the sender to the receiver.
We can change the color, the width and the style of the arrow, and we can adjust how quickly
inactive links fade away. With the <i>fadeOutMode</i> we can set the base
of the elapsed time.

Here is the configuration:

@dontinclude omnetpp.ini
@skipline [Config Visualization10]
@until ####

@section s10results Results

The VoIP application starts at 1s. Then the pedestrian0 sends the first VoIP message. Because only
the accessPoint0 is in its communication range, only between them appears an arrow. But when the sender
is the accessPoint0, and the destination is the pedestrian1, an array turns up towards
the pedestrian0 too. This happens, because the pedestrian0 is in the accessPoint0's communication
range too, so its wlan NIC also can receive the VoIP packet.The array always points
to the receiver.

Sources: @ref omnetpp.ini, @ref VisualizationNetworks.ned

@nav{step9,step11}
@fixupini

<!------------------------------------------------------------------------>

@page step11 Step 11 - Showing active data links

@nav{step10,step12}

@section s11goals Goals

In this step we want to display the active data links. Data links are shown,
when a network node receives a packet and it has to do something with it.
For example its Layer 2 address is the packet's destination or
the device has to switch towards the destination.

@section s11model The model

The configuration is similar as the physical link visualizer's settings.
We can adjust the same parameters, such as lineColor, lineWidth, lineStyle,
the only difference is, we set the <i>dataLinkVisualizer</i> submodule now.

@note This is because all link visualizer have a common parent class.

Configuration:
@dontinclude omnetpp.ini
@skipline [Config Visualization11]
@until ####

@section s11results Results

We hide the physicalLinkVisualizer, because it's confusing. When the VoIP
communications starts, and a packet reach the destination an arrow appears
from the sender, towards the receiver. It fades away the same way as physical
links.

Sources: @ref omnetpp.ini, @ref VisualizationNetworks.ned

@nav{step10,step12}
@fixupini

<!------------------------------------------------------------------------>

@page step12 Step 12 - Displaying statistic

@nav{step11,step13}

@section s12goals Goals

The VoIP application is working right now. We want to create a statistic from that,
and print it out to the screen. Our statistic will be the playout delay.

@note When a router receives a Real-Time Protocol (RTP) audio stream for Voice over IP (VoIP),
it must compensate for the jitter that is encountered. The mechanism that handles this function
is the playout delay buffer. The playout delay buffer must buffer these packets and then play
 them out in a steady stream to the digital signal processors (DSPs) to be converted back
 to an analog audio stream. The playout delay buffer is also sometimes referred to as the de-jitter buffer.

@section s12model The model

Communication is still the same. Pedestrian0 sends VoIP stream to pedestrian1 through accessPoint0.
We need to configure only the <i>statisticVisualizer</i>, because we set the adaptive playout true,
when we adjust the VoIP application in step 8. <i>StatisticVisualizer</i> keeps track of the last value of a statistic
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

@section s12results Results

When we start the simulation here's what happens:

[gif]

After 1 second the VoIP application starts. After each talk spurt <i>SimpleVoIPReceiver</i> recalculate the playout
because of the adaptive playout setting. After that, the visualizer display the statistic above the pedestrian1
with that font and background color, that we set.

Sources: @ref omnetpp.ini, @ref VisualizationNetworks.ned

@nav{step11,step13}
@fixupini

<!------------------------------------------------------------------------>

@page step13 Step 13 -

@nav{step12,step14}

@section s13goals Goals

@section s13model The model

@section s13results Results

Sources: @ref omnetpp.ini, @ref VisualizationNetworks.ned

@nav{step12,step14}
@fixupini

<!------------------------------------------------------------------------>

@page step14 Step 14 -

@nav{step13,step15}

@section s14goals Goals

@section s14model The model

@section s14results Results

Sources: @ref omnetpp.ini, @ref VisualizationNetworks.ned

@nav{step13,step15}
@fixupini

<!------------------------------------------------------------------------>

@page step15 Step 15 -

@nav{step14,step16}

@section s15goals Goals

@section s15model The model

@section s15results Results

Sources: @ref omnetpp.ini, @ref VisualizationNetworks.ned

@nav{step14,step16}
@fixupini

<!------------------------------------------------------------------------>

@page step16 Step 16 -

@nav{step15,step17}

@section s16goals Goals

@section s16model The model

@section s16results Results

Sources: @ref omnetpp.ini, @ref VisualizationNetworks.ned

@nav{step15,step17}
@fixupini

<!------------------------------------------------------------------------>

@page step17 Step 17 -

@nav{step16,step18}

@section s17goals Goals

@section s17model The model

@section s17results Results

Sources: @ref omnetpp.ini, @ref VisualizationNetworks.ned

@nav{step16,step18}
@fixupini

<!------------------------------------------------------------------------>

@page step18 Step 18 -

@nav{step17,step19}

@section s18goals Goals

@section s18model The model

@section s18results Results

Sources: @ref omnetpp.ini, @ref VisualizationNetworks.ned

@nav{step17,step19}
@fixupini

<!------------------------------------------------------------------------>

@page step19 Step 19 -

@nav{step18,step20}

@section s19goals Goals

@section s19model The model

@section s19results Results

Sources: @ref omnetpp.ini, @ref VisualizationNetworks.ned

@nav{step18,step20}
@fixupini

<!------------------------------------------------------------------------>

@page step20 Step 20 -

@nav{step19,step21}

@section s20goals Goals

@section s20model The model

@section s20results Results

Sources: @ref omnetpp.ini, @ref VisualizationNetworks.ned

@nav{step19,step21}
@fixupini

<!------------------------------------------------------------------------>

@page step21 Step 21 -

@nav{step20,step22}

@section s21goals Goals

@section s21model The model

@section s21results Results

Sources: @ref omnetpp.ini, @ref VisualizationNetworks.ned

@nav{step12,conclusion}
@fixupini

<!------------------------------------------------------------------------>

@page step22 Step 22 -

@nav{step21,conclusion}

@section s22goals Goals

@section s22model The model

@section s22results Results

Sources: @ref omnetpp.ini, @ref VisualizationNetworks.ned

@nav{step21,conclusion}
@fixupini

<!------------------------------------------------------------------------>

@page conclusion Conclusion -

@nav{step22,index}

Sources: @ref omnetpp.ini, @ref VisualizationNetworks.ned

@nav{step22,index}
@fixupini

<!------------------------------------------------------------------------>*/

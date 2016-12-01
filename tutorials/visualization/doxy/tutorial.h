/**
@mainpage Visualization Tutorial for the INET Framework -- Part 1
In this tutorial we show you, how to use the visualization module in the INET Framework. The tutorial contains a series of simulation models,
numbered from 1 through 22. The models are of increasing complexity – they start from the basics and in each step,
they introduce new INET features and concepts related to visualization module.
Our goal is show the working of the visualization module and its submodules in INET.

This is an advanced tutorial, and it assumes that you are familiar with creating
and running simulations in @opp and  INET. If you aren't, you can check out
the <a href="https://omnetpp.org/doc/omnetpp/tictoc-tutorial/" target="_blank">TicToc Tutorial</a> to get started with using @opp.
The <a href="../../../doc/walkthrough/tutorial.html" target="_blank">INET Walkthrough</a> is an introduction to INET and working with protocols.

If you need more information at any time, feel free to refer to the @opp and INET documentation:

- <a href="https://omnetpp.org/doc/omnetpp/manual/usman.html" target="_blank">@opp User Manual</a>
- <a href="https://omnetpp.org/doc/omnetpp/api/index.html" target="_blank">@opp API Reference</a>
- <a href="https://omnetpp.org/doc/inet/api-current/inet-manual-draft.pdf" target="_blank">INET Manual draft</a>
- <a href="https://omnetpp.org/doc/inet/api-current/neddoc/index.html" target="_blank">INET Reference</a>

In the tutorial, each step is a separate configuration in the same omnetpp.ini file.
Steps build on each other, they extend the configuration of the previous step by adding a few new lines. Consecutive steps mostly share the same network,
defined in NED.

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
In the first step we want to show how the scene visualizer works. We'll display a real map of Boston.

@section s1model The model

Here is the NED file of the network:
@dontinclude VisualizationNetworks.ned
@skip network VisualizationA
@until ####


This is a very simple model, contains an IntegratedVisualizer and an OsgGeographicCoordinateSystem submodule.
The ini file contains the parameters of these submodules.

@dontinclude omnetpp.ini
@skipline [Config Visualization01]
@until ####

In this part of the file, there are some coordinate system and the scene visualizer parameters.
Coordinate system parameters set the longitude, latitude and altitude coordinate of the playground origin and the heading of it.
Scene visualizer parameters set the visualizer type. We add a map to the simulation with the mapFile parameter.
Besides these options we turn off the playground shading and configure the opacity and the color of it.
With the axisLength we can change the axis' size on the map.

@section s1results Results

When we start the simulation we can see what we expected. There's the map of Boston downtown with the axis in that size, that we configure.

<img src="step1_result1.png" width="850">

If we hold down the left mouse button, we can navigate on the map. Holding down the mouse wheel or both mouse button at the same time we can rotate the camera,
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

@page step3 Step 3 - Displaying communiation/interference range

@nav{step2,step4}

@section s3goals Goals

In this step we extend our model with two pedestrians, and an access point. Later we want to see the communication between them,
so we have to place them in each other's communication range. Our goal is the visualization of these ranges.

@section s3model The model

This is our extended network file:

@dontinclude VisualizationNetworks.ned
@skip network VisualizationB
@until ####

To achieve our goal, we need to add two pedestrians, and an access point to the network. To communicate with each other, we need an IPv4NetworkConfigurator and
an Ieee80211ScalarRadioMedium submodule. The configurator prepares the network nodes to the communication, the radioMedium manages the media.

In the ini file, we adjust the transmission power of the network nodes. The access point's transmission power is bigger. We have to do that,
because by using the default transmission power parameter, the ranges will be too big by.
Then we turn on the ranges.
We can modify their color with the communicationRangeColor and interferenceRangeColor parameters.
Now we set the communication ranges' color to red (by default it's blue), and the interference ranges' color to black (by default it's gray).
Below, there is the appropriate part of the ini file.

@dontinclude omnetpp.ini
@skipline [Config Visualization03]
@until ####

@section s3results Results

If we run the simulation in the 3D Scene view mode, we can see the three nodes and three circles around them. Each node is in the center of a circle,
that circle is the node's communication range.

<img src="step3_result1.png"> WIP

We configured the visualization of interference ranges too. They're also on the map, but they're very big, so we have to zoom out
or move to any direction to see these ranges.
The communication and interference ranges seem in the Module view mode too.

<img src="step3_result2.png"> WIP

When we run the simulation, the pedestrians associate with the access point.
In Module view mode there's a bubble message when it happens.

Sources: @ref omnetpp.ini, @ref VisualizationNetworks.ned

@nav{step2,step4}
@fixupini

<!------------------------------------------------------------------------>

@page step4 Step 4 - Using 3D models for network nodes

@nav{step3,step5}

@section s4goals Goals

Pedestrians are WirelessHost type, so by default their icon is a cellphone. Our goal is changing that a model, which is more similar to humans.

@section s4model The model

In INET Framework it's possible to change device appearance to an external 3D osg model. The osgModel's first attribute is the boxman.osgb.
It's the 3D model name. In addition we can set the size and the rotation of the model. The "(0.06).scale" means the model size is 6% of the original.
The three numbers are in for the rot key word mean the rotation of the 3D model around x, y and z axis.

@dontinclude omnetpp.ini
@skipline [Config Visualization04]
@until ####

@section s4results Results

In Module view mode there's no difference compared to the simulation before this. But in 3D view mode instead of phones we see boxmans.

<img src="step4_result1.gif"> WIP

Sources: @ref omnetpp.ini, @ref VisualizationNetworks.ned

@nav{step3,step5}
@fixupini

<!------------------------------------------------------------------------>


@page step5 Step 5 - Displaying recent movement and orientation

@nav{step4,step6}

@section s5goals Goals

In this step we want to show how can mobile network nodes move. To achieve our goal, we need to use the mobility visualizer submodule.

@section s5model The model

Here is the configuration:

@dontinclude omnetpp.ini
@skipline [Config Visualization05]
@until ####

We have to adjust the nodes' mobility settings. The pedestrians movement is calculated using "MassMobility".
This is a random mobility model for a mobile host with a mass.
We set their initial position, and a border, because we don't want to let them go out from the access point's communication range.
We have more ways to set the nodes initial position.
We can set that in meter, in this case the origin position counts, and we can add that in degree.
The pedestrians' movement based on three parameters. EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEThe changing interval is the frequency of changing speed and angle,
the change angle by change the direction of the movement, and the speed means the movement speed.

After that we need to add some visualizer parameters. We can display the movement of the pedestrians. We display a trail,
that shows where the pedestrians come from, and we can show an arrow,
that shows the velocity of the pedestrians, but we can see that in Module view mode only.

@section s5results Results

If we run the simulation, here's what happens. The pedestrians walk between borders that we adjust to them.

<img src="step5_result1.gif"> WIP

<img src="step5_result3.gif" width="850"> WIP

Sources: @ref omnetpp.ini, @ref VisualizationNetworks.ned

@nav{step4,step6}
@fixupini

<!------------------------------------------------------------------------>

@page step6 Step 6 - Showing IP adresses

@nav{step5,step7}

@section s6goals Goals

In this step we want to show the wireless hosts IP addresses.

@section s6model The model

If we want to see the network nodes' IP addresses we have set the parameters of the interfaceTableVisualizer. Here is the configuration:

@dontinclude omnetpp.ini
@skipline [Config Visualization06]
@until ####

With the nodeFilter parameter we can define the nodes list, that are considered. If we want to see any IP address, we have to set it,
because by default that list is empty. Then we can set an interface list. That specifies what interfaces are considered at each node.
Besides of these parameters we can change the font color, the background color, and the opacity of the text.  These are optional looking parameters,
that make the IP addresses clearly visible.

With interfaceTableVisualizer we can display not only the IP address of an interface, but the MAC address too.
To that we need to change the content parameter to "macAddress".

@section s6results Results

If we run the simulation, we can see a yellow bubble above each pedestrian with its wlan0 network interface card (NIC) IP address.

<img src="step6_result1.png">

<img src="step6_result2.png">

If we set the content parameter to "macAddress", we can see the given NIC MAC (layer 2) address.

<img src="step6_result3.png" width="850">

<img src="step6_result4.gif" width="850">

Sources: @ref omnetpp.ini, @ref VisualizationNetworks.ned

@nav{step5,step7}
@fixupini

<!------------------------------------------------------------------------>

@page step7 Step 7 - Showing wifi association

@nav{step6,step8}

@section s7goals Goals

The pedestrians are in the access point communication range, but they need to associate with that, if they want to talk to each other.
When do they associate with the access point? We'll show that in this step.

@section s7model The model

In the ini file we need to set only the ieee80211Visualizer's parameters. This visualizer will display us information about the association.

@section s7results Results



Sources: @ref omnetpp.ini, @ref VisualizationNetworks.ned

@nav{step6,step8}
@fixupini

<!------------------------------------------------------------------------>

@page step8 Step 8 -

@nav{step7,step9}

@section s8goals Goals

@section s8model The model

@section s8results Results

Sources: @ref omnetpp.ini, @ref VisualizationNetworks.ned

@nav{step7,step9}
@fixupini

<!------------------------------------------------------------------------>

@page step9 Step 9 -

@nav{step8,step10}

@section s9goals Goals

@section s9model The model

@section s9results Results

Sources: @ref omnetpp.ini, @ref VisualizationNetworks.ned

@nav{step8,step10}
@fixupini

<!------------------------------------------------------------------------>

@page step10 Step 10 -

@nav{step9,step11}

@section s10goals Goals

@section s10model The model

@section s10results Results

Sources: @ref omnetpp.ini, @ref VisualizationNetworks.ned

@nav{step9,step11}
@fixupini

<!------------------------------------------------------------------------>

@page step11 Step 11 -

@nav{step10,step12}

@section s11goals Goals

@section s11model The model

@section s11results Results

Sources: @ref omnetpp.ini, @ref VisualizationNetworks.ned

@nav{step10,step12}
@fixupini

<!------------------------------------------------------------------------>

@page step12 Step 12 -

@nav{step11,step13}

@section s12goals Goals

@section s12model The model

@section s12results Results

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

/**
@mainpage Visualization Tutorial for the INET Framework -- Part 1
In this tutorial we show you, how to use the visualization module in the INET Framework. The tutorial contains a series of simualtion models, numbered from 1 through 13. The communication is the same for the whole simulation. There are two servers, that send video stream to two cars through access points. In the park there are two pedestrians, who speak with each other by phone, also through access points. Three routers make the routing in the network statically.
It doesn't seem too complicated, but in this tutorial our main focus is on the visualization, not on the operation of the network.
Our goal is show the working of the visualization module and its submodules in INET.

This is an advanced tutorial, and it assumes that you are familiar with creating
and running simulations in @opp and  INET. If you aren't, you can check out
the <a href="https://omnetpp.org/doc/omnetpp/tictoc-tutorial/"target="_blank">TicToc Tutorial</a> to get started with using @opp. The <ahref="../../../doc/walkthrough/tutorial.html" target="_blank">INET Walkthrough</a> is an introduction to INET and working with protocols.

If you need more information at any time, feel free to refer to the @opp and INET documentation:

- <a href="https://omnetpp.org/doc/omnetpp/manual/usman.html" target="_blank">@opp User Manual</a>
- <a href="https://omnetpp.org/doc/omnetpp/api/index.html" target="_blank">@opp API Reference</a>
- <a href="https://omnetpp.org/doc/inet/api-current/inet-manual-draft.pdf" target="_blank">INET Manual draft</a>
- <a href="https://omnetpp.org/doc/inet/api-current/neddoc/index.html" target="_blank">INET Reference</a>

In the tutorial, each step is a separate configuration in the same omnetpp.ini file.
Steps build on each other, they extend the configuration of the previous step by adding a few new lines. Consecutive steps mostly share the same network, defined in NED.

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

@section s1results Results



Sources: @ref omnetpp.ini, @ref

@nav{index,step2}
@fixupini

<!------------------------------------------------------------------------>

@page step2 Step 2 -

@nav{step1,step3}

@section s1goals Goals

@section s1model The model

@section s1results Results

Sources: @ref omnetpp.ini, @ref

@nav{step1,step3}
@fixupini

<!------------------------------------------------------------------------>

@page step3 Step 3 -

@nav{step2,step4}

@section s1goals Goals

@section s1model The model

@section s1results Results

Sources: @ref omnetpp.ini, @ref

@nav{step2,step4}
@fixupini

<!------------------------------------------------------------------------>

@page step4 Step 4 -

@nav{step3,step5}

@section s1goals Goals

@section s1model The model

@section s1results Results

Sources: @ref omnetpp.ini, @ref

@nav{step3,step5}
@fixupini

<!------------------------------------------------------------------------>


@page step5 Step 5 -

@nav{step4,step6}

@section s1goals Goals

@section s1model The model

@section s1results Results

Sources: @ref omnetpp.ini, @ref

@nav{step4,step6}
@fixupini

<!------------------------------------------------------------------------>

@page step6 Step 6 -

@nav{step5,step7}

@section s1goals Goals

@section s1model The model

@section s1results Results

Sources: @ref omnetpp.ini, @ref

@nav{step5,step7}
@fixupini

<!------------------------------------------------------------------------>

@page step7 Step 7 -

@nav{step6,step8}

@section s1goals Goals

@section s1model The model

@section s1results Results

Sources: @ref omnetpp.ini, @ref

@nav{step6,step8}
@fixupini

<!------------------------------------------------------------------------>

@page step8 Step 8 -

@nav{step7,step9}

@section s1goals Goals

@section s1model The model

@section s1results Results

Sources: @ref omnetpp.ini, @ref

@nav{step7,step9}
@fixupini

<!------------------------------------------------------------------------>

@page step9 Step 9 -

@nav{step8,step10}

@section s1goals Goals

@section s1model The model

@section s1results Results

Sources: @ref omnetpp.ini, @ref

@nav{step8,step10}
@fixupini

<!------------------------------------------------------------------------>

@page step10 Step 10 -

@nav{step9,step11}

@section s1goals Goals

@section s1model The model

@section s1results Results

Sources: @ref omnetpp.ini, @ref

@nav{step9,step11}
@fixupini

<!------------------------------------------------------------------------>

@page step11 Step 11 -

@nav{step10,step12}

@section s1goals Goals

@section s1model The model

@section s1results Results

Sources: @ref omnetpp.ini, @ref

@nav{step10,step12}
@fixupini

<!------------------------------------------------------------------------>

@page step12 Step 12 -

@nav{step11,step13}

@section s1goals Goals

@section s1model The model

@section s1results Results

Sources: @ref omnetpp.ini, @ref

@nav{step11,step13}
@fixupini

<!------------------------------------------------------------------------>

@page step13 Step 13 -

@nav{step12,step14}

@section s1goals Goals

@section s1model The model

@section s1results Results

Sources: @ref omnetpp.ini, @ref

@nav{step12,step14}
@fixupini

<!------------------------------------------------------------------------>

@page step14 Step 14 -

@nav{step13,step15}

@section s1goals Goals

@section s1model The model

@section s1results Results

Sources: @ref omnetpp.ini, @ref

@nav{step13,step15}
@fixupini

<!------------------------------------------------------------------------>

@page step15 Step 15 -

@nav{step14,step16}

@section s1goals Goals

@section s1model The model

@section s1results Results

Sources: @ref omnetpp.ini, @ref

@nav{step14,step16}
@fixupini

<!------------------------------------------------------------------------>

@page step16 Step 16 -

@nav{step15,step17}

@section s1goals Goals

@section s1model The model

@section s1results Results

Sources: @ref omnetpp.ini, @ref

@nav{step15,step17}
@fixupini

<!------------------------------------------------------------------------>

@page step17 Step 17 -

@nav{step16,step18}

@section s1goals Goals

@section s1model The model

@section s1results Results

Sources: @ref omnetpp.ini, @ref

@nav{step16,step18}
@fixupini

<!------------------------------------------------------------------------>

@page step18 Step 18 -

@nav{step17,step19}

@section s1goals Goals

@section s1model The model

@section s1results Results

Sources: @ref omnetpp.ini, @ref

@nav{step17,step19}
@fixupini

<!------------------------------------------------------------------------>

@page step19 Step 19 -

@nav{step18,step20}

@section s1goals Goals

@section s1model The model

@section s1results Results

Sources: @ref omnetpp.ini, @ref

@nav{step18,step20}
@fixupini

<!------------------------------------------------------------------------>

@page step20 Step 20 -

@nav{step19,step21}

@section s1goals Goals

@section s1model The model

@section s1results Results

Sources: @ref omnetpp.ini, @ref

@nav{step19,step21}
@fixupini

<!------------------------------------------------------------------------>

@page step21 Step 21 -

@nav{step20,step22}

@section s1goals Goals

@section s1model The model

@section s1results Results

Sources: @ref omnetpp.ini, @ref

@nav{step12,conclusion}
@fixupini

<!------------------------------------------------------------------------>

@page step22 Step 22 -

@nav{step21,conclusion}

@section s1goals Goals

@section s1model The model

@section s1results Results

Sources: @ref omnetpp.ini, @ref

@nav{step21,conclusion}
@fixupini

<!------------------------------------------------------------------------>

@page conclusion Conclusion -

@nav{step22,index}

@section s1goals Goals

@section s1model The model

@section s1results Results

Sources: @ref omnetpp.ini, @ref

@nav{step22,index}
@fixupini

<!------------------------------------------------------------------------>*/

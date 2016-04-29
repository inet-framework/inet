/**
@mainpage INET Framework for OMNEST/OMNeT++<br>A Guided Tour

This tutorial introduces the reader to using the INET Framework by taking
them through the process of running and exploring an example simulation.

We will use the model of an Ethernet-based network containing switches,
routers, and IP-based hosts. In the model, hosts exchange information over
TCP.

@section contents Contents

-   @ref intro
-   @ref launching
-   @ref gui
-   @ref running
-   @ref inside
-   @ref steps
-   @ref arpbegins
-   @ref requestpacket
-   @ref pendingqueue
-   @ref arpcache
-   @ref transmission
-   @ref arpreply
-   @ref procreply
-   @ref syn
-   @ref discovering
-   @ref conclusion

NEXT: @ref intro
*/
-------------------------------------------------------------------------------
/**
@page intro Introducing the model

UP: @ref contents

The tutorial will use the ARPTest model, which can be found in the
`inet/examples/ethernet/arptest2/` folder of the INET project. The model
consists of a NED file that defines the network topology, and an
omnetpp.ini file that configures it. Otherwise the model contains only
standard INET components such as StandardHost.

The network looks like this when the NED file is opened in the graphical
editor in the IDE:

<img src="nededitor.png">

Some explanations. In the model, MAC addresses are assigned automatically
to Ethernet interfaces at the start of the simulation, by incrementing a
counter. Ethernet link speeds are set by using appropriately configured
channels  At the IP level, the model uses static routing. Assigning IP
addresses and adding static routes is done by a separate configurator
module. TCP traffic is configured in the ini file by adding application
modules to the hosts.

Here is the interesting part of the NED source:

@dontinclude ARPTest.ned
@skip network ARPTest
@until server.pppg
@skipline }

And we will use this configuration file:

@include omnetpp.ini

NEXT: @ref launching
*/
-------------------------------------------------------------------------------
/**
@page launching Launching the ARPTest simulation

UP: @ref contents

We assume that the INET framework has already been built from sources. If
not, it can be built in the IDE using the Ctrl+B keyboard shortcut.

There are several variations on how one can launch the simulation from the
IDE. One way is to select omnetpp.ini in Project Explorer, and click the Run
button on the toolbar. Alternatively, one can select the NED file or the
containing folder, and click Run. (In this case, the IDE will ask which ini
file to use if there are more than one.) One can also right-click the
omnetpp.ini file, select Run As from the menu, then choose OMNeT++ Simulation.

<img src="ide.png">

When launching is successful, a new window, similar to the one shown below,
will appear. The window belongs to the running simulation program, which contains the
OMNeT++ simulation kernel, the compiled code for the components of the INET
Framework, and a graphical runtime environment (Qtenv or Tkenv). This
program is able to run any INET simulation if one provides the necessary
NED and INI files for it. Note that the GUI and the simulation are running
in the same process, so if the simulation crashes due to a programming error,
it also brings down the GUI with it. Also note that the GUI is optional,
one can also run simulations using a command line interface, which is
useful for example for batch runs.

<img src="arptest.png">

NEXT: @ref gui
*/
-------------------------------------------------------------------------------
/**
@page gui The GUI at the first glance

UP: @ref launching

Let us examine the Qtenv user interface a little.

<img src="qtenv-parts.png">

The top of the window contains the following elements below the menu bar:

- *Toolbar*: The toolbar lets one access the most frequently used functions,
  such as stepping, running and stopping the simulation.
- *Status bar*: Two rows of various fields and gauges, displaying the current event number,
  simulation time, information about the next simulation event, and other details.
  When the simulation is running, it displays performance data like the number of
  events processed per second. The second row can be turned off to free up vertical space.
- *Timeline*: Displays the contents of the Future Events Set (FES) on a logarithmic time scale.
  The timeline can be turned off to free up vertical space.

The main window is divided into the following areas:

- *Object Navigator*: Displays the hierarchy of objects in the current simulation and in the FES.
- *Object Inspector*: Displays the contents and properties of the selected object.
- *Network Display*: Displays the network or any module graphically. This is also where
  animation takes place.
- *Log Viewer*: It has two modes. In *Messages* mode, it displays the log of packets
  or messages sent between modules; in *Log* mode, it displays the log written by modules
  (using `EV<<` statements) during simulation. Mode switching buttons are on the local toolbar.

Additionally, one can open inspector windows that float on top of the main window.

NEXT: @ref running
*/
-------------------------------------------------------------------------------
/**
@page running Running the simulation

UP: @ref launching

In this step we'll explore how one can run the simulation.

Click on the "Run" button on the toolbar, or hit F5.

<img src="tb-run.png"/>

Since the underlying network is Ethernet, before being able to send
the TCP SYN packet the client has to perform an ARP request
to learn the MAC address for the default router. After some more messaging,
the TCP connection will be established and data transfer begins.
We'll explore these events later.

Qtenv animates packet transfers, packets are represented by red dots.

<img src="animation.gif">

One can make the animation faster or slower by adjusting the slider at the
top of the network window.

<img src="animspeed.png"/>

The simulation can be stopped by clicking the red "STOP" traffic
sign icon (or by hitting F8), resumed by clicking the "Run" icon or pressing F5.

The 'step' button (F4) can be used to execute one event only.

The wandering red frame (around `router1` on the screenshot) simply
indicates the location of the current/next event in the model, as dictated
by the basic event processing algorithm of discrete event simulation.

<img src="tb-run.png"/>

The "Fast" icon or (F6) will turn off message animation and
update the graphics only in every 500 milliseconds or so (the exact number
can be configured in the Simulation Execution dialog,
File menu/Preferences/General tab, accessible from the toolbar as well, with the wrench symbol). The "Express"
icon provides the highest speed: it turns off all GUI functions while the
simulation is running, even writing to the log is turned off.
Express mode updates the display every 1000 miliseconds or so
(also configurable in Simulation Options).

<img src="dlg-simoptions1.png"/>

In Express mode one can only stop the simulation by clicking the
"Big Red STOP Button" (see below). Simulation speed in Express mode may
significantly depend on the state of the "auto update inspectors" checkbox.

<img src="dlg-stopexpress.png"/>

One can read the speed of the simulation on the the main
window status bar. Ev/sec tells one how many events the CPU crunches
in a second -- this depends on the strength of the hardware, on the
amount processing (C++ code) to be done for an average event
(refer to the <tt>handleMessage(cMessage *)</tt> functions in the model
source), and on the run mode (Normal, Fast or Express). It is not unusual
for the Express mode to be 100-200 times faster than Fast mode.

<img src="speedbar.png"/>

The other two readings on that status bar are simsec/sec (how many simulated
seconds the model progresses in one "real" second) and ev/simsec (how many events
the model contains per simulated second). The simsec/sec value is useful
for estimating how long the simulation will take. Ev/simsec is independent
of whether the simulation runs in Normal, Fast or Express mode, and it only depends
on the nature and size of the model (ATM cell-level simulations will have
a few magnitudes higher ev/simsec values than call center simulations.)

While in Expess mode, the following dialog will appear after a while:

<img src="dlg-nomoreevents.png"/>

"No more events" is one way a simulation can terminate normally
(i.e. not with an error). It means that there's nothing more to
simulate -- in our simulation this occurs when the client has
successfully closed the TCP connection and finished its operation,
and there're no more packets underway and no timers running anywhere
in the whole model.

Other simulations may end with the "Simulation time limit reached -- simulation stopped"
message which means that the simulation time configured in <tt>omnetpp.ini</tt>
has been reached. (For completeness: other possibilities include
"CPU time limit reached," and "Simulation stopped with endSimulation()"
meaning that some component in the model called the <tt>endSimulation()</tt>
C++ function -- for example when it detects that desired statistical accuracy
has been reached).

In both cases, one can restart the simulation using the "Rebuild network"
command on the Simulate menu. In fact, one can do that any time to start over.

<img src="m-restart.png"/>

Now that we've dealt with the basics, we can go back to our ARPTest network model.

NEXT: @ref inside
*/
-------------------------------------------------------------------------------
/**
@page inside What is inside the hosts and routers?

UP: @ref running

One can double-click on a node (client, server, router) to see its internals.

@htmlonly
<center>
<a href="client.png"><img src="client.png" width=350></a>
<a href="server.png"><img src="server.png" width=350></a>
<a href="router.png"><img src="router.png" width=350></a>
<a href="switch.png"><img src="switch.png" width=500></a>
<a href="net.png"><img src="net.png" width=400></a>
</center>
@endhtmlonly

Bottom up, the client model contains:

<ul>
    <li>an Ethernet interface (<tt>eth[0]</tt>)
    <li>a loopback interface (<tt>lo0</tt>)
    <li>network layer (<tt>networkLayer</tt>, a further double-click will
        reveal it's basically IP);
    <li>TCP model (<tt>tcp</tt>). The number of connections in various
        TCP states are displayed in blue above the icon;
    <li>an application model (<tt>tcpApp[0]</tt>)
    <li>the <tt>routingTable</tt> module contains, well, the routing table
        (number of unicast+multicast routes displayed above the icon)
    <li>the <tt>interfaceTable</tt> contains information about network interfaces of the host
</ul>

A router is similar, but obviously it has nothing from L3 up. (There is also no routing protocol,
because this example uses static routing.)

Further double-clicking on these components (TCP, UDP, etc.) will reveal
what modules they are composed of. One can use the arrow buttons in the
local toolbar to navigate levels (i.e. go up a level). After some levels
one gets an empty window. It means that the component is implemented in
C++, and cannot be subdivided any further (it is a <i>simple module</i>, as
opposed to <i>compund modules</i> which are composed of submodules).
Right-clicking the simple module and selecting <i>Open Details</i> will
open an inspector window like this:

<img src="tcpmain_v2.png"/></a>

The inspector window exposes the internals of the module and we'll use it a
lot later in this walkthrough, but let's just leave it for now. One can
also view the information presented here in the object inspector panel at
the bottom left side of the simulation window. Click on any module and its
properties will be displayed there.

The ARPTest simulation is quite complex in that it has TCP, IP, ARP and
Ethernet in it. In this walkthrough we'll go for ARP, but the steps learned
here will be useful for exploring other protocols as well.

NEXT: @ref steps
*/
-------------------------------------------------------------------------------
/**
@page steps Steps towards exploring ARP

UP: @ref inside

If we are interested in ARP, it's a bit annoying that we have to wade
throught all those protocol registration events until we get to the first
ARP request at 1s. Luckily, one can use the "Run until..." function to
fast-forward past them.

Restart the simulation (Simulate|Rebuild network), then click the
"Run until..." icon on the toolbar.

<img src="tb-rununtil.png"/>

When protocol registration finishes we are still at 0s simulation time, but the first ARP
request is around 1s, so it is safe to enter anything in the Run until dialog between 0 and 1s, so now let's enter 1ms.

<img src="dlg-rununtil.png"/>

After clicking OK, the simulation progresses past the protocol registration period in no time.
Then, both the red frame around the laptop icon (the next event marker) and the
status bar at the top will show that the next event is going to be a timer event, take place
in the client PC's tcpApp, at t=1s simulation time (which will be 1s from the current time, as indicated by the +1s in brackets). Opening "scheduled-events" in the
object tree (left side of main window) will also reveal that we have only
one event. Double click on the event to open the inspector window (or single click and check out the object inspector at the bottom left). One can switch to flat mode at the top of the panel to see the details of the event object.

<img src="nextevent_v2.png">

Right-clicking on a module and choosing <i>Open Graphical View</i> opens a separate window displaying the module's insides. This is a useful feature because multiple views can be open at the same time. Let's go into the client computer and open a graphical view of <i>networkLayer</i>. Now we can observe as packets arrive at the client module and the path they take in the network layer.

<img src="graph_view.png">

Pressing F4 (single-step) repeatedly will take one through the next events. It can be seen as the application tells TCP to open a connection, as TCP obeys and sends a SYN (TCP connection request) packet. You can follow this packet as it enters the network layer.
You will find the SYN packet sitting at the input of the IP module. The IP module is doing a function call to the routing table one level up and the arp module to the right. This is indicated by the dotted lines (this isn't part of normal packet traffic, but how network operation is implemented behind the scenes).

Here we are at the beginning of the interesting part. You may actually take a note that we are at event #25 (it's displayed in the top line of the main window status bar) so that next time you can quickly get here using the "Run until..." dialog (it can stop either at a simulation time or at an event number -- or if you enter both, it'll stop at whichever comes first. Note that you enter the value of the next event you want to observe -- so in this case we enter 26 in the run until dialog).

<img src="licecap3.gif">

You can find the <i>Run until next event in this module</i> button on the toolbar at the top of the model window. This is a very useful tool if you want to fast-forward to the next thing that will happen in the given module. There is also a fast run version.

<img src="nextevent.png">

NEXT: @ref arpbegins
*/
-------------------------------------------------------------------------------
/**
@page arpbegins ARP begins

UP: @ref steps

We can see what's going on by checking out the module logs. These
log messages (output via <tt>ev &lt;&lt;</tt> from the C++ code) will appear in the log panel at the bottom of the main window, but that'll soon be useless because other modules
dump their output there too. Luckily, we can filter the logs so we can concentrate on messages from modules we are interested in now -- we can do that with the <i>filter window contents</i> button at the top right of the logs panel. Here we can select which module's logs we want to display -- select arp and ip from <tt>client/networkLayer</tt>.

<img src="filterlogs.png">

So we are at event #25, let's press F4 (or click at the <i>step</i> button) for the next event, and let IP process the SYN packet.
You'll see that:

<ul>
<li>the syn packet disappears in IP
<li>arp creates an arp request packet
<li>IP sends the packet
</ul>

The logs will look like this:

<img src="arprequestlogs.png">

ARP maps L3 addresses (IP address) to L2 addresses (Ethernet MAC address).
It uses a lookup table (ARP cache) for this purpose, but if an IP address
is not found in the table, ARP has to ask around by sending a broadcast
ARP Request. That's what we have seen here. Let's check it in detail!

NEXT: @ref requestpacket
*/
-------------------------------------------------------------------------------
/**
@page requestpacket The ARP Request packet

UP: @ref arpbegins

Click on the ARP request packet to view it's details in the object inspector panel (bottom left of the main window).

<img src="arpreq-fields.png"/>

It shows the contents of the ARP packet: source MAC address,
destination MAC address, source IP address, destination IP address.
The source MAC and IP addresses are our own addresses (in the model,
autoconfigured MAC addresses begin with "0A AA"); the destination IP
address contains the address to be resolved, and the destination MAC
address is empty because that's what we're requesting.

The ARP packet is a single C++ object, with its class <tt>ARPPacket</tt>
having an <tt>int _opcode</tt> data member, <tt>int opcode()</tt> and
<tt>setOpcode(int)</tt> functions, and similar data and functions for the
other data fields as well. However, the class was not hand-coded but
generated from a <tt>.msg</tt> file, which has a very concise format:

<pre class="src">
<i>// file: ARPPacket.msg</i>
message ARPPacket
{
    fields:
        int opcode enum(ARPOpcode);
        MACAddress srcMACAddress;
        MACAddress destMACAddress;
        IPAddress srcIPAddress;
        IPAddress destIPAddress;
};
</pre>

Definition via a <tt>.msg</tt> file not only saved a lot of programming effort,
but it was also necessary for the packet's content to be displayed in the Details.

So far so good, but how do lower layers (Ethernet) know which MAC address
the ARP request should be sent to? That's not part of the packet!

This information is attached to the packet in small data structure called
"control info". It is used to pass information between network layers, generally from a higher level to a lower one. Control info is also a C++ object and similarly to packets,
its C++ source is generated from <tt>.msg</tt> files.
You can view the contents of the control info attached to this ARP packet
by clicking "Control info" in the object inspector.

<pre class="src">
<i>// file: EtherCtrl.msg</i>
class EtherCtrl
{
    fields:
        MACAddress src;  // used with ETHCTRL_DATA
        MACAddress dest; // used with ETHCTRL_DATA
        int etherType;   // used with ETHCTRL_DATA and EthernetIIFrame
        int ssap;        // used with ETHCTRL_DATA and EtherFrameWithLLC
        int dsap;        // used with ETHCTRL_DATA/REGISTER_DSAP/DEREGISTER_DSAP
        int pauseUnits;  // used with ETHCTRL_PAUSE
};
</pre>

<img src="arpreq-ctrlinfo.png"/>

It says that source MAC address is not specified (all zero -- in this case
it'll be filled in by Ethernet) and the destination MAC address is all FF's
(the Ethernet broadcast address). SSAP and DSAP are unused here (they're for
802.3 frame type but we use Ethernet II frames in this simulation), and pauseUnits
has also no significance (only used for telling MAC to send PAUSE frames).
etherType is set to 2054 here. <!-- FIXME set etherType to proper value --><!do we need this?>

NEXT: @ref pendingqueue
*/
-------------------------------------------------------------------------------
/**
@page pendingqueue The pending queue

UP: @ref arpbegins

While ARP resolution is pending, IP datagrams which wait for it have to be
buffered somewhere. The <i>IP</i> module is storing the packets while waiting
for ARP to finish. If you click on the <i>IP</i> module, you can see that it has
a SYN packet queued.

<img src="arpqueue.png"/>

You may double-click on the SYN packet, to view the datagram's contents
(source and destination IP address, TTL, encapsulated protocol, etc.)
in an inspector window. Here you can find the encapsulated TCP packet. Double click and
you can explore the TCP header fields.

<img src="TCPpacket.png"/></a>

NEXT: @ref arpcache
*/
-------------------------------------------------------------------------------
/**
@page arpcache The ARP cache

UP: @ref pendingqueue

You maybe be wondering what the ARP cache contains right now.
No problem, we can check it out!

Click on the <i>arp</i> icon in <tt>client/networkLayer</tt>, and examine the object inspector.

The list reveals that we have several counters here for sent and received ARP requests
etc -- and also the ARP cache.
Double-click on the <tt>arpCache</tt> line. Another inspector will open, double click on elements to see the arp cache contents.

<img src="ARPcache.png"/>

First of all, you'll notice that the cache contains what you'd probably expect:
one entry, IP address 10.0.0.25, pending resolution.

The ARP cache is
implemented with a standard STL <tt>map</tt>. You can check the source code in the OMNeT++ IDE. Press ctrl + shift + R to search for resources, and search for <i>ARP.cc</i> and <i>ARP.h</i>.
In <i>ARP.cc</i> you can see that the <i>key</i> in the map
is <tt>class IPAddress</tt> and the <i>value</i> is
<tt>class ARPCacheEntry</tt> (or rather, a pointer to it). If you'd be
looking in the C++ code to find out what makes STL map able appear like
this in the GUI, you'd find only one extra statement in <i>ARP.h</i>:
<tt>WATCH_PTRMAP(arpCache)</tt>, plus <tt>&lt;&lt;</tt> operators
defined for <tt>IPAddress</tt> and <tt>ARPCacheEntry</tt>. That's all
it takes -- the rest is up to the simulation environment.

But let us follow the ARP request further.

NEXT: @ref transmission
*/
-------------------------------------------------------------------------------
/**
@page transmission Transmission over Ethernet

UP: @ref arpcache

With a further F4 key (or "Step" toolbar button), the ARP request packet is sent down to the
ethernet module. You can go into the module to see the packet at the input of the <i>encap</i> module.
After another F4 key press, the <tt>encap</tt>
will encapsulate the ARP request into an Ethernet II frame.
You can verify by going into the queue module, and double clicking
on the message and opening an inspector that the destination address
in the frame has been set to broadcast (all FF's). The source address
is still all zeros, it'll be set by MAC. You
can also verify that the ARP packet has been indeed encapsulated in an
Ethernet frame.

After a few more steps, the packet leaves the queue and through the MAC module, it is transmitted from the client.
You can watch this if you go to the highest level. The packet moves to router 1. The packet animation
signifies the start of packet transmission. Notice that after the packet moves to the router, the link
turns yellow, meaning there is a transmission going on between the client and the router.

Hit step once more and the link color goes back to normal, transmission of the frame is completed,
this can be seen in the logs as well.

Note the <i>Start of IFG period</i> in the logs.
IFG stands for
Inter-Frame Gap in the Ethernet standards, and it is the time needed
by older Ethernet hardware to switch from receive (carrier sense)
mode to transmit mode. In the model, end of the IFG is marked with
an event which can also be seen in "scheduled-events".

The end of the interframe gap period is implemented with a self message -- EndIFG in the scheduled events list. You can see it on the timeline at the top of the main window, and tell that
it is going to take place 10 microseconds from now (you can also single click events in the timeline to view them in the object inspector and double click to open their inspector windows).

<img src="endifg.png">

Most actions we've seen so far, from <tt>app</tt>'s TCP Active OPEN
command to encapsulation of the ARP packet into an Ethernet frame,
took zero simulation time. We assumed that -- compared
to other time periods occurring in the model -- they take such a short
time that it is not worth modelling explicitly (they wouldn't make
a difference in the simulation results).

However, the time an Ethernet frame transmission takes <i>is</i>
interesting to us, and so is propagation delay on the cable (the cable may be
long) -- so in fact we have four discrete points of time, and we need
four simulation events to model them:

<ul>
    <li>transmitter starts transmitting the frame
    <li>beginning of frame reaches receiver (start receiving)
    <li>transmitter completes transmission
    <li>end of frame reaches receiver (end of reception)
</ul>

The message which you've seen <tt>mac</tt> having sent (the Ethernet
frame message object in fact) represents the <i>beginning</i> of the frame.
End of transmission is an event you'll find in "scheduled-events", it is
a self-message scheduled by the transmitting <tt>mac</tt>:

<img src="endtx-event.png">

The "start of reception" event will occur in the receiver of course,
and will be represented by the arrival of the Ethernet frame message
object. You can see in the first line of the status bar, in the
"next event" field that it's going to be the next event, and will take
place in <tt>router1</tt>'s second interface (<tt>eth[1]</tt>).

<img src="statusbar-startrx.png">

Open the graphics for this Ethernet interface by double-clicking
on <tt>router1</tt>, then on <tt>eth[1]</tt> (just follow the red frames
which indicate the location of the next event).

Pressing F4 once more makes this <tt>mac</tt> process the event and
go into "receiving" state -- its icon will also turn blueish to indicate
it. From the frame length and the data rate <tt>mac</tt> has calculated
the time the last bit will arrive (end reception), and scheduled an
event accordingly (you can verify it in the "scheduled-events" list).

@htmlonly
<center>
<img src="router1-rx.gif"/>
<img src="thumbs/router1eth1-rx.gif"/>
</center>
@endhtmlonly

During the next few F4 keypresses, the client laptop <tt>mac</tt> will
finish transmission, the <tt>router1</tt>'s <tt>mac</tt> will complete
reception, and it will pass up the frame to <tt>encap</tt> which in turn will
rip the Ethernet header and pass up the ARP Request packet to <tt>arp</tt>.

NEXT: @ref arpreply
*/
-------------------------------------------------------------------------------
/**
@page arpreply ARP Reply

UP: @ref transmission

In the next few event, <tt>arp</tt> will process the request. Hit F4.
<tt>arp</tt>'s log output will show what happened.

<img src="arp-reply-sent.png">

<!do we need this?>
You can check the arp cache and see that a new entry has been added, as indicated by the logs:

<img src="router-arpcache-entry.png">

ARP took the source IP address and source MAC address, and added them
to its own ARP cache. Then it had to determine whether to reply to the
request. The destination IP address was did not belong to the router;
however, proxy ARP is turned on (you can verify this in the ARP module's
inspector window, "Parameters" section -- see below). With proxy ARP, the
router has to reply to the ARP Request if it can route the address, and
send back its own MAC address. That's what happened here.

<img src="arp-params.png"/></a>

While ARP processed the request packet, you probably saw a red flashing
arrow, like this:

<img src="arp-outputport.png">

It indicates ARP asking the routing table component whether the address
10.0.0.1 is routable, as part of the proxy ARP process. (More precisely,
ARP asks which is the output port for that address -- if the reply is "none"
it means the address is not routable). In the C++ code, what you've seen
was the animation of a direct method call -- ARP module's C++ code
invoked the <tt>int outportNo(IPAddress)</tt> method of the routing table
object (class <tt>RoutingTable</tt>) with 10.0.0.1 as argument.
It was the simulation environment which made this request visible
in the simulation GUI.

The answer was probably "routable", because <tt>arp</tt> sent a reply.
You can verify by opening the ARP Reply packet: source and destination
fields have been swapped, and the now-source (formerly destination)
MAC address field contains the MAC address of this interface.

<img src="arpreply-fields.png"/></a>

The next few F4 keypresses will transmit the ARP Reply over Ethernet
back to the client's ARP.

NEXT: @ref procreply
*/
-------------------------------------------------------------------------------
/**
@page procreply Processing the ARP Reply

UP: @ref arpreply

Results of processing the ARP reply can be seen in the next screenshot.
The MAC address of the router's Ethernet interface has been added to the
ARP cache. ("age=0" in the cache entry is needed because after certain
period of inactivity [120s configured here] the entry is considered
obsolete and gets thrown out of the cache. In fact, the entry really stores
last update time, but age is more practical when displayed.)
The datagram carrying the TCP SYN segment that was wainting in the queue
has been sent down to Ethernet for transmission.

<img src="arp-resolved.png">

You can even verify that the control info attached to the datagram
indeed contains the MAC address just received via ARP.

<img src="syn-ctrlinfo.png"/></a>

NEXT: @ref syn
*/
-------------------------------------------------------------------------------
/**
@page syn The client's SYN gets to the server

UP: @ref procreply

Now that ARP has done its job, you can hit F5 (or click "Run" on the main
window toolbar), sit back and watch as the SYN packet makes its way to the
server. It is still not any easy job for SYN, because this example
network is full of distinct Ethernet segments, so it'll take still
a series of ARP requests and replies get to the server.

You'd maybe think that the remote TCP's response, SYN+ACK will have easier
job on the way back, because SYN paved the way for it. But not really:
it was the IP addresses of routers initiating ARP resolutions that
made it into the ARP caches, and not the client's IP address 10.0.0.26.
So the series of ARP request/replies repeats as SYN+ACK progresses
back to the client computer.

But further packets really don't have to go through ARP resolution
any more.

After some time, the logs can be quite cluttered ,so you may want to use the <i>Show message/packet traffic</i> feature of the log panel to view packet traffic only. If you enter a module and switch to message/packet traffic view, the viewer shows packets that were sent and received by that module. You can also filter which modules' traffic you want to examine. For example, you might be interested in traffic between the client and router 1 only -- you tick these two when you filter the modules, and you will get a result like this:

<img src="packettraffic_v2.png">

Whereas the logs are created by the <tt>ev <<</tt> statements in the C++ code, this message/packet traffic view is created automatically by the simulation environment -- no additional code necessary. The useful thing about this feature is that you can filter and examine messages and packets even after the simulation has finished.

Packet traffic can be visualized with the <i>Sequence chart tool</i>. This is created from the event log. In order to use it, you must record an event log -- you can do that by clicking the <i>REC</i> button in the simulation environment (this can be automated by specifying <tt>record-eventlog = true</tt> in <i>omnetpp.ini</i>).

<img src="rec.png">

Before the log data is written to a results file, the finish() function must be invoked. It is invoked automatically when the simulation finishes (if it finished normally without an error), or you can do it manually if you don't want to wait -- click the <i>Conclude simulation</i> button at the top of the simulation environment.

<img src="finish.png">

The event log file (.elog) will be in arptest2/results. Double click to open the event log and the sequence chart.

<img src="sequencechart.png" width="1000px">

In this example sequence chart you can follow the initial SYN packet as it makes its way to the server, and the various ARP requests and replies.

NEXT: @ref discovering
*/
-------------------------------------------------------------------------------
/**
@page discovering Following ARP from the documentation

UP: @ref syn

In the above guided tour, you've seen how to run a simulation model,
view logs, examine packet contents, look at queues, and explore internal
tables and variables. But what do you do if something is still not
clear? If log messages do not give enough information to find out how
the thing works?

The answer is: <i>documentation</i>. You can find the INET reference <a href="https://omnetpp.org/doc/inet/api-current/neddoc/index.html" target="_blank">here</a>, which contains information about all the INET modules, the module hierarchy, and C++ source documentation.

<img src="reference.png">

You can read the <a href="https://omnetpp.org/doc/inet/api-current/inet-manual-draft.pdf" target="_blank">INET manual draft</a>.

The source code can be searched and examined in the OMNeT++ IDE. Useful functions:

<ul>
    <li>ctrl + shift + R:   Resource search
    <li>ctrl + shift + N:   Search for NED type
    <li>ctrl + H:           File and C++ search
    <li>ctrl + shift + G:   Usage search (click on a function and press the key combination to show where function is used)
    <li>You can also ctrl-click functions in the code to open their  source
</ul>

Possible starting points for exploring ARP would be the <tt>ARP</t> module</tt> itself. Basically,
if you know what you're looking for (we know now), just click <tt>ARP</tt>
on the "Simpe modules" list on the left. If you don't
know, you can start from an example network (here: <tt>ARPTest</tt>, in "Networks" -- choose the second one for ARPTest2),
and click on submodules until you find what you're looking for.

<img src="doc-arp.png" width="1000px">

ARP's page lists the parameters and gates (or ports) (further below, not shown in the screenshot).
It also contains its NED definition (also further below). The usage diagram's icons contain links, and will take you to the particular module. And what's important
for us now, there's a link to the C++ documentation.

<img src="doc-cc.png">

Click the "C++ documentation" link. The C++ documentation that emerges
has been generated from commented C++ source by the Doxygen tool. The sheer size and detail of the
C++ documentation might be overwhelming to you, but not if you know
where to start looking.

There are two good starting points: the <tt>handleMessage(cMessage*)</tt> member
function, and the data structure used in the module (Doxygen lists data members
under the <i>Protected</i> (or <i>Public</i> or <i>Private</i>) <i>Attributes</i>
heading). <tt>handleMessage(cMessage*)</tt> is the function that gets invoked
whenever a message (packet, etc) arrives at the module or a timer goes off
(a message scheduled by the module itself comes back).

<img src="arp-cc.png" width="1000px">

As a rule of thumb: whatever large number of functions you see in the
documentation, always click at <tt>handleMessage()</tt> first. If the
module code was well written, it'll lead you from there.

NEXT: @ref conclusion
*/
-------------------------------------------------------------------------------
/**
@page conclusion Conclusion

Up: @ref discovering

While ARP might not be the most important protocol on earth, it was chosen as the
topic of this tutorial because it is simple enough to be presented in such
a short space, and yet it does something interesting -- with the results
immediately visible. And not least, it is well known and on everybody's desk
(who doesn't have Ethernet LAN?).

It is not ARP which is important anyway. By following the tutorial, you've
acquired significant knowledge which will be applicable whichever part
of INET (or another OMNET++/OMNEST simulation model) you're going to explore.
You've learned how to run a model, how to explore it by looking at
module output, by peeking into packets, queues and internal tables, and
how to browse the documentation and read the source code for answers.

I hope you've found this tutorial useful. If you have questions, comments
or suggestions to improve it, please write to <u>andras at omnetpp org</u>.

*/


/**
@mainpage TCP Tutorial for the INET Framework

The INET Framework contains a detailed and faithful TCP model.
This tutorial explains how you can trace what is going in
TCP simulation, how you can use TCP in your models, and
finally, we say a few words on how TCP can be extended.

It is expected that you have a basic understanding of OMNeT++'s
architecture (simple and compound modules, NED files, messages,
omnetpp.ini, etc.) and of TCP of course. If you need to brush
up your knowledge on the first topic, go through the TicToc
tutorial, and read the relevant parts of the Manual.

@section contents Contents

@ref part1
  - @ref sec1
  - @ref sec2
  - @ref sec3
  - @ref sec4
  - @ref sec5

@ref part2
  - @ref sec6a
  - @ref sec6b
  - @ref sec6c
  - @ref sec6d

*/
/* TBD next few lines are currently off

@ref part3
  - @ref sec7
  - @ref sec8
  - @ref sec9
  - @ref sec10
*/

/**
@page part1 Part 1: Tracing and analysing models using TCP

@section sec1 Getting started

Launch any network model that contains TCP. Two good ones to start
with are the INET/NClients and INET/REDTest example simulations.
If you use NClients, we recommend that you set the number of
client hosts to just one or two -- that'll make it easier
to follow the events. (Find and modify the "*.n=4" line
in omnetpp.ini.)

To start, run the simulation animated slowly (use the slider to adjust
speed) for a while. You should see the familiar SYN, SYN+ACK, ACK packets,
parts of the TCP three-way connection setup coming and going, and later you
should see data segments and ACKs. If you can catch connection teardown (if
you take note once when it occurs, in the next runs you can just
"fast-forward" there using the "Run until.." dialog), you should see FIN
and ACK packets in both directions. If these words don't ring a bell, you
should consider reading about TCP a bit before doing your simulations.

<img src="syn-anim.png">



@section sec2 Peeking inside TCP packets

Restart the simulation, and single-step through it by pressing
F4 repeatedly. You can open an inspector for the packet by
double-clicking its red icon.

However, sometimes that's not easy: animated packets may disappear before
you can click them. There are two ways around this. One is to reduce
animation speed to the minimum so that you have more time to click the
message icons en-route (still you have to be quick). The other way is to
find the packet inside the router or host it entered: double-click the icon
to see the internals, then check the inside of the ppp[0], ppp[1],
interfaces as well. You should see something like this:

<img src="syn-at-ppp.png">

Once you found the packet, you can double-click it to open an inspector
window.

<img src="syn-pppframe.png">

You'll find that the message represents the PPP frame. To see the IPv4
datagram header, click the "Encapsulated message" button to open the IPv4Datagram
inspector window. To get to the TCP header, you have to click the
"Encapsulated message" button of the datagram's inspector, and select the
Fields tab.

<img src="syn-segment.png">



@section sec3 Tracing the application's listen() and connect() calls

The application layer opens a connection by sending a message to TCP. This
message is called ActiveOPEN or PassiveOPEN (at least if you use the
TCPSocket utility class in the application; see later), and you can catch
it if you open the host module (dbl-click) before the application starts,
and execute the simulation event-by-event (single-step, F4). You should see
something like this:

<img src="activeopen.png">

The ActiveOPEN message corresponds to the connect() call, and PassiveOPEN
to the listen() call. The OPEN message itself doesn't contain data: all
connection parameters are in the "Control Info" structure. You can view it
by opening the inspector (double-click on the message) and selecting the
Control Info tab.

<img src="activeopen-fields.png">

The <i>connId</i> field identifies the connection from the application's
point of view. It is only used between the application and TCP, and it is
meaningless outside that context; for example, it is not sent in any TCP
packet. The connId has to be supplied (as part of the appropriate Control
Info object) in every message (data or control) the application sends to
TCP, so that TCP knows which connection the application is talking about.
Likewise, TCP attaches a Control Info with connId to every message it sends
to the application.

The other fields are parameters of the OPEN call. <i>Remote address</i>,
<i>remote port</i>, <i>local address</i> and <i>local port</i> shouldn't
require any further explanation. <i>Fork</i> is only used with passive OPEN
(listen()): if set to true, an incoming connection (SYN segment) will cause
the connection to fork, always leaving one connection listening for further
incoming connections. With fork=false, no connection forking is done, and
TCP will refuse futher incoming calls. The rest of the parameters are
specific to this TCP model, and will be discussed later.

The command type itself (active open, passive open, send, close, etc) is
carried in the numeric <i>message kind</i> field, which is displayed on the
first page of the inspector window. Likewise, when TCP sends a message up
to the application, it sets the <i>message kind</i> to indicate what it is:
a notification that the connection was established, notification that the
connection was closed by the remote TCP, that it was reset, that it timed
out etc; data arriving on the connection is also sent up by TCP, with
Control Info attached and message kind set to indicate that the message is
<i>data</i> or <i>urgent data</i>. The C++ API defines symbolic constants
for message kind values: <tt>TCP_C_OPEN_ACTIVE</tt> etc are in the
<tt>TcpCommandCode</tt> enum, and <tt>TCP_I_ESTABLISHED</tt> etc are
in the <tt>TcpStatusInd</tt> enum.



@section sec4 How can I "see" my TCP connections

The list of TCP connections is kept in the <tt>tcpConnMap</tt> and
<tt>tcpAppConnMap</tt> data structures inside TCP. The two data structures
contain the same connections, only they are indexed in a different way:
<tt>tcpConnMap</tt> is optimized for quickly finding a connection for an
incoming TCP segment, while <tt>tcpAppConnMap</tt> identifies the
connection from the applications' point of view, by <i>connId</i>.

To inspect these data structures, double-click TCP, select the "Contents"
tab and double-click the <tt>tcpConnMap</tt> or the <tt>tcpAppConnMap</tt>
entries. E.g. <tt>tcpConnMap</tt>:

<img src="tcp-conns.png">

<img src="tcp-connmap.png">

Currently, information about each connection is printed on a single,
very long line which gets wrapped. Later releases may present
connection data in a more structured form.


@section sec5 And where should I look to find information about open sockets?

There isn't such data structure in INET's TCP as "socket" -- the TCPSocket
class just stores the connId and offers a few convenience functions to connect,
send, close, etc.

@ref part2
*/

/**
@page part2 Part 2: Drawing sequence number charts

@section sec6a Recording data into output vectors

An established way of visualizing TCP behaviour is via drawing <i>sequence
number charts</i>. The TCP module records sequence numbers and other
info into OMNeT++ <i>output vectors</i>.

OMNeT++ <i>output vectors</i> are basically time series data: every output
vector contains an ordered sequence of <i>(time, value)</i> pairs, where
<i>time</i> is the simulation time at which <i>value</i> was recorded. All
output vectors from a single simulation run are written into an single
output vector file, which is usually called <tt>omnetpp.vec</tt> (or
<tt><i>something</i>.vec</tt>). Contents of the output vector file can be
displayed with the Plove program which is part of OMNeT++, but its contents
is textual so you can peek into it and process its contents with the tool
of your liking (e.g. awk, perl, octave, matlab, gnuplot, xmgrace, R,
spreadsheet programs, etc)

Output vectors that record sequence numbers in sent and received TCP segments
are the following:
   - <tt>"send seq"</tt>: sequence number in sent segment
   - <tt>"sent ack"</tt>: ack number in sent segment
   - <tt>"rcvd seq"</tt>: sequence number in received segment
   - <tt>"rcvd ack"</tt>: ack number in received segment

Most simulators count in segments. OMNeT++ TCP, being more faithful to the RFC,
counts in bytes.

In addition, the following are recorded:
   - <tt>"advertised window"</tt>: the "window" values in received segments
   - <tt>"congestion window"</tt>: commonly referred to as "cwnd"
   - <tt>"measured RTT"</tt>: round-trip time measured (time between sending a
     segment and the arrival of its acknowledgement, only measured if
     there was no retransmission)
   - <tt>"smoothed RTT"</tt>: the exponentially weighted average of measured
     round trip time values (according to Jacobson's algorithm)

Sometimes less is more, so you have control over which output vectors get
actually recorded into the file, and you can limit the interval
as well. For this you have to add a few lines to <tt>omnetpp.ini</tt>;
see the section about configuring output vectors in the OMNeT++ manual.


@section sec6b An example: Round-Trip Time plot

Let us see an example. Run the <i>REDTest</i> simulation for a while, then
stop and exit it. As a result, an <tt>omnetpp.vec</tt> file should appear in
the directory. You can open that in Plove by typing

<pre>
$ plove omnetpp.vec
</pre>

at the prompt. The Plove window will appear, and the list of output vector
files will be displayed in the left panel. Suppose you want to see
the round-trip times. We can plot output vectors from the right panel.
You can copy there things by double-clicking vectors in the left panel,
or by selecting one or more and using the arrow button in the middle.

<img src="plove-rtt.png">

Then create a plot using the toolbar button. After some zooming and customizing
(right-clicking the chart will present you with a context menu for setting
line styles, axis labels, etc) you might have something like the following:

<img src="plove-rttplot.png">

You can save the plot in Postscript or in GIF, or copy it to the clipboard
(Windows) for using it in reports and presentations.



@section sec6c Following TCP's operation on a sequence number plot

Let us see a sequence number plot from the same simulation:

<img src="plove-seqplot.png"></a>

This chart tells a whole story by itself. The horizontal axis is time, and
the vertical is TCP sequence numbers. It shows that TCP in "s1" was sending
data to "s2". In the beginning, the sequence numbers in sent segments
(blue) are growing steadily. These segments also arrive at "s2" after some
delay (see red dots by about .1s = 100ms to the right of the corresponding
blue dots). These segments get acknowledged by "s2" (green triangles --
they are one MSS = 1024 bytes above the red dots, because red dots
represent the first bytes of full-sized (1024 byte) segments that arrived,
while the acknowledgements carry the next expected sequence number as ack.)
These acknowledgements arrive at "s1" about 30ms later (yellow plus signs).
The asymmetry in delays (100ms vs 30ms) is because acks are smaller, which
results in smaller queueing delay at bottleneck links.

Then a segment gets lost: one red dot at around t=44.81s is missing,
meaning that that segment was not received by "s2" -- it was probably
dropped in a router. The sender "s1" TCP doesn't know this, so it keeps
transmitting until the window lasts.  But "s2" keeps sending the same ack
number (green triangles become horizontal), saying that it still wants the
missing segment. When these acks arrive at "s1", at the 4th ack (3rd
duplicate ack) it recognizes that something is wrong, and re-sends the
missing segment (solitary blue dot). Finally, after 100ms this segment
arrives at "s2" (solitary red dot), but until then "s2" sends further
duplicate acks for all the segments that were still in the queue.

When the missing segment arrives at "s2", ack numbers sent by "s2" jump up
(solitary green triangle at about t=44.93s), signalling that the segment
filled in the gap. (This also indicates that received segments above the
gap were not discarded by "s2", but rather preserved for the future).

When this ack arrives at "s1" (yellow cross under blue dot at t=44.95s),
"s1" knows that all is well now, and spits out several segments at same
time (blue dots in vertical line). This is because those duplicate acks
inflated the congestion window (by indicating that above-sequence segments
were received properly by "s2") -- this is the Fast Recovery algorithm. Of
course these segments arrive at "s2" one after another (sequence of red
dots starting at about t=44.98s is slanting), because they individually
have to queue up for transmission. "s1" TCP gets permission to send further
segments when further acks arrive (yellow crosses after t=45s).

That's all.



@section sec6d Other hints for plotting

When plotting sequence numbers in Plove, useful filters are the
"divide" filter (to translate from byte counts to segments), and "modulo"
(to fold continually growing sequence numbers to the same interval, so that
it can be displayed conveniently). To apply filters, right-click vectors in
panel, and choose Pre-plot filtering... from the context menu.

*/

/* * <-- TBD FIXME enable this again when it gets reasonably complete.

@page part3 Part 3: Using, configuring and programming the TCP model

@section sec7 Application models for TCP

There are several application or traffic generator models that work over
TCP. On the client side, there is <tt>TCPBasicClientApp</tt>,
<tt>TCPSessionApp</tt> and <tt>TelnetApp</tt>, and on the server side there
is <tt>TCPSinkApp</tt> <tt>TCPEchoApp</tt>, <tt>TCPGenericSrvApp</tt> and
<tt>TCPSrvHostApp</tt>.

The most flexible one is <tt>TCPBasicClientApp</tt>
combined with <tt>TCPGenericSrvApp</tt>.


@section sec8 Programming TCP

Natively, you talk to the TCP module with messages that have attached TCPCommand
object controlInfo. (You've seen that above).

A more advanced programming technique to talk to TCP is to use the TCPSocket
class, which takes the burden of assembling TCPCommand objects off your shoulder.
It also looks at messages arriving from TCP, looks into them, and calls methods
in your module such as socketConnected(), socketDataArrived(), etc.
You decide what should happen when the socket gets connected, when data
arrives, etc. by filling out the body of those methods.

A third way is to subclass from one of the application models.

@subsection sub1 The Pedestrian Way

These commands are represented by
the C++ constants <tt>TCP_C_OPEN_ACTIVE</tt>, <tt>TCP_C_OPEN_PASSIVE</tt>, <tt>TCP_C_SEND</tt>,
<tt>TCP_C_CLOSE</tt>, <tt>TCP_C_ABORT</tt> and <tt>TCP_C_STATUS</tt> (defined in the <tt>TcpCommandCode</tt>
enum).

To send data over TCP, you would....

TCP indications, sent by TCP to the application. TCP will set these
// constants as message kind on messages it sends to the application.
TcpStatusInd
    TCP_I_DATA
    TCP_I_URGENT_DATA
    TCP_I_ESTABLISHED
    TCP_I_PEER_CLOSED
    TCP_I_CLOSED
    TCP_I_CONNECTION_REFUSED
    TCP_I_CONNECTION_RESET
    TCP_I_TIMED_OUT
    TCP_I_STATUS

@subsection sub2 Using the TCPSocket class

...

@subsection sub3 Subclassing from Application Base Classes

...


@section sec9 Can I listen and connect using the same socket (or port)?

Nope. Not just in INET -- with any TCP, be it Linux's socket calls,
winsock or anything.

You could try with one socket, or two socket (one for listening, one for
connecting). With one socket, you'd have to issue listen() and then
connect() on the same socket. In theory this is not impossible (the TCP
state machine contains a LISTEN --> SYN_SENT transition), but in practice
this is probably not supported in most implementations. E.g. on Windows,
the connect() call is just ignored. Perhaps you can try on Linux, but I bet
it wouldn't work, and the man pages never mention this possibility.

If you try to use separate sockets for listening and for connecting, the
bind() call for the second socket will return EADDRINUSE (Address already in use).

So if you need to both listen and connect, use two sockets, and don't bind() the
connecting one (or at least use a different port number).



@section sec10 Do you want to transmit read data, or just "dummy bytes"?

Real-life TCP's always transmit byte stream. However, in a simulation
environment, that's not practical to mirror exactly: if you simulate the
transmission of 200 megabytes, would you really want to physically copy 200
megabytes of memory from one module to another? Probably not: if content
doesn't matter and only the number of bytes is important, then a TCP
segment carrying 4K data in the simulation can just carry an <tt>int</tt>
or <tt>long</tt> variable <tt>byteLength = 4096</tt> -- and there's no need
to actually allocate 4K of memory for that. Similarly, the "send queue" in
TCP doesn't need to actually store anything, except a <tt>queueLength</tt>
integer variable. Not suprisingly, storing just <tt>int</tt>s improves
simulation performance more than just a little.

In other cases, the content matters, and you really want actual bytes to
get transmitted.

But typically, in OMNeT++ simulations you don't work with byte arrays, but
with messages represented as C++ objects. For example, if you simulate HTTP,
the header might contain an array of strings...



@section sec11 TCP flavours

TCP supports several "flavours" of TCP such as Tahoe, Reno
and it is extensible with new ones.

*/



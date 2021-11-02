.. _ug:cha:packetfilter:

Packet Filter Expressions
=========================

A packet filter expression takes a packet as input and returns a boolean value
as output. It is used in many modules such as packet filters, packet classifiers,
packet schedulers, various visualizers, and so on. The filter expression can be
either a simple string literal or a full blown NED expression.

The string variant is basically a pattern that is matched against the name of
the packet. It can still contain boolean operators such as and/or/not but it
cannot refer to any part of the packet other than its name.

For example, the following pattern matches all packets having a name that starts
with 'ping' and that doesn't end with 'reply':

:ini:`"ping* and not *reply"`

The expression variant is evaluated by the module for each packet as needed. The
expression can contain all NED expression syntax in addition to several implicitly
defined variables as shown below. The implicitly defined variables can refer to
the packet itself, to a protocol specific chunk, to a chunk with a specific chunk
type, or to a packet tag with a specific type.

The following list gives a few examples:

:ini:`expr(hasBitError)` matches packets with bit error

:ini:`expr(name == 'P1')` matches packets having 'P1' as their name

:ini:`expr(name =~ 'P*')` matches packets having a name that starts with 'P'

:ini:`expr(totalLength >= 100B)` matches packets longer than 100 bytes

Implicitly defined variables can be used to check the presence of a chunk of a specific protocol or of a chunk of a specific type:

:ini:`expr(udp != null)` matches packets that have at least one UDP protocol chunk

:ini:`expr(has(udp))` shorthand for the above

:ini:`expr(has(udp[0]))` same as above using indexing

:ini:`expr(has(UdpHeader))` matches packets that have at least one chunk with UdpHeader type

:ini:`expr(has(UdpHeader[0]))` same as above using indexing

:ini:`expr(has(ipv4))` matches packets that have at least one IPv4 protocol chunk

:ini:`expr(has(ipv4[0]))` a packet may contain multiple protocol headers, they can be indexed

:ini:`expr(has(Ipv4Header))` matches packets that have at least one chunk with Ipv4Header type

:ini:`expr(has(Ipv4Header[0]))` same as above using indexing

:ini:`expr(has(ethernetmac))` matches packets that have at least one Ethernet MAC protocol chunk

:ini:`expr(has(ethernetmac[0]))` same as above using indexing

:ini:`expr(has(ethernetmac[1]))` this would most likely match the Ethernet FCS chunk

:ini:`expr(has(EthernetMacHeader))` matches packets that have at least one chunk with EthernetMacHeader type

:ini:`expr(has(EthernetMacHeader[0]))` same as above using indexing

The expression can also refer to fields of chunks:

:ini:`expr(ipv4.destAddress.getInt() == 0x0A000001)` matches packets with a specific binary IPv4 destination address

:ini:`expr(ipv4.destAddress.str() == '10.0.0.1')` same as above using strings

:ini:`expr(ipv4.destAddress.str() =~ '10.0.0.*')` matches packets that have an IPv4 destination address starting with '10.0.0.'

:ini:`expr(udp.destPort == 42)` matches packets if the UDP destination port number equals to 42

It's also possible to combine expressions using boolean operators:

:ini:`expr(name == 'P1' && totalLength == 128B && ipv4.destAddress.str() == '10.0.0.1' && udp.destPort == 42)`


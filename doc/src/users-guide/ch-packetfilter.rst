.. _ug:cha:packetfilter:

Packet Filter Expressions
=========================

A packet filter expression is used to evaluate a packet and return a boolean value.
It is used in several modules, such as packet filters, packet classifiers,
packet schedulers, various visualizers, and so on. The filter expression can be
either a simple string literal or a full-blown NED expression.

The string variant is basically a pattern that is matched against the name of
the packet. It can still contain boolean operators such as and/or/not, but it
cannot refer to any part of the packet other than its name.

For example, the following pattern matches all packets that have a name starting
with "ping" and do not end with "reply":

:ini:`"ping* and not *reply"`

The expression variant is evaluated by the module for each packet as needed. The
expression can contain all NED expression syntax, along with implicitly defined
variables shown below. These implicitly defined variables can refer to
the packet itself, a protocol-specific chunk, a chunk with a specific chunk
type, or a packet tag with a specific type.

The following list provides a few examples:

:ini:`expr(hasBitError)` matches packets with a bit error

:ini:`expr(name == 'P1')` matches packets that have 'P1' as their name

:ini:`expr(name =~ 'P*')` matches packets that have a name starting with 'P'

:ini:`expr(totalLength >= 100B)` matches packets that are longer than 100 bytes

Implicitly defined variables can be utilized to check the presence of a chunk of a
specific protocol or of a chunk of a specific type:

:ini:`expr(udp != null)` matches packets that have at least one UDP protocol chunk

:ini:`expr(has(udp))` shorthand for the above

:ini:`expr(has(udp[0]))` same as above using indexing

:ini:`expr(has(UdpHeader))` matches packets that have at least one chunk with the UdpHeader type

:ini:`expr(has(UdpHeader[0]))` same as above using indexing

:ini:`expr(has(ipv4))` matches packets that have at least one IPv4 protocol chunk

:ini:`expr(has(ipv4[0]))` a packet may contain multiple protocol headers; they can be indexed

:ini:`expr(has(Ipv4Header))` matches packets that have at least one chunk with the Ipv4Header type

:ini:`expr(has(Ipv4Header[0]))` same as above using indexing

:ini:`expr(has(ethernetmac))` matches packets that have at least one Ethernet MAC protocol chunk

:ini:`expr(has(ethernetmac[0]))` same as above using indexing

:ini:`expr(has(ethernetmac[1]))` this would most likely match the Ethernet FCS chunk

:ini:`expr(has(EthernetMacHeader))` matches packets that have at least one chunk with the EthernetMacHeader type

:ini:`expr(has(EthernetMacHeader[0]))` same as above using indexing

The expression can also refer to fields of chunks:

:ini:`expr(ipv4.destAddress.getInt() == 0x0A000001)` matches packets with a specific binary IPv4 destination address

:ini:`expr(ipv4.destAddress.str() == '10.0.0.1')` same as above using strings

:ini:`expr(ipv4.destAddress.str() =~ '10.0.0.*')` matches packets that have an IPv4 destination address starting with 10.0.0.

:ini:`expr(udp.destPort == 42)` matches packets if the UDP destination port number equals 42

Expressions can also be combined using boolean operators:

:ini:`expr(name == 'P1' && totalLength == 128B && ipv4.destAddress.str() == '10.0.0.1' && udp.destPort == 42)`

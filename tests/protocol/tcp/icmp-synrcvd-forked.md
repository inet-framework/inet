# C8a: A hard ICMP error in SYN_RCVD informs only an application that has the connection

RFC 9293 section 3.9.2.2 says that a hard ICMP error (Destination Unreachable, codes 2 to 4) SHOULD abort the connection (SHLD-26), and `Tcp` does so in SYN_SENT and in SYN_RCVD. For the application, the abort in SYN_RCVD must act as a RST in SYN_RCVD does. RFC 9293 section 3.10.7.3 says for that RST: "If this connection was initiated with a passive OPEN (i.e., came from the LISTEN state), then return this connection to LISTEN state and return. The user need not be informed."

A listener of INET usually forks (`TcpServerSocket`, the default of the server applications): at a SYN, `Tcp` makes a new connection with a new socket id, and the listener stays in LISTEN. The application gets the new socket id only with `TCP_I_AVAILABLE`, when the connection becomes ESTABLISHED (or at the SYN of Fast Open), and the socket exists for it only after it accepts the connection. Before that, the `MessageDispatcher` of the node does not know the socket id, and a message to it stops the run.

The original scenario, finding PF-16 of the [catalog of the parameter tests](../../../doc/project/evidence/model/tcp/parameters.md#findings), which the cross-class check of C8 found: host1 has `TcpSpoof` in place of its TCP and sends a SYN to host2:1000, where `TcpEchoApp` listens. host2 forks a connection and answers with a SYN-ACK. host1 has no TCP, so its IP answers the SYN-ACK with an ICMP Destination Unreachable, Protocol Unreachable (3/2). The forked connection of host2 aborted, and sent `TCP_I_CONNECTION_REFUSED` to the socket id of the forked connection. The dispatcher of host2 stopped the run with "Unknown socket: socketId = 1".

The cause is the condition of the indication in `TcpConnection::processIcmpv4Error()` and `TcpConnection::processIcmpv6Error()`: it informed a connection that was active, forked or Fast Open. A forked connection in SYN_RCVD without Fast Open always waits for its accept (`isToBeAccepted()`), so the forked case was always wrong. The handler of a RST in SYN_RCVD (`TcpConnection::processRstInSynReceived()`) had the right condition already.

The correction takes the condition of the RST: the application gets the indication only for an active OPEN, or for a Fast Open connection that the application has (`state->fastopenAccelerated && !isToBeAccepted()`). The state transition does not change: a forked or Fast Open connection closes, and a connection of a listener that does not fork returns to LISTEN.

```mermaid
sequenceDiagram
    participant H1 as host1 (TcpSpoof, no TCP)
    participant H2 as TCP of host2 (listener, forks)
    participant A as TcpEchoApp of host2
    H1->>H2: SYN, seq 12345
    Note over H2: fork: a new connection in SYN_RCVD, socket id 1, not known to the application
    H2->>H1: SYN-ACK, ack 12346
    H1->>H2: ICMP Destination Unreachable, Protocol Unreachable (3/2)
    Note over H2: the forked connection goes to CLOSED; the listener stays in LISTEN
    Note over H2,A: after the correction: no indication
    H2--xA: before the correction: TCP_I_CONNECTION_REFUSED for socket id 1, "Unknown socket"
```

## Direct checks

- [TcpSpoofIsSYNTrueNoTcp.test](parameter/spoof/TcpSpoofIsSYNTrueNoTcp.test), facet `true-no-tcp` of `TcpSpoof.isSYN`: the network `TapPairNetwork`, host1 with `TcpSpoof` and no application, host2 with `TcpEchoApp` at the port 1000. Steps: the SYN of the spoof leaves host1 at 1 s; host2 sends a SYN-ACK that acknowledges 12346; host1 sends the ICMP error 3/2; host2 sends no segment in the next 3 s, in which the timer of a connection in SYN_RCVD would send the SYN-ACK again. Before the correction the run stopped at the ICMP error with "Unknown socket"; after it, the test passes. With `isSYN = false` it fails, because no SYN-ACK comes.
- [tcp_icmp_synrcvd_2.test](../../module/tcp_icmp_synrcvd_2.test) and [tcp_icmp6_synrcvd_2.test](../../module/tcp_icmp6_synrcvd_2.test), module tests: the scripted tester answers the SYN-ACK of a forking server with Destination Unreachable 3/2 (ICMPv4) or 1/4 (ICMPv6). The trace shows the forked connection from SYN_RCVD to CLOSED, no indication to the application, and at 3 s a new forked connection for the retransmitted SYN of the client, which the application accepts (`TCP_I_AVAILABLE`, `TCP_I_ESTABLISHED`). The module tests have no dispatcher, so before the correction the application logged `TCP_I_CONNECTION_REFUSED`, and the trace did not match; after it, both pass. They are copies of `tcp_icmp_synrcvd_1.test` and `tcp_icmp6_synrcvd_1.test`, whose server does not fork and which keep their trace.

The checks cover a forked connection without Fast Open. They do not cover a Fast Open connection in SYN_RCVD that gets an ICMP error before or after its accept; for that case, the correction only takes the condition of the RST in SYN_RCVD. The 808 module tests that the filter `tcp` selects pass, except the 7 tests of IEEE 802.1Q tags that fail on `master` too, and the packetdrill wrappers keep their verdicts.

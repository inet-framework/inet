# C5: A stopped Tcp drops the segments that reach it

When a node stops, `stopOperationExtraTime` keeps the layers below TCP up for a time after the stage of TCP has ended, while TCP is down already. A peer does not know that the node stops, so its segments can arrive in this time, and the network layer gives them to TCP. A TCP that is down should take no part: the segment gets no answer, as on a node whose TCP is off.

The original scenario, finding PF-14 of the [catalog of the parameter tests](../../../doc/project/evidence/model/tcp/parameters.md#findings): host1 stops at 1 s with an extra time of 1 s, and host2 opens a TCP connection to host1 at 1.5 s. The SYN reaches the TCP of host1, which is down, and the run stopped with "Message 'SYN' received when Tcp is down".

The cause: `Tcp` did not override `handleMessageWhenDown()`, so the default of `OperationalMixin` handled the SYN, which stops the run for each message that arrives later than the time of the change of the state. The correction overrides it as `MacProtocolBase` does for a down interface: a message from the network layer (`TransportProtocolBase::isLowerMessage()`, the gate `ipIn`) is dropped with the signal `packetDropped` for a packet, and each other message keeps the error of the mixin, because a self-message or a command of an application at a stopped TCP is a model error.

```mermaid
sequenceDiagram
    participant T as Tester
    participant H1 as TCP of host1
    participant IP as Network layer of host1
    participant H2 as host2
    T->>H1: stop at 1.0 s; no connection waits, so TCP is down at once
    Note over IP: stays up for the extra time of 1 s
    H2->>IP: SYN, 1.5 s
    IP->>H1: SYN
    Note over H1: dropped, no answer (before: the run stopped)
    Note over IP: goes down at 2.0 s; the pings of host2 get no reply after it
```

## Direct checks

[TcpStopOperationExtraTimeOtherSegment.test](parameter/module/TcpStopOperationExtraTimeOtherSegment.test) stops host1 at 1 s with an extra time of 1 s while host2 sends a ping every 50 ms, and host2 opens a connection to host1 at 1.5 s. It checks that the SYN of host2 leaves, that the last echo reply is the reply to the ping 29 (1.975 s), and that the run goes on to its end. Before the correction the run stopped at 1.5 s; after it, the test passes. With the value 0s of the class `zero` it fails, because the network layer goes down at 1 s and no reply after it comes.

The check covers a SYN. A segment of an existing connection takes the same path, and so does an ICMP error, which is not a packet and so gets no signal; neither has a test of its own.

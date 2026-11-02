# C2: TcpWestwood takes an RTT sample from each ACK of new data

TCP Westwood sets its slow-start threshold after a loss from an estimate of the bandwidth, not by halving the window: the threshold is the estimate times the smallest round-trip time divided by a reduction factor (`TcpWestwood::recalculateSlowStartThreshold()`). The comment of `Tcp.tcpAlgorithmClass` and the class `TcpWestwood` promise this behavior. It needs two inputs that each ACK of new data supplies: an RTT sample, which gives the smallest RTT `w_RTTmin`, and the acknowledged bytes, which give the estimate `w_bwe`.

The original scenario, finding PF-6 of the [catalog of the parameter tests](../../../doc/project/evidence/model/tcp/parameters.md#findings): host1 runs TcpWestwood without SACK, sends 100000 bytes over a round trip of 40 ms, and loses one segment. At the fast retransmission, the threshold keeps its initial value and no `ssthresh` signal comes; the window becomes 4294967295 and then wraps to 0.

The cause is the order of two calls in `TcpWestwood::receivedAckForUnackedData()`. The function cleared the list of transmit times up to SND.UNA (`sentInfo.clearTo(snd_una)`) and then looked up the first acknowledged segment in it (`sentInfo.get(firstSeqAcked)`). The cleared list never holds that segment, so the function never took a sample: `w_RTTmin` kept its initial value `SIMTIME_MAX`, the estimate was never updated, and `recalculateSlowStartThreshold()` never ran, because both callers run it only for a real `w_RTTmin`.

The correction reads the item first and clears the list after the sample, as TcpVegas, the other algorithm that uses this list, does. It adds no state.

```mermaid
sequenceDiagram
    participant Peer
    participant W as TcpWestwood
    participant L as Transmit time list
    Peer->>W: ACK of new data (first acknowledged byte: firstSeqAcked)
    W->>L: get(firstSeqAcked)
    L-->>W: first send time and transmit count of the segment
    Note over W: RTT sample: w_RTTmin; acknowledged bytes: w_bwe
    W->>L: clearTo(SND.UNA)
    Note over W: before the correction the clear came first, so get() found nothing
```

## Direct checks

[TcpAlgorithmClassWestwood.test](parameter/congestion/TcpAlgorithmClassWestwood.test) drops the 15th data segment of host1 once. It checks that a threshold comes in the same instant as the fast retransmission, that the window is then at least two segments, and that the next duplicate ACK makes it larger. Before the correction it failed, because no threshold came; after it, it passes, and it fails with each other algorithm.

[TcpAlgorithmClassWestwoodRto.test](parameter/congestion/TcpAlgorithmClassWestwoodRto.test), new in this commit, drops the last data segment once, with TLP off, so only the timer sends it again. At the timeout, the threshold that host1 signals must equal the estimate times the smallest RTT divided by the reduction factor, read from the state of the connection (level 3: no signal carries the estimate), and the smallest RTT must be a real sample. Before the correction it failed; after it, it passes, and it fails with each other algorithm, which has no estimate.

The fingerprints of `examples/inet/tcpclientserver`, configuration `inet-westwood`, change with the correction, and so does its statistical baseline; both are updated deliberately with this commit.

The checks cover one loss in a short transfer. They do not check the value of the estimate against the capacity of the path, which depends on the ACK pattern.

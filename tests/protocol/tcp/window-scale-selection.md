# C1: The automatic window scale factor represents the whole advertised window

With `windowScalingFactor = -1`, the NED comment of `Tcp` promises "the smallest shift count that represents the largest window that the connection can offer". RFC 7323 section 2.2 lets a receiver offer at most 65535 << s bytes with the shift count s that its SYN announces, so a shift count that is too small takes the last bytes of the offer away for the whole connection.

The original scenario, finding PF-4 of the [catalog of the parameter tests](../../../doc/project/evidence/model/tcp/parameters.md#findings): host1 has `advertisedWindow = 262144` and sends a SYN with the shift count 2. The largest window of the shift count 2 is 65535 << 2 = 262140 bytes, so host1 can never offer the last 4 bytes of its window. Linux `tcp_select_initial_window()` announces 3 for this window.

The cause had two parts in `TcpConnection::writeHeaderOptions()`. First, the window came from the sequence numbers of the receive queue, and in the SYN of an active opener it was one byte less than the advertised window; the SYN-ACK of a passive opener had the exact window. Second, a loop shifted the window right until it fit into 16 bits, and this truncation stops one shift count too early when the window is just above the largest window of a shift count: 131071 >> 1 = 65535 gives 1. So the SYN got a shift count one too small for the windows in the 2^s bytes above the largest window of each shift count s, and the SYN-ACK for the same windows except the last one of each band.

The correction takes the largest window directly, as the larger of the receive buffer and the window clamp, or the bound of auto-tuning, and it selects the smallest s from 0 to 14 for which 65535 << s is at least that window. It adds no state. The table shows the shift count for some windows:

| Window (bytes) | SYN before | SYN-ACK before | After, both | The largest window of the new shift count |
| ---: | ---: | ---: | ---: | ---: |
| 65535 | 0 | 0 | 0 | 65535 |
| 65536 | 0 | 1 | 1 | 131070 |
| 131070 | 1 | 1 | 1 | 131070 |
| 131071 | 1 | 1 | 2 | 262140 |
| 131072 | 1 | 2 | 2 | 262140 |
| 262144 | 2 | 3 | 3 | 524280 |
| 350000 | 3 | 3 | 3 | 524280 |

For 131071 bytes Linux announces 1, because it computes `ilog2(space) - 15`; INET follows its own NED comment here. No diagram is given: the change is a computation on one host, and the table shows its inputs and results.

## Direct checks

[TcpWindowScalingFactorAuto.test](parameter/setup/TcpWindowScalingFactorAuto.test) gives host1 a window of 262144 bytes and checks that its SYN announces the shift count 3. Before the correction it failed with 2; after it, it passes. It also fails with each other class of `windowScalingFactor` (the cross-class check).

[TcpWindowScalingFactorAutoBoundary.test](parameter/setup/TcpWindowScalingFactorAutoBoundary.test) gives host1 a window of 131071 bytes, one byte above the largest window of the shift count 1, and checks that its SYN announces 2. [TcpWindowScalingFactorAutoBoundarySynAck.test](parameter/setup/TcpWindowScalingFactorAutoBoundarySynAck.test) gives the same window to host2, the passive opener, and checks that its SYN-ACK announces 2. Both tests failed with 1 when the selection of before the correction ran in a build of this commit, and both pass after it.

[tcp_synack_window_1.test](../../module/tcp_synack_window_1.test), a module test with an expected output, has a window of 131072 bytes at a passive opener, which already announced 2. It passes unchanged, and so it checks that the correction keeps a shift count that was right.

The checks cover the shift count of the SYN and of the SYN-ACK. They do not check a window field after the handshake, or a transfer that fills the whole window.

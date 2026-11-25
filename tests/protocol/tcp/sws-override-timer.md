# The override timer of the sender SWS avoidance

The sender silly-window-syndrome (SWS) avoidance of RFC 9293 section 3.8.6.2.1 holds data when the window of the peer is open but too small to use: smaller than one segment, smaller than the queued data, and smaller than half of the largest window that the peer offered. Rule (4) of the section ends such a hold: "it is necessary to have a timeout to force transmission of data, overriding the SWS avoidance algorithm", and "The override timeout should be in the range 0.1 - 1.0 seconds." The section also says: "It may be convenient to combine this timer with the timer used to probe zero windows".

`TcpAlgorithmBase` does so. The persist timer has two roles: the zero-window probe of section 3.8.6.1, and the override timeout. `sendData()` starts it when data waits and nothing is in flight. When it expires with an open window, `processPersistTimer()` sends what the window allows, as Linux `tcp_write_wakeup()` does; with a zero window it sends a probe.

Three faults are repaired here (BD-21 and BD-23 of `plan/pending/baseline-catch-up-defects.md`, and one that the review of the repair found).

## C6: a segment of the peer kept the held data waiting

The original scenario: a sender holds data, and the peer sends data with the same window at intervals shorter than the RTO. Each segment of the peer acknowledges SND.UNA and keeps the window, and `processWindowUpdate()` treated it as a window update: it cancelled the persist timer, and `sendData()` started it again with a full RTO. The held data left only one RTO after the last segment of the peer; a peer that never stops sending held it for ever.

`startPersistTimer()` now records the role of the timer in the state field `persistOverride`: the override when the window is open and nothing is in flight. `processWindowUpdate()` cancels only a probe timer. An override timer keeps running through a segment that updates no window, and it stops when the held data left. When the window closes while the override timer runs, `restartOverrideTimerAsProbe()` starts the timer again in the probe role, so that the first probe comes one RTO after the window closed (SHLD-29). Before, the timer kept what was left of the override, and the first probe came too early.

```mermaid
sequenceDiagram
    participant H1 as host1 (sender)
    participant H2 as host2 (peer)
    H2->>H1: ACK, window 500 bytes: host1 holds the rest
    Note over H1: persist timer starts in the override role
    H2->>H1: data, ACK = SND.UNA, window 500
    Note over H1: before C6: timer cancelled and started again; now it keeps running
    H2->>H1: data, ACK = SND.UNA, window 500
    Note over H1: the override timeout expires
    H1->>H2: the held data, as much as the window allows
```

## C7 and C8: the interval of the override

The override timer started with the RTO, which is at least 1 s in `~Tcp` and grows with the backoff. C8 holds the RTO in the range 0.1 s to 1.0 s of the RFC in the override role, and then at most `maxPersistTimeout`. The interval of a zero-window probe does not change. With the defaults of `~Tcp` the RTO is at least 1 s, so the override is 1.0 s.

Linux sends such held data from its probe timer, whose interval is the RTO (`tcp_check_probe_timer()`, `tcp_probe_timer()`, `tcp_write_wakeup()`). The parameter `swsOverrideAfterRto` of C7 keeps the RTO, and `~LinuxTcp` sets it.

## Direct checks

The protocol tests are in `rfc/`; each host has an MSS of 1000 bytes, so a full segment with the TS option holds 988 bytes, and the peer offers 3464 bytes, so its window after three segments is 500 bytes.

- [Rfc9293SwsOverride.test](rfc/Rfc9293SwsOverride.test): with no traffic of the peer, the held data leaves one RTO after the hold. It passes before and after the repairs.
- [Rfc9293SwsOverridePeerData.test](rfc/Rfc9293SwsOverridePeerData.test): the peer sends data at 0.35 s, 0.65 s and 0.95 s with the same window, and the held data must leave before 1.3 s. Before C6 it left at about 1.95 s, and the test failed; now it passes. The module test `tests/module/tcp_sws_4.test` checks the same case with the packet dump of `LegacyTcp`.
- [Rfc9293SwsOverrideWindowUpdate.test](rfc/Rfc9293SwsOverrideWindowUpdate.test): a window update of a full segment ends the hold at once, before the override timeout. It passes before and after.
- [Rfc9293SwsOverrideZeroWindow.test](rfc/Rfc9293SwsOverrideZeroWindow.test): the window closes at 0.5 s while the override timer runs, and the first probe must come one RTO, 2 s, later. With C6 to C8 before the fix it came at 1.2 s, and the test failed; now it passes.
- The parameter tests [TcpSwsOverrideAfterRtoRfcUpper](parameter/send/TcpSwsOverrideAfterRtoRfcUpper.test) (an RTO of 2 s: the data leaves after 1.0 s), [TcpSwsOverrideAfterRtoRfcLower](parameter/send/TcpSwsOverrideAfterRtoRfcLower.test) (an RTO of 10 ms: after 0.1 s) and [TcpSwsOverrideAfterRtoLinux](parameter/send/TcpSwsOverrideAfterRtoLinux.test) (an RTO of 2 s with the value of `~LinuxTcp`: after 2 s). The first two fail before C8.

## Recorded results

No recorded result changes. All 1847 stored fingerprint runs, in release mode with all features but the two OSG features: 1836 PASS, 1 ERROR that a marker declares (`tests/fingerprint/ethernet-nonstandardspeed.ini`), and 10 runs of `showcases/visualizer/osg` that a build without OSG can not start. All 945 statistical tests, in the image of the CI against the baselines of master: 934 PASS and 11 expected SKIP.

The entry BD-21 found the fault in `examples/inet/bulktransfer` `inet_inet_2b` at step 30 of the baseline catch-up, with 3223 starts of the persist timer. On master that simulation holds no data any more: a debug run with the repair starts the persist timer 0 times, and the repair touches no condition of a hold. So no stored simulation reaches the case, and the protocol tests above are its only check.

## Limits

- By a reading of `tcp_ack()` and `tcp_ack_probe()` that was not checked against the source of Linux 7.1, Linux restarts its probe timer at an ACK that leaves the head of the queue outside the window, but not at an in-order data segment of the fast path, which is the case of BD-21. `~LinuxTcp` never restarts the override timer, so it can differ from Linux at a pure ACK or at a window that grows but stays too small.
- A `maxPersistTimeout` below 0.1 s gives an override below the range of the RFC; the parameter limits each interval of the timer.
- Two older waits stay, and the plan records them: data that TCP_CORK holds below one segment, with a window below it, can wait as long as the cork stays set, because the override does not bypass the cork; and a retransmission timer that runs with nothing in flight blocks the start of the override timer.
- No test checks a window that grows during the hold but stays too small.

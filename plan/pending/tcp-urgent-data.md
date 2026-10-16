# Urgent data (T8)

Status: **done** — item T8 of `plan/pending/tcp-open-items.md`, decision D-131; landed on master on 2026-10-16, the owner approved it.

- Branch `topic/tcp-urgent-data`, on master `bbad103b0f`.
- The decision numbers continue those of `plan/done/tcp-hystartpp.md` (D-161 onward).
- The oracles are RFC 9293 (section 3.8.5, the SEND and RECEIVE calls of section 3.10, and the sixth step of section 3.10.7.4), RFC 6093, and the code of Linux 7.1 (`tcp_check_urg()`, `tcp_urg()` and `tcp_recv_urg()` in `tcp_input.c` and `tcp.c`, the urgent pointer of `tcp_transmit_skb()`). This host has no `packetdrill` binary, so no new script can get a Linux verdict. One script of the suite has an urgent segment, `splice/tcp_splice_loop_test` (Linux splices 99 of 100 bytes, because the last urgent byte leaves the stream); PacketDrillApp does not run `splice()` yet, so the script passes without a check.

## 1. The problem

RFC 9293 section 3.8.5: "TCP implementations MUST still include support for the urgent mechanism (MUST-30)": a sequence of urgent data of any length (MUST-31), the urgent pointer at the octet after the urgent data (MUST-62), a notice to the application when an urgent pointer arrives or advances (MUST-32), and a way for the application to learn how much urgent data remains (MUST-33). INET has the URG bit and the urgent pointer in the header and the fields `snd_up` and `rcv_up` in the state, but it never sets URG, a receiver ignores it ("TODO URG currently not supported"), the indication `TCP_I_URGENT_DATA` is never sent, and the socket has no urgent send.

Linux reads the urgent pointer as RFC 9293 does (`tcp_stdurg = 0`), but it takes the last urgent byte out of the stream: the application reads it with `recv(MSG_OOB)`, a normal read stops at it and skips it, `SIOCATMARK` tells whether the read position is at it, and `SO_OOBINLINE` keeps it in the stream. RFC 6093 describes this difference between the implementations.

## 2. The steps

| Step | Commit | Test |
| --- | --- | --- |
| 1 ✅ | `plan: add: urgent data` | — |
| 2 ✅ | `tcp: add: the urgent send` | `tcp_urgent_1` (new): URG and the urgent pointer in each segment before the end of urgent data of three segments, none after its ACK |
| 3 ✅ | `tcp: add: the urgent receive of RFC 9293` | `tcp_urgent_2` (new): the notice, the deliveries that end at the mark, the urgent bytes that remain, a second notice when the pointer advances |
| 4 ✅ | `tcp: add: urgentOutOfBand, the out-of-band byte of Linux` | `tcp_urgent_3` (new): the byte leaves the stream in `LinuxTcp`; `tcp_urgent_4` (new): `setOobInline()` keeps it |
| 5 ✅ | `doc: change: the release note of urgent data` | WHATSNEW entry 56 |
| 6 ✅ | `plan: change: record urgent data` | — |
| 7 ✅ | (inet-gpl) `PacketDrillApp: urgent data of the socket API` | after the landing, because inet-gpl builds against INET master (D-164) |

The test client of the module tests gets an urgent send in its send script and prints the notices and the urgent deliveries.

## 3. The checks

As for T7: each commit builds and passes the module tests, a new test fails before its commit, the CI fingerprint job and the TCP statistics, the packetdrill suite of inet-gpl, INET's TCP protocol suite with inet-gpl, and the gates. No example sends urgent data, so no baseline should move. In the packetdrill suite, `LinuxTcp` gets the out-of-band byte, so the receive side of `tcp_splice_loop_test` changes, but its verdict should stay MATCH; after step 7, the script checks it.

## 4. The decisions

- **D-161 — The urgent send.** `TcpSocket::sendUrgent()` adds the tag `TcpSendUrgentReq` to the data; TCP sets SND.UP to the end of the send queue after this data, as the SEND call of RFC 9293 ("SND.UP <- SND.NXT") and Linux `tcp_mark_urg()` (`snd_up = write_seq`). While SND.UNA is before SND.UP, each segment that starts before SND.UP, retransmissions and ACKs included, carries URG and the urgent pointer SND.UP - SEG.SEQ (MUST-62), as `tcp_transmit_skb()`; a pointer that does not fit in 16 bits is 65535 when SEG.SEQ + 65535 lies beyond SND.NXT, else the segment has no URG, as Linux. While urgent data is outstanding, the Nagle algorithm holds no segment, as Linux `tcp_nagle_test()`, for the "timely delivery" that RFC 9293 section 3.8.5 wants.
- **D-162 — The urgent receive of RFC 9293.** In ESTABLISHED, FIN-WAIT-1 and FIN-WAIT-2, a URG segment gives the pointer SEG.SEQ + SEG.UP. As Linux `tcp_check_urg()`, the receiver ignores a pointer whose urgent data the application has received, one whose last urgent byte is before RCV.NXT (no replay), and one that does not advance RCV.UP. A new or advanced RCV.UP sends the application the indication `TCP_I_URGENT` with `TcpUrgentInfo`, the bytes before the mark that it has not received (MUST-32, MUST-33); `TcpSocket` calls the new callback `socketUrgentArrived()`. The data before the mark comes with the kind `TCP_I_URGENT_DATA` (`socketDataArrived()` with `urgent = true`), each delivery ends at the mark ("data following the urgent pointer ... cannot be delivered to the user in the same buffer with preceding urgent data"), and the tag `TcpUrgentInd` tells the urgent bytes that remain after it, as the URGENT flag of RECEIVE.
- **D-163 — The out-of-band byte of Linux.** The parameter `urgentOutOfBand` (`Tcp` false, `LinuxTcp` true) takes the last urgent byte out of the stream. The data before it comes as `TCP_I_DATA`, as in Linux, and a delivery ends before the byte (a read of Linux stops at the mark); the byte comes as a packet of one byte with the kind `TCP_I_URGENT_DATA` and the tag `TcpOutOfBandInd`, and `TcpUrgentInfo` does not count it. A newer mark leaves an older byte in the stream, except an unread one at the read position, which is dropped, as `tcp_check_urg()`. `TcpSocket::setOobInline(true)` keeps the byte in the stream for one connection (`SO_OOBINLINE`).
- **D-164 — PacketDrillApp after the landing.** `send(MSG_OOB)`, `recv(MSG_OOB)` with the results of `tcp_recv_urg()` (the byte; `EINVAL` without a byte, after its read, or with `SO_OOBINLINE`; `EAGAIN` before it arrives), `setsockopt(SO_OOBINLINE)`, `ioctl(SIOCATMARK)`, and `splice()` as a read that stops at the urgent mark, so that `tcp_splice_loop_test` checks the out-of-band byte.

## 5. The record

Commits on master `bbad103b0f`: `34ae30d28c` (plan), `c6707b8d95` (the urgent send), `0f539ef4c8` (the urgent receive of RFC 9293), `dc3df5e1b6` (the out-of-band byte of Linux), `5c3f1a2504` (the release note), and this record. The landing on 2026-10-16: master moved to `484d3c0258` by a fast-forward. Step 7 is inet-gpl `0bea63d`, on inet-gpl master `0dfbcae`: `send(MSG_OOB)`, `recv(MSG_OOB)` with the results of `tcp_recv_urg()`, `SO_OOBINLINE`, `ioctl(SIOCATMARK)`, a read that stops at the mark, and `splice()`. PacketDrillApp did not know the symbol `NULL`, so it skipped each call with a `NULL` argument without a word; it is 0 now. With it, `tcp_splice_loop_test` checks the out-of-band byte: it passes with `LinuxTcp`, and it fails with `urgentOutOfBand = false` ("splice() reads 1 bytes, the script expects 0"). The packetdrill suite keeps each of its 318 verdicts, the scripts with a `NULL` buffer included, and INET's TCP protocol suite gives 329 PASS and 4 FAIL (expected).

Facts found during the work:

- `TcpConnection::sendToIP()` is the one path of each segment of a connection, so URG and the urgent pointer go there, for data, ACKs, FINs and retransmissions alike, as in `tcp_transmit_skb()`.
- The explicit READ had its own copy of the delivery; both delivery paths now take their data through `extractDataForApp()`, which ends a delivery at the urgent mark and sets the kind and the tags.
- The out-of-band byte comes from the segment that holds it, in the sixth step, before the data of that segment reaches the application, as `tcp_urg()` sets `urg_data` before `sk_data_ready`. With `ByteCountChunk` data, the byte has no value; with `BytesChunk` data, it has the value of the stream.
- PacketDrillApp does not run `splice()`, `ioctl()` or recv flags yet (step 7), so the out-of-band byte changes no verdict of the suite: `splice/tcp_splice_loop_test` passes without a check.
- A receiver of `LinuxTcp` that gets urgent data from an application of INET now gets the out-of-band byte; no example or baseline sends urgent data.

| Evidence | Result |
| --- | --- |
| Each commit builds alone; module tests at each commit | 683, 684, 685, 687, 687 PASS; no undefined `inet::` symbol |
| The new tests fail before their commits | yes, each of `tcp_urgent_1` to `_4` |
| CI fingerprint job (podman, clang-16) at the three code commits | OK at each; the one ERROR row of `ethernet.csv` is the expected one of master |
| Statistics of the 38 TCP working directories, against statistics `ebda488` | 122 KEEP, 4 SKIP (expected), no file changes: no example sends urgent data |
| Packetdrill suite of inet-gpl `0dfbcae` | 303 MATCH, 12 KERNEL_DRIFT, 3 UNSUPPORTED_FEATURE; each of the 318 verdicts as before |
| INET's TCP protocol suite with inet-gpl | 333 tests: 329 PASS, 4 FAIL (expected) |

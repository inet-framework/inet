# ARP standards tests — level 3

> **Kind:** how · **Status:** in progress · **Stands on:**
> [derive-tests-from-a-standard.md](../../doc/project/guide/derive-tests-from-a-standard.md)

ARP has no evidence tree yet. One test sits in `tests/protocol/arp/` — `ArpResolution.test`
— and it comes from the framework demo, not from a standard. This plan runs the whole
workflow for ARP at target level 3, so the pass does the work of levels 1, 2 and 3 in one
go. The work is on `master`, in the worktree `inet-master`.

## The in-scope set, and why

The RFC-editor metadata of RFC 826 names two documents that update it: RFC 5227 and
RFC 5494. RFC 1122 states the host requirements for ARP in §2.3.2. The level table of the
guide puts the host-requirements document at level 3 and the timer documents at level 4.
That gives:

| Document | Relation | In scope now | Reason |
| --- | --- | --- | --- |
| RFC 826 | `base` | yes | the protocol |
| RFC 1122 §2.3.2, §2.3.3, §2.4 | `companion` | yes | the host requirements: cache flush, flood prevention, the packet queue, no error report |
| RFC 5494 | `updates` | yes | the reserved and the experimental values of `ar$hrd` and `ar$op`; it makes a crafted opcode a defined edge case |
| RFC 5227 | `updates` | no | the address conflict detection is a control loop with five timers (PROBE_WAIT, PROBE_NUM, ANNOUNCE_WAIT, ANNOUNCE_INTERVAL, DEFEND_INTERVAL). The guide files timers and control loops at level 4 |
| RFC 903 | separate protocol | no | RARP is its own protocol |
| RFC 1027 | `companion` | no | proxy ARP over a subnet gateway; level 5 |
| RFC 1868 | experimental | no | UNARP was never a standards-track update |

RFC 826 predates RFC 2119. It carries two `must` words and no other keyword, so most of its
catalog is `description`, and the level of a feature comes from the "only path" rule of the
guide.

## What level 3 adds for ARP

Level 2 for ARP is the normal exchange: a request goes out broadcast, the target answers
directly, the datagram follows, and the second datagram needs no request. Level 3 needs a
crafted packet, which no application can produce:

- a request whose target protocol address is a third host (RFC 826 discards it after the
  merge step, and answers nothing);
- an unsolicited reply (the same discard, through the other branch of the opcode test);
- a packet with the experimental opcode 24 of RFC 5494;
- a packet whose sender protocol address is zero, the shape RFC 5227 calls an ARP Probe;
- a second request from the same protocol address with a different hardware address, which
  supersedes the table entry.

The level 3 toolset is therefore **injection**, and one check uses **interception**. Both
are in the framework already.

## Steps

- [ ] **Step 1 — download.** `standard/rfc826/rfc826.txt`, `standard/rfc5494/rfc5494.txt`.
      RFC 1122 is cached already, from the IPv4 pass.
- [ ] **Step 2 — standards map.** `protocol/arp/standards.md`: the document list, the
      override table, the in-scope set, and the target level.
- [ ] **Step 3 — catalogs.** `standard/rfc826/catalog.md` and
      `standard/rfc5494/catalog.md`, new. `standard/rfc1122/catalog.md` gets an ARP part
      whose identifiers start with `A`; its scope statement and its title change, as the
      UDP pass changed them.
- [ ] **Step 4 — features.** `protocol/arp/features.md`.
- [ ] **Step 5 — checks.** `protocol/arp/checks.md` plus one file per feature under
      `protocol/arp/checks/`.
- [ ] **Step 6 — tests.** `tests/protocol/arp/`, one `.test` per check, plus
      `ArpMutations.h` for the crafted packets. `ArpResolution.test` goes away: the suite
      holds only standards-derived tests, and `Rfc826AddressResolution` replaces it. The
      cookbook heading of `lib/AUTHORING.md` keeps its snippet and loses the file name.
- [ ] **Step 7 — run and analyze.** `model/arp/results.md`, with the run record.
- [ ] **Step 8 — claims and conformance.** `model/arp/conformance.md`.
- [ ] **Step 9 — categories.** `model/arp/categories.md`.
- [ ] **Ledger.** `model/arp/coverage.md`, and `model/arp/notes.md`.
- [ ] **Commit.** One commit per step, and the links checked.

## Facts found during the work

(filled in as the work goes)

## Result

(filled in at the end)

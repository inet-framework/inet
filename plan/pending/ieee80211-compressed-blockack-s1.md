# S1: repair Block Ack recipient capacity and delivery

> **Kind:** plan · **Status:** snapshot 2026-10-14 · **Seal:** none · **Owns:** — · **Stands on:** [contribution procedure](../../doc/project/guide/contribute-a-change.md), [test rules](../../doc/project/rule/testing.md)

S1 uses three recipient commits. The user authorizes capacity repair, ordered delivery, and BAR record creation in that order. The user also authorizes fingerprint updates at each source boundary. The first commit repairs capacity. This second commit repairs ordered delivery. The last commit will add BAR record creation.

The base is `ac6344cc5bcac972012759281a11b3f8255b5d46`. The original reviewed topic is `3fa8cbd1d131e1948bf77cfc2413899b034afcc0`. The rewrite retains that topic's final source, fixture, release text, migration text, and S1 documents.

The parent counts each fragment as one buffer position. A one-position buffer accepts fragment 0 but rejects fragment 1 of the same MSDU. `ReceiveBuffer` now counts occupied sequence positions from its map. All fragments of one MSDU occupy one position. A duplicate fragment consumes no extra position.

Scenario 0 supplies an accepted Basic agreement through the production recipient service. It requires both fragments to complete the MSDU and checks the window count. The fixture does not prove ADDBA buffer-size negotiation. Basic fragment retention remains an INET compatibility choice.

The capacity commit must pass scenario 0 and the three current recipient NAV runs in debug mode. The selected legacy fingerprint cases must pass against the capacity commit's own baselines. A later source change must carry its own baseline changes.

The reorder owner now releases every consecutive complete unit when DATA fills a gap. It returns released units in cyclic order. The recipient preserves that order through reassembly and A-MSDU deaggregation. DATA beyond the window moves the window, releases older complete units, and discards incomplete older units.

BAR release uses the same ordered result for a current record. A BAR before DATA still creates no record at this stage. The fixture now covers scenarios 0, 1, 2, 4, 5, 6, and 7. Scenario 3 belongs to the last commit. The window rules cite IEEE Std 802.11-2024, Clauses 10.25.6.6.2.1 and 10.25.6.6.3.1.

# The INET 4.8 release notes after the master of 2026-11-25

## Summary

The themes of the 4.8 release notes ([plan/done/whatsnew-4.8.md](../done/whatsnew-4.8.md)) cover the commits up to d69fc0eaff. Since then, `master` has 110 more commits (up to c7e7cdaabd). They add seven small entry files and one section of the migration guide. This plan folds the new entries into the themes, in the short form of the themes, and adds the changes that have no entry.

## Facts

- Branch `topic/whatsnew-4.8`, rebased onto `origin/master` c7e7cdaabd without a conflict. The new commits add files only; they change no entry file of the themes.
- New entries: `incompatible/ipv4-igmp-and-mld-corrections.txt` (1653 words), `incompatible/ipv6-neighbor-discovery-corrections.txt` (652 words), `fixes/icmpv6-mld-report-before-link-local-address.txt`, three IEEE 802.11 fixes of Miguel González-López (`ieee80211-basic-block-ack-receipts`, `ieee80211-block-ack-duplicate-drop-reporting`, `ieee80211-group-frame-reservation`), and `fixes/examples-lan80211-mobility-rng.txt` (also his).
- The IGMP and MLD entry says that a Report or a Done waits until the link-local address is tested. The later commits da2bb530c8, 617ce3bad0 and d5418ec4c3 changed that: such a message now leaves from the unspecified address, and the node reports its groups again when the address becomes valid.
- No entry describes 7a69bbc8fd (a router does not forward a multicast datagram from the unspecified address) or 481b429765 (PimChecksumInsertionHook no longer crashes on a datagram with an IPv6 extension header, with computed checksums).
- The other new commits are plans, tests, comments, the CREDITS file, two re-recorded showcase videos, and a TCP comment commit: none of them changes the behavior of a model.

## Decisions

- **C-1** The IGMP and MLD entry, with the MLD fix, becomes one short theme in `ipv4-igmp-and-mld-corrections.txt`, which states the final behavior.
- **C-2** The Neighbor Discovery corrections join `ipv6-neighbor-discovery.txt`.
- **C-3** The three IEEE 802.11 fixes join `ieee80211-fixes.txt`, and the lan80211 fix joins the examples theme; both credit Miguel González-López.
- **C-4** The entry of the changed results and the introduction name the new changes.

## Steps

1. [ ] Fold the new entries into the themes (C-1 to C-3). Commit.
2. [ ] The changed results and the introduction (C-4). Commit.
3. [ ] Check: `whatsnew.py check`, `check-pull-request.sh`. Move this plan to `plan/done/`. Commit.

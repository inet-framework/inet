# ND — coverage ledger

> **Kind:** ledger · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [rfc4861/catalog.md](../../standard/rfc4861/catalog.md), [rfc4862/catalog.md](../../standard/rfc4862/catalog.md), [rfc5942/catalog.md](../../standard/rfc5942/catalog.md), [rfc6980/catalog.md](../../standard/rfc6980/catalog.md), [features.md](../../protocol/nd/features.md), [results.md](results.md)

The single place that holds the changing state of the ND workflow. Every other artifact of
this protocol states what a standard says and never changes again unless the standard
changes. This one changes on every pass.

**No step edits an artifact of an earlier step. Steps 5, 6 and 7 record their outcome here.**

State of the ledger, from this run:

- Date: 2026-09-29 17:29 +0200
- INET: branch `master`, commit `24675c3a37`, tree clean
- Trees: src `8b4f86968e`, tests/protocol `1f1d62beca`
- OMNeT++: 6.4.0, commit `cf58891643`
- Build: debug, built from this commit
- Compiler: Ubuntu clang version 23.0.0
- Platform: Ubuntu 26.04.1 LTS, Linux 7.0.0-34-generic x86_64
- Command: `inet_run_protocol_tests -p inet -m debug -w '^tests/protocol/nd$'`
- Suite: 37 tests, 22 PASS, 15 FAIL (unexpected), 0 FAIL (expected), so the suite reports FAIL
- Target level: 2

The analysis of every failure is in [`results.md`](results.md#the-model-gaps).

## Statement coverage

`Status` is the state of the workflow, not of the standard:

| Status | Means |
| --- | --- |
| `selected` | a check targets it |
| `covered` | a selected check establishes it as a side effect, or it is a permission that a check accepts |
| `owed` | the model claims the behavior and **no check exists yet**; the note names what a check would need |
| `later` | it needs a toolset of a higher level — a crafted message at level 3, a statistical test or the state of a node at level 4 — or its category is another suite |
| `no check` | the model does not claim the behavior, so [the third principle](../../../guide/derive-tests-from-a-standard.md#principle-a-claimed-feature-gets-a-test) asks for no test |

A verdict belongs to the test. Where a test fails at a later observation and the observation
of a statement held, the row says PASS and names the observation in the note; where a failure
kept a test from the observation a statement needs, the row says FAIL and says so.

| Status | RFC 4861 | RFC 4862 | RFC 5942 | RFC 6980 | Together |
| --- | --- | --- | --- | --- | --- |
| `selected` | 186 | 25 | 6 | 1 | 218 |
| `covered` | 34 | 6 | 0 | 0 | 40 |
| `owed` | 65 | 22 | 1 | 0 | 88 |
| `later` | 111 | 15 | 3 | 5 | 134 |
| `no check` | 7 | 2 | 0 | 0 | 9 |
| all | 403 | 70 | 10 | 6 | 489 |

### RFC 4861

| Statement | Status | Check | Test | Verdict | Note |
| --- | --- | --- | --- | --- | --- |
| [RFC4861-RS-1](../../standard/rfc4861/catalog.md#rfc4861-rs-1) | selected | [Router Advertisement in answer to a solicitation](../../protocol/nd/checks/router-discovery.md#router-advertisement-in-answer-to-a-solicitation) | `Rfc4861RaSolicited` | FAIL | fails: [gap 5](results.md#gap-5-defect--the-router-answers-only-its-first-solicitation) |
| [RFC4861-RS-2](../../standard/rfc4861/catalog.md#rfc4861-rs-2) | covered | [Router Solicitation of a host that comes up](../../protocol/nd/checks/router-discovery.md#router-solicitation-of-a-host-that-comes-up) | `Rfc4861RsHostComesUp` | PASS | — |
| [RFC4861-RS-3](../../standard/rfc4861/catalog.md#rfc4861-rs-3) | selected | [Router Solicitation of a host that comes up](../../protocol/nd/checks/router-discovery.md#router-solicitation-of-a-host-that-comes-up) | `Rfc4861RsHostComesUp` | PASS | — |
| [RFC4861-RS-4](../../standard/rfc4861/catalog.md#rfc4861-rs-4) | selected | [Router Solicitation of a host that comes up](../../protocol/nd/checks/router-discovery.md#router-solicitation-of-a-host-that-comes-up) | `Rfc4861RsHostComesUp` | PASS | — |
| [RFC4861-RS-5](../../standard/rfc4861/catalog.md#rfc4861-rs-5) | selected | [Router Solicitation of a host that comes up](../../protocol/nd/checks/router-discovery.md#router-solicitation-of-a-host-that-comes-up) | `Rfc4861RsHostComesUp` | PASS | — |
| [RFC4861-RS-6](../../standard/rfc4861/catalog.md#rfc4861-rs-6) | selected | [Router Solicitation of a host that comes up](../../protocol/nd/checks/router-discovery.md#router-solicitation-of-a-host-that-comes-up) | `Rfc4861RsHostComesUp` | PASS | — |
| [RFC4861-RS-7](../../standard/rfc4861/catalog.md#rfc4861-rs-7) | selected | [Router Solicitation of a host that comes up](../../protocol/nd/checks/router-discovery.md#router-solicitation-of-a-host-that-comes-up) | `Rfc4861RsHostComesUp` | PASS | — |
| [RFC4861-RS-8](../../standard/rfc4861/catalog.md#rfc4861-rs-8) | later | — | — | — | another suite: the ICMPv6 checksum of RFC 4443, the same for every message type |
| [RFC4861-RS-9](../../standard/rfc4861/catalog.md#rfc4861-rs-9) | selected | [Router Solicitation of a host that comes up](../../protocol/nd/checks/router-discovery.md#router-solicitation-of-a-host-that-comes-up) | `Rfc4861RsHostComesUp` | PASS | — |
| [RFC4861-RS-10](../../standard/rfc4861/catalog.md#rfc4861-rs-10) | later | — | — | — | level 3: needs a crafted message |
| [RFC4861-RS-11](../../standard/rfc4861/catalog.md#rfc4861-rs-11) | selected | [Router Solicitation of a host that comes up](../../protocol/nd/checks/router-discovery.md#router-solicitation-of-a-host-that-comes-up) | `Rfc4861RsHostComesUp` | PASS | — |
| [RFC4861-RS-12](../../standard/rfc4861/catalog.md#rfc4861-rs-12) | selected | [Router Solicitation of a host that comes up](../../protocol/nd/checks/router-discovery.md#router-solicitation-of-a-host-that-comes-up) | `Rfc4861RsHostComesUp` | PASS | — |
| [RFC4861-RS-13](../../standard/rfc4861/catalog.md#rfc4861-rs-13) | later | — | — | — | level 3: needs a crafted message |
| [RFC4861-RA-1](../../standard/rfc4861/catalog.md#rfc4861-ra-1) | selected | [Router Advertisement header and addressing](../../protocol/nd/checks/router-discovery.md#router-advertisement-header-and-addressing) | `Rfc4861RaHeader` | PASS | — |
| [RFC4861-RA-2](../../standard/rfc4861/catalog.md#rfc4861-ra-2) | covered | [Router Advertisement fields](../../protocol/nd/checks/router-discovery.md#router-advertisement-fields) | `Rfc4861RaFields` | PASS | — |
| [RFC4861-RA-3](../../standard/rfc4861/catalog.md#rfc4861-ra-3) | selected | [Router Advertisement header and addressing](../../protocol/nd/checks/router-discovery.md#router-advertisement-header-and-addressing) | `Rfc4861RaHeader` | PASS | — |
| [RFC4861-RA-4](../../standard/rfc4861/catalog.md#rfc4861-ra-4) | selected | [Router Advertisement header and addressing](../../protocol/nd/checks/router-discovery.md#router-advertisement-header-and-addressing) | `Rfc4861RaHeader` | PASS | — |
| [RFC4861-RA-5](../../standard/rfc4861/catalog.md#rfc4861-ra-5) | selected | [Router Advertisement header and addressing](../../protocol/nd/checks/router-discovery.md#router-advertisement-header-and-addressing) | `Rfc4861RaHeader` | PASS | — |
| [RFC4861-RA-6](../../standard/rfc4861/catalog.md#rfc4861-ra-6) | selected | [Router Advertisement header and addressing](../../protocol/nd/checks/router-discovery.md#router-advertisement-header-and-addressing) | `Rfc4861RaHeader` | PASS | — |
| [RFC4861-RA-7](../../standard/rfc4861/catalog.md#rfc4861-ra-7) | selected | [Router Advertisement header and addressing](../../protocol/nd/checks/router-discovery.md#router-advertisement-header-and-addressing) | `Rfc4861RaHeader` | PASS | — |
| [RFC4861-RA-8](../../standard/rfc4861/catalog.md#rfc4861-ra-8) | later | — | — | — | another suite: the ICMPv6 checksum of RFC 4443, the same for every message type |
| [RFC4861-RA-9](../../standard/rfc4861/catalog.md#rfc4861-ra-9) | selected | [Hop limit from the router](../../protocol/nd/checks/parameters.md#hop-limit-from-the-router) | `Rfc4861HopLimitFromRouter` | FAIL | fails: [gap 1](results.md#gap-1-defect--the-hop-limit-of-a-host-is-a-constant) |
| [RFC4861-RA-10](../../standard/rfc4861/catalog.md#rfc4861-ra-10) | owed | — | — | — | a later level 2 pass: a router variable of zero, which leaves the host value as it is, and a lifetime of all one bits, which is infinity |
| [RFC4861-RA-11](../../standard/rfc4861/catalog.md#rfc4861-ra-11) | no check | — | — | — | the model has no DHCPv6 client; the flags themselves are checked in Router Advertisement fields |
| [RFC4861-RA-12](../../standard/rfc4861/catalog.md#rfc4861-ra-12) | no check | — | — | — | the model has no DHCPv6 client; the flags themselves are checked in Router Advertisement fields |
| [RFC4861-RA-13](../../standard/rfc4861/catalog.md#rfc4861-ra-13) | selected | [Router Advertisement header and addressing](../../protocol/nd/checks/router-discovery.md#router-advertisement-header-and-addressing) | `Rfc4861RaHeader` | PASS | — |
| [RFC4861-RA-14](../../standard/rfc4861/catalog.md#rfc4861-ra-14) | later | — | — | — | level 3: needs a crafted message |
| [RFC4861-RA-15](../../standard/rfc4861/catalog.md#rfc4861-ra-15) | selected | [The default router](../../protocol/nd/checks/router-discovery.md#the-default-router) | `Rfc4861DefaultRouter` | PASS | — |
| [RFC4861-RA-16](../../standard/rfc4861/catalog.md#rfc4861-ra-16) | later | — | — | — | level 3: needs a crafted message |
| [RFC4861-RA-17](../../standard/rfc4861/catalog.md#rfc4861-ra-17) | selected | [A router with Router Lifetime zero](../../protocol/nd/checks/router-discovery.md#a-router-with-router-lifetime-zero) | `Rfc4861RouterLifetimeZero` | FAIL | not reached: [gap 7](results.md#gap-7-defect--a-host-drops-every-advertisement-during-duplicate-address-detection) keeps the test from the observation |
| [RFC4861-RA-18](../../standard/rfc4861/catalog.md#rfc4861-ra-18) | owed | — | — | — | a later level 2 pass: a router with Router Lifetime zero that alone advertises a prefix |
| [RFC4861-RA-19](../../standard/rfc4861/catalog.md#rfc4861-ra-19) | selected | [Router Advertisement fields](../../protocol/nd/checks/router-discovery.md#router-advertisement-fields) | `Rfc4861RaFields` | PASS | — |
| [RFC4861-RA-20](../../standard/rfc4861/catalog.md#rfc4861-ra-20) | owed | — | — | — | a later level 2 pass: a router variable of zero, which leaves the host value as it is, and a lifetime of all one bits, which is infinity |
| [RFC4861-RA-21](../../standard/rfc4861/catalog.md#rfc4861-ra-21) | selected | [Router Advertisement fields](../../protocol/nd/checks/router-discovery.md#router-advertisement-fields) | `Rfc4861RaFields` | PASS | — |
| [RFC4861-RA-22](../../standard/rfc4861/catalog.md#rfc4861-ra-22) | owed | — | — | — | a later level 2 pass: a router variable of zero, which leaves the host value as it is, and a lifetime of all one bits, which is infinity |
| [RFC4861-RA-23](../../standard/rfc4861/catalog.md#rfc4861-ra-23) | selected | [Router Advertisement fields](../../protocol/nd/checks/router-discovery.md#router-advertisement-fields) | `Rfc4861RaFields` | PASS | — |
| [RFC4861-RA-24](../../standard/rfc4861/catalog.md#rfc4861-ra-24) | covered | [Router Advertisement fields](../../protocol/nd/checks/router-discovery.md#router-advertisement-fields) | `Rfc4861RaFields` | PASS | a permission that the check accepts: observation 2 judges the option only when it is present |
| [RFC4861-RA-25](../../standard/rfc4861/catalog.md#rfc4861-ra-25) | later | — | — | — | a link with a variable MTU or without multicast; no mockup of this level has one |
| [RFC4861-RA-26](../../standard/rfc4861/catalog.md#rfc4861-ra-26) | covered | [Router Advertisement fields](../../protocol/nd/checks/router-discovery.md#router-advertisement-fields) | `Rfc4861RaMtuOption` | PASS | its observation held before the failure |
| [RFC4861-RA-27](../../standard/rfc4861/catalog.md#rfc4861-ra-27) | selected | [Prefix Information option](../../protocol/nd/checks/router-discovery.md#prefix-information-option) | `Rfc4861RaPrefixInformation` | PASS | — |
| [RFC4861-RA-28](../../standard/rfc4861/catalog.md#rfc4861-ra-28) | later | — | — | — | level 3: needs a crafted message |
| [RFC4861-NS-1](../../standard/rfc4861/catalog.md#rfc4861-ns-1) | selected | [Address resolution of a host](../../protocol/nd/checks/address-resolution.md#address-resolution-of-a-host) | `Rfc4861AddressResolutionHost` | PASS | — |
| [RFC4861-NS-2](../../standard/rfc4861/catalog.md#rfc4861-ns-2) | selected | [Address resolution of a host](../../protocol/nd/checks/address-resolution.md#address-resolution-of-a-host) | `Rfc4861AddressResolutionHost` | PASS | — |
| [RFC4861-NS-3](../../standard/rfc4861/catalog.md#rfc4861-ns-3) | covered | [Neighbor Solicitation and Advertisement fields](../../protocol/nd/checks/address-resolution.md#neighbor-solicitation-and-advertisement-fields) | `Rfc4861NsNaFields` | PASS | — |
| [RFC4861-NS-4](../../standard/rfc4861/catalog.md#rfc4861-ns-4) | selected | [Neighbor Solicitation and Advertisement fields](../../protocol/nd/checks/address-resolution.md#neighbor-solicitation-and-advertisement-fields) | `Rfc4861NsNaFields` | PASS | — |
| [RFC4861-NS-5](../../standard/rfc4861/catalog.md#rfc4861-ns-5) | selected | [Address resolution of a host](../../protocol/nd/checks/address-resolution.md#address-resolution-of-a-host) | `Rfc4861AddressResolutionHost` | PASS | — |
| [RFC4861-NS-6](../../standard/rfc4861/catalog.md#rfc4861-ns-6) | selected | [Neighbor Solicitation and Advertisement fields](../../protocol/nd/checks/address-resolution.md#neighbor-solicitation-and-advertisement-fields) | `Rfc4861NsNaFields` | PASS | — |
| [RFC4861-NS-7](../../standard/rfc4861/catalog.md#rfc4861-ns-7) | selected | [Neighbor Solicitation and Advertisement fields](../../protocol/nd/checks/address-resolution.md#neighbor-solicitation-and-advertisement-fields) | `Rfc4861NsNaFields` | PASS | — |
| [RFC4861-NS-8](../../standard/rfc4861/catalog.md#rfc4861-ns-8) | selected | [Neighbor Solicitation and Advertisement fields](../../protocol/nd/checks/address-resolution.md#neighbor-solicitation-and-advertisement-fields) | `Rfc4861NsNaFields` | PASS | — |
| [RFC4861-NS-9](../../standard/rfc4861/catalog.md#rfc4861-ns-9) | later | — | — | — | another suite: the ICMPv6 checksum of RFC 4443, the same for every message type |
| [RFC4861-NS-10](../../standard/rfc4861/catalog.md#rfc4861-ns-10) | selected | [Neighbor Solicitation and Advertisement fields](../../protocol/nd/checks/address-resolution.md#neighbor-solicitation-and-advertisement-fields) | `Rfc4861NsNaFields` | PASS | — |
| [RFC4861-NS-11](../../standard/rfc4861/catalog.md#rfc4861-ns-11) | later | — | — | — | level 3: needs a crafted message |
| [RFC4861-NS-12](../../standard/rfc4861/catalog.md#rfc4861-ns-12) | selected | [Address resolution of a host](../../protocol/nd/checks/address-resolution.md#address-resolution-of-a-host) | `Rfc4861AddressResolutionHost` | PASS | — |
| [RFC4861-NS-13](../../standard/rfc4861/catalog.md#rfc4861-ns-13) | selected | [Neighbor Solicitation and Advertisement fields](../../protocol/nd/checks/address-resolution.md#neighbor-solicitation-and-advertisement-fields) | `Rfc4861NsNaFields` | PASS | — |
| [RFC4861-NS-14](../../standard/rfc4861/catalog.md#rfc4861-ns-14) | selected | [Link-local address and its Duplicate Address Detection](../../protocol/nd/checks/autoconfiguration.md#link-local-address-and-its-duplicate-address-detection) | `Rfc4862LinkLocalDad` | PASS | — |
| [RFC4861-NS-15](../../standard/rfc4861/catalog.md#rfc4861-ns-15) | selected | [Address resolution of a host](../../protocol/nd/checks/address-resolution.md#address-resolution-of-a-host) | `Rfc4861AddressResolutionHost` | PASS | — |
| [RFC4861-NS-16](../../standard/rfc4861/catalog.md#rfc4861-ns-16) | later | — | — | — | level 4: Neighbor Unreachability Detection, RFC 4861 §7.3, sends the unicast solicitations and uses these flags |
| [RFC4861-NS-17](../../standard/rfc4861/catalog.md#rfc4861-ns-17) | later | — | — | — | level 3: needs a crafted message |
| [RFC4861-NA-1](../../standard/rfc4861/catalog.md#rfc4861-na-1) | selected | [Address resolution of a host](../../protocol/nd/checks/address-resolution.md#address-resolution-of-a-host) | `Rfc4861AddressResolutionHost` | PASS | — |
| [RFC4861-NA-2](../../standard/rfc4861/catalog.md#rfc4861-na-2) | covered | [Neighbor Solicitation and Advertisement fields](../../protocol/nd/checks/address-resolution.md#neighbor-solicitation-and-advertisement-fields) | `Rfc4861NsNaFields` | PASS | — |
| [RFC4861-NA-3](../../standard/rfc4861/catalog.md#rfc4861-na-3) | selected | [Neighbor Solicitation and Advertisement fields](../../protocol/nd/checks/address-resolution.md#neighbor-solicitation-and-advertisement-fields) | `Rfc4861NsNaFields` | PASS | — |
| [RFC4861-NA-4](../../standard/rfc4861/catalog.md#rfc4861-na-4) | selected | [Address resolution of a host](../../protocol/nd/checks/address-resolution.md#address-resolution-of-a-host) | `Rfc4861AddressResolutionHost` | PASS | — |
| [RFC4861-NA-5](../../standard/rfc4861/catalog.md#rfc4861-na-5) | owed | — | — | — | a later level 2 pass: a change of the link-layer address of a node, for unsolicited advertisements |
| [RFC4861-NA-6](../../standard/rfc4861/catalog.md#rfc4861-na-6) | selected | [Neighbor Solicitation and Advertisement fields](../../protocol/nd/checks/address-resolution.md#neighbor-solicitation-and-advertisement-fields) | `Rfc4861NsNaFields` | PASS | — |
| [RFC4861-NA-7](../../standard/rfc4861/catalog.md#rfc4861-na-7) | selected | [Neighbor Solicitation and Advertisement fields](../../protocol/nd/checks/address-resolution.md#neighbor-solicitation-and-advertisement-fields) | `Rfc4861NsNaFields` | PASS | — |
| [RFC4861-NA-8](../../standard/rfc4861/catalog.md#rfc4861-na-8) | selected | [Neighbor Solicitation and Advertisement fields](../../protocol/nd/checks/address-resolution.md#neighbor-solicitation-and-advertisement-fields) | `Rfc4861NsNaFields` | PASS | — |
| [RFC4861-NA-9](../../standard/rfc4861/catalog.md#rfc4861-na-9) | later | — | — | — | another suite: the ICMPv6 checksum of RFC 4443, the same for every message type |
| [RFC4861-NA-10](../../standard/rfc4861/catalog.md#rfc4861-na-10) | selected | [Address resolution of a router](../../protocol/nd/checks/address-resolution.md#address-resolution-of-a-router) | `Rfc4861AddressResolutionRouter` | PASS | — |
| [RFC4861-NA-11](../../standard/rfc4861/catalog.md#rfc4861-na-11) | later | — | — | — | level 4: Neighbor Unreachability Detection, RFC 4861 §7.3, sends the unicast solicitations and uses these flags |
| [RFC4861-NA-12](../../standard/rfc4861/catalog.md#rfc4861-na-12) | selected | [Neighbor Solicitation and Advertisement fields](../../protocol/nd/checks/address-resolution.md#neighbor-solicitation-and-advertisement-fields) | `Rfc4861NsNaFields` | PASS | — |
| [RFC4861-NA-13](../../standard/rfc4861/catalog.md#rfc4861-na-13) | later | — | — | — | level 4: Neighbor Unreachability Detection, RFC 4861 §7.3, sends the unicast solicitations and uses these flags |
| [RFC4861-NA-14](../../standard/rfc4861/catalog.md#rfc4861-na-14) | selected | [Duplicate link-local address](../../protocol/nd/checks/autoconfiguration.md#duplicate-link-local-address) | `Rfc4862DuplicateLinkLocal` | PASS | — |
| [RFC4861-NA-15](../../standard/rfc4861/catalog.md#rfc4861-na-15) | later | — | — | — | level 3: needs a crafted message |
| [RFC4861-NA-16](../../standard/rfc4861/catalog.md#rfc4861-na-16) | later | — | — | — | level 3: needs a crafted message |
| [RFC4861-NA-17](../../standard/rfc4861/catalog.md#rfc4861-na-17) | owed | — | — | — | a later level 2 pass: a proxy |
| [RFC4861-NA-18](../../standard/rfc4861/catalog.md#rfc4861-na-18) | selected | [Neighbor Solicitation and Advertisement fields](../../protocol/nd/checks/address-resolution.md#neighbor-solicitation-and-advertisement-fields) | `Rfc4861NsNaFields` | PASS | — |
| [RFC4861-NA-19](../../standard/rfc4861/catalog.md#rfc4861-na-19) | selected | [Neighbor Solicitation and Advertisement fields](../../protocol/nd/checks/address-resolution.md#neighbor-solicitation-and-advertisement-fields) | `Rfc4861NsNaFields` | PASS | — |
| [RFC4861-NA-20](../../standard/rfc4861/catalog.md#rfc4861-na-20) | later | — | — | — | level 3: needs a crafted message |
| [RFC4861-NA-21](../../standard/rfc4861/catalog.md#rfc4861-na-21) | selected | [Address resolution of a host](../../protocol/nd/checks/address-resolution.md#address-resolution-of-a-host) | `Rfc4861AddressResolutionHost` | PASS | — |
| [RFC4861-NA-22](../../standard/rfc4861/catalog.md#rfc4861-na-22) | owed | — | — | — | a later level 2 pass: a change of the link-layer address of a node, for unsolicited advertisements |
| [RFC4861-NA-23](../../standard/rfc4861/catalog.md#rfc4861-na-23) | selected | [Neighbor Solicitation and Advertisement fields](../../protocol/nd/checks/address-resolution.md#neighbor-solicitation-and-advertisement-fields) | `Rfc4861NsNaFields` | PASS | — |
| [RFC4861-NA-24](../../standard/rfc4861/catalog.md#rfc4861-na-24) | selected | [Address resolution of a host](../../protocol/nd/checks/address-resolution.md#address-resolution-of-a-host) | `Rfc4861AddressResolutionHost` | PASS | — |
| [RFC4861-NA-25](../../standard/rfc4861/catalog.md#rfc4861-na-25) | later | — | — | — | level 4: Neighbor Unreachability Detection, RFC 4861 §7.3, sends the unicast solicitations and uses these flags |
| [RFC4861-NA-26](../../standard/rfc4861/catalog.md#rfc4861-na-26) | later | — | — | — | level 3: needs a crafted message |
| [RFC4861-RDM-1](../../standard/rfc4861/catalog.md#rfc4861-rdm-1) | selected | [Redirect from the first-hop router](../../protocol/nd/checks/redirect.md#redirect-from-the-first-hop-router) | `Rfc4861RedirectFirstHop` | PASS | — |
| [RFC4861-RDM-2](../../standard/rfc4861/catalog.md#rfc4861-rdm-2) | covered | [Redirect fields](../../protocol/nd/checks/redirect.md#redirect-fields) | `Rfc4861RedirectFields` | PASS | its observation held before the failure |
| [RFC4861-RDM-3](../../standard/rfc4861/catalog.md#rfc4861-rdm-3) | selected | [Redirect from the first-hop router](../../protocol/nd/checks/redirect.md#redirect-from-the-first-hop-router) | `Rfc4861RedirectFirstHop` | PASS | — |
| [RFC4861-RDM-4](../../standard/rfc4861/catalog.md#rfc4861-rdm-4) | selected | [Redirect from the first-hop router](../../protocol/nd/checks/redirect.md#redirect-from-the-first-hop-router) | `Rfc4861RedirectFirstHop` | PASS | — |
| [RFC4861-RDM-5](../../standard/rfc4861/catalog.md#rfc4861-rdm-5) | selected | [Redirect fields](../../protocol/nd/checks/redirect.md#redirect-fields) | `Rfc4861RedirectFields` | PASS | observation 4 held |
| [RFC4861-RDM-6](../../standard/rfc4861/catalog.md#rfc4861-rdm-6) | selected | [Redirect fields](../../protocol/nd/checks/redirect.md#redirect-fields) | `Rfc4861RedirectFields` | PASS | observation 4 held |
| [RFC4861-RDM-7](../../standard/rfc4861/catalog.md#rfc4861-rdm-7) | selected | [Redirect fields](../../protocol/nd/checks/redirect.md#redirect-fields) | `Rfc4861RedirectFields` | PASS | observation 4 held |
| [RFC4861-RDM-8](../../standard/rfc4861/catalog.md#rfc4861-rdm-8) | later | — | — | — | another suite: the ICMPv6 checksum of RFC 4443, the same for every message type |
| [RFC4861-RDM-9](../../standard/rfc4861/catalog.md#rfc4861-rdm-9) | selected | [Redirect fields](../../protocol/nd/checks/redirect.md#redirect-fields) | `Rfc4861RedirectFields` | PASS | observation 4 held |
| [RFC4861-RDM-10](../../standard/rfc4861/catalog.md#rfc4861-rdm-10) | later | — | — | — | level 3: needs a crafted message |
| [RFC4861-RDM-11](../../standard/rfc4861/catalog.md#rfc4861-rdm-11) | selected | [Redirect from the first-hop router](../../protocol/nd/checks/redirect.md#redirect-from-the-first-hop-router) | `Rfc4861RedirectFirstHop` | PASS | — |
| [RFC4861-RDM-12](../../standard/rfc4861/catalog.md#rfc4861-rdm-12) | selected | [Redirect to an on-link destination](../../protocol/nd/checks/redirect.md#redirect-to-an-on-link-destination) | `Rfc4861RedirectOnLink` | PASS | — |
| [RFC4861-RDM-13](../../standard/rfc4861/catalog.md#rfc4861-rdm-13) | selected | [Redirect from the first-hop router](../../protocol/nd/checks/redirect.md#redirect-from-the-first-hop-router) | `Rfc4861RedirectFirstHop` | PASS | — |
| [RFC4861-RDM-14](../../standard/rfc4861/catalog.md#rfc4861-rdm-14) | selected | [Redirect from the first-hop router](../../protocol/nd/checks/redirect.md#redirect-from-the-first-hop-router) | `Rfc4861RedirectFirstHop` | PASS | — |
| [RFC4861-RDM-15](../../standard/rfc4861/catalog.md#rfc4861-rdm-15) | selected | [Redirect fields](../../protocol/nd/checks/redirect.md#redirect-fields) | `Rfc4861RedirectFields` | FAIL | fails: [gap 8](results.md#gap-8-defect--the-redirect-carries-no-option) |
| [RFC4861-RDM-16](../../standard/rfc4861/catalog.md#rfc4861-rdm-16) | later | — | — | — | a link with a variable MTU or without multicast; no mockup of this level has one |
| [RFC4861-RDM-17](../../standard/rfc4861/catalog.md#rfc4861-rdm-17) | selected | [Redirect of a large packet](../../protocol/nd/checks/redirect.md#redirect-of-a-large-packet) | `Rfc6980RedirectLargePacket` | FAIL | fails: [gap 8](results.md#gap-8-defect--the-redirect-carries-no-option) |
| [RFC4861-OPT-1](../../standard/rfc4861/catalog.md#rfc4861-opt-1) | covered | [Router Advertisement fields](../../protocol/nd/checks/router-discovery.md#router-advertisement-fields) | `Rfc4861RaFields` | PASS | — |
| [RFC4861-OPT-2](../../standard/rfc4861/catalog.md#rfc4861-opt-2) | covered | [Router Advertisement fields](../../protocol/nd/checks/router-discovery.md#router-advertisement-fields) | `Rfc4861RaFields` | PASS | — |
| [RFC4861-OPT-3](../../standard/rfc4861/catalog.md#rfc4861-opt-3) | covered | [Router Advertisement fields](../../protocol/nd/checks/router-discovery.md#router-advertisement-fields) | `Rfc4861RaFields` | PASS | — |
| [RFC4861-OPT-4](../../standard/rfc4861/catalog.md#rfc4861-opt-4) | covered | [Prefix Information option](../../protocol/nd/checks/router-discovery.md#prefix-information-option) | `Rfc4861RaPrefixInformation` | PASS | — |
| [RFC4861-OPT-5](../../standard/rfc4861/catalog.md#rfc4861-opt-5) | covered | [Router Advertisement fields](../../protocol/nd/checks/router-discovery.md#router-advertisement-fields) | `Rfc4861RaFields` | PASS | — |
| [RFC4861-OPT-6](../../standard/rfc4861/catalog.md#rfc4861-opt-6) | later | — | — | — | level 3: needs a crafted message |
| [RFC4861-OPT-7](../../standard/rfc4861/catalog.md#rfc4861-opt-7) | covered | [Router Advertisement fields](../../protocol/nd/checks/router-discovery.md#router-advertisement-fields) | `Rfc4861RaFields` | PASS | — |
| [RFC4861-OPT-8](../../standard/rfc4861/catalog.md#rfc4861-opt-8) | selected | [Router Advertisement fields](../../protocol/nd/checks/router-discovery.md#router-advertisement-fields) | `Rfc4861RaFields` | PASS | — |
| [RFC4861-OPT-9](../../standard/rfc4861/catalog.md#rfc4861-opt-9) | selected | [Router Advertisement fields](../../protocol/nd/checks/router-discovery.md#router-advertisement-fields) | `Rfc4861RaFields` | PASS | — |
| [RFC4861-OPT-10](../../standard/rfc4861/catalog.md#rfc4861-opt-10) | covered | [Router Advertisement fields](../../protocol/nd/checks/router-discovery.md#router-advertisement-fields) | `Rfc4861RaFields` | PASS | — |
| [RFC4861-OPT-11](../../standard/rfc4861/catalog.md#rfc4861-opt-11) | covered | [Router Solicitation of a host that comes up](../../protocol/nd/checks/router-discovery.md#router-solicitation-of-a-host-that-comes-up) | `Rfc4861RsHostComesUp` | PASS | — |
| [RFC4861-OPT-12](../../standard/rfc4861/catalog.md#rfc4861-opt-12) | selected | [Redirect fields](../../protocol/nd/checks/redirect.md#redirect-fields) | `Rfc4861RedirectFields` | FAIL | fails: [gap 8](results.md#gap-8-defect--the-redirect-carries-no-option) |
| [RFC4861-OPT-13](../../standard/rfc4861/catalog.md#rfc4861-opt-13) | later | — | — | — | level 3: needs a crafted message |
| [RFC4861-OPT-14](../../standard/rfc4861/catalog.md#rfc4861-opt-14) | covered | [Prefix Information option](../../protocol/nd/checks/router-discovery.md#prefix-information-option) | `Rfc4861RaPrefixInformation` | PASS | — |
| [RFC4861-OPT-15](../../standard/rfc4861/catalog.md#rfc4861-opt-15) | selected | [Prefix Information option](../../protocol/nd/checks/router-discovery.md#prefix-information-option) | `Rfc4861RaPrefixInformation` | PASS | — |
| [RFC4861-OPT-16](../../standard/rfc4861/catalog.md#rfc4861-opt-16) | selected | [Prefix Information option](../../protocol/nd/checks/router-discovery.md#prefix-information-option) | `Rfc4861RaPrefixInformation` | PASS | — |
| [RFC4861-OPT-17](../../standard/rfc4861/catalog.md#rfc4861-opt-17) | covered | [Prefix Information option](../../protocol/nd/checks/router-discovery.md#prefix-information-option) | `Rfc4861RaPrefixInformation` | PASS | — |
| [RFC4861-OPT-18](../../standard/rfc4861/catalog.md#rfc4861-opt-18) | selected | [On-link neighbor reached directly](../../protocol/nd/checks/on-link.md#on-link-neighbor-reached-directly) | `Rfc4861OnLinkNeighbor` | PASS | — |
| [RFC4861-OPT-19](../../standard/rfc4861/catalog.md#rfc4861-opt-19) | selected | [On-link neighbor reached directly](../../protocol/nd/checks/on-link.md#on-link-neighbor-reached-directly) | `Rfc4861OnLinkNeighbor` | PASS | — |
| [RFC4861-OPT-20](../../standard/rfc4861/catalog.md#rfc4861-opt-20) | owed | — | — | — | a later level 2 pass: a router that changes the L flag of a prefix, advertises a prefix with Valid Lifetime zero, or refreshes it; a prefix length other than 64; a router whose lifetime runs out |
| [RFC4861-OPT-21](../../standard/rfc4861/catalog.md#rfc4861-opt-21) | selected | [Global address and its Duplicate Address Detection](../../protocol/nd/checks/autoconfiguration.md#global-address-and-its-duplicate-address-detection) | `Rfc4862GlobalAddressDad` | PASS | — |
| [RFC4861-OPT-22](../../standard/rfc4861/catalog.md#rfc4861-opt-22) | selected | [Prefix Information option](../../protocol/nd/checks/router-discovery.md#prefix-information-option) | `Rfc4861RaPrefixInformation` | PASS | — |
| [RFC4861-OPT-23](../../standard/rfc4861/catalog.md#rfc4861-opt-23) | later | — | — | — | level 3: needs a crafted message |
| [RFC4861-OPT-24](../../standard/rfc4861/catalog.md#rfc4861-opt-24) | selected | [Prefix after its valid lifetime](../../protocol/nd/checks/on-link.md#prefix-after-its-valid-lifetime) | `Rfc5942PrefixLifetime` | PASS | — |
| [RFC4861-OPT-25](../../standard/rfc4861/catalog.md#rfc4861-opt-25) | owed | — | — | — | a later level 2 pass: a router variable of zero, which leaves the host value as it is, and a lifetime of all one bits, which is infinity |
| [RFC4861-OPT-26](../../standard/rfc4861/catalog.md#rfc4861-opt-26) | owed | — | — | — | a later level 2 pass: a deprecated address with an alternative, and the renewal of a known prefix under the two-hour rule |
| [RFC4861-OPT-27](../../standard/rfc4861/catalog.md#rfc4861-opt-27) | owed | — | — | — | a later level 2 pass: a router variable of zero, which leaves the host value as it is, and a lifetime of all one bits, which is infinity |
| [RFC4861-OPT-28](../../standard/rfc4861/catalog.md#rfc4861-opt-28) | selected | [Prefix Information option](../../protocol/nd/checks/router-discovery.md#prefix-information-option) | `Rfc4861RaPrefixInformation` | PASS | — |
| [RFC4861-OPT-29](../../standard/rfc4861/catalog.md#rfc4861-opt-29) | selected | [Prefix Information option](../../protocol/nd/checks/router-discovery.md#prefix-information-option) | `Rfc4861RaPrefixInformation` | PASS | — |
| [RFC4861-OPT-30](../../standard/rfc4861/catalog.md#rfc4861-opt-30) | later | — | — | — | level 3: needs a crafted message |
| [RFC4861-OPT-31](../../standard/rfc4861/catalog.md#rfc4861-opt-31) | covered | [Prefix Information option](../../protocol/nd/checks/router-discovery.md#prefix-information-option) | `Rfc4861RaPrefixInformation` | PASS | — |
| [RFC4861-OPT-32](../../standard/rfc4861/catalog.md#rfc4861-opt-32) | selected | [Prefix Information option](../../protocol/nd/checks/router-discovery.md#prefix-information-option) | `Rfc4861RaPrefixInformation` | PASS | — |
| [RFC4861-OPT-33](../../standard/rfc4861/catalog.md#rfc4861-opt-33) | later | — | — | — | level 3: needs a crafted message |
| [RFC4861-OPT-34](../../standard/rfc4861/catalog.md#rfc4861-opt-34) | selected | [Prefix Information option](../../protocol/nd/checks/router-discovery.md#prefix-information-option) | `Rfc4861RaPrefixInformation` | PASS | — |
| [RFC4861-OPT-35](../../standard/rfc4861/catalog.md#rfc4861-opt-35) | later | — | — | — | level 3: needs a crafted message |
| [RFC4861-OPT-36](../../standard/rfc4861/catalog.md#rfc4861-opt-36) | later | — | — | — | level 3: needs a crafted message |
| [RFC4861-OPT-37](../../standard/rfc4861/catalog.md#rfc4861-opt-37) | covered | [Redirect fields](../../protocol/nd/checks/redirect.md#redirect-fields) | `Rfc4861RedirectedHeader` | FAIL | not reached: [gap 8](results.md#gap-8-defect--the-redirect-carries-no-option) keeps the test from the observation |
| [RFC4861-OPT-38](../../standard/rfc4861/catalog.md#rfc4861-opt-38) | selected | [Redirect fields](../../protocol/nd/checks/redirect.md#redirect-fields) | `Rfc4861RedirectedHeader` | FAIL | fails: [gap 8](results.md#gap-8-defect--the-redirect-carries-no-option) |
| [RFC4861-OPT-39](../../standard/rfc4861/catalog.md#rfc4861-opt-39) | selected | [Redirect fields](../../protocol/nd/checks/redirect.md#redirect-fields) | `Rfc4861RedirectedHeader` | FAIL | fails: [gap 8](results.md#gap-8-defect--the-redirect-carries-no-option) |
| [RFC4861-OPT-40](../../standard/rfc4861/catalog.md#rfc4861-opt-40) | selected | [Redirect fields](../../protocol/nd/checks/redirect.md#redirect-fields) | `Rfc4861RedirectedHeader` | FAIL | fails: [gap 8](results.md#gap-8-defect--the-redirect-carries-no-option) |
| [RFC4861-OPT-41](../../standard/rfc4861/catalog.md#rfc4861-opt-41) | later | — | — | — | level 3: needs a crafted message |
| [RFC4861-OPT-42](../../standard/rfc4861/catalog.md#rfc4861-opt-42) | selected | [Redirect of a large packet](../../protocol/nd/checks/redirect.md#redirect-of-a-large-packet) | `Rfc6980RedirectLargePacket` | PASS | observation 3 held |
| [RFC4861-OPT-43](../../standard/rfc4861/catalog.md#rfc4861-opt-43) | later | — | — | — | level 3: needs a crafted message |
| [RFC4861-OPT-44](../../standard/rfc4861/catalog.md#rfc4861-opt-44) | covered | [Router Advertisement fields](../../protocol/nd/checks/router-discovery.md#router-advertisement-fields) | `Rfc4861RaFields` | PASS | — |
| [RFC4861-OPT-45](../../standard/rfc4861/catalog.md#rfc4861-opt-45) | selected | [Router Advertisement fields](../../protocol/nd/checks/router-discovery.md#router-advertisement-fields) | `Rfc4861RaMtuOption` | PASS | the option that the router sends has these fields right |
| [RFC4861-OPT-46](../../standard/rfc4861/catalog.md#rfc4861-opt-46) | selected | [Router Advertisement fields](../../protocol/nd/checks/router-discovery.md#router-advertisement-fields) | `Rfc4861RaMtuOption` | PASS | the option that the router sends has these fields right |
| [RFC4861-OPT-47](../../standard/rfc4861/catalog.md#rfc4861-opt-47) | selected | [Router Advertisement fields](../../protocol/nd/checks/router-discovery.md#router-advertisement-fields) | `Rfc4861RaMtuOption` | PASS | the option that the router sends has these fields right |
| [RFC4861-OPT-48](../../standard/rfc4861/catalog.md#rfc4861-opt-48) | later | — | — | — | level 3: needs a crafted message |
| [RFC4861-OPT-49](../../standard/rfc4861/catalog.md#rfc4861-opt-49) | selected | [MTU from the router](../../protocol/nd/checks/parameters.md#mtu-from-the-router) | `Rfc4861MtuFromRouter` | FAIL | fails: [gap 3](results.md#gap-3-defect--the-host-fragments-to-the-mtu-of-the-interface-not-to-the-advertised-mtu) |
| [RFC4861-OPT-50](../../standard/rfc4861/catalog.md#rfc4861-opt-50) | later | — | — | — | level 3: needs a crafted message |
| [RFC4861-RVAL-1](../../standard/rfc4861/catalog.md#rfc4861-rval-1) | later | — | — | — | level 3: needs a crafted message |
| [RFC4861-RVAL-2](../../standard/rfc4861/catalog.md#rfc4861-rval-2) | later | — | — | — | level 3: needs a crafted message |
| [RFC4861-RVAL-3](../../standard/rfc4861/catalog.md#rfc4861-rval-3) | later | — | — | — | level 3: needs a crafted message |
| [RFC4861-RVAL-4](../../standard/rfc4861/catalog.md#rfc4861-rval-4) | later | — | — | — | level 3: needs a crafted message |
| [RFC4861-RVAL-5](../../standard/rfc4861/catalog.md#rfc4861-rval-5) | later | — | — | — | level 3: needs a crafted message |
| [RFC4861-RVAL-6](../../standard/rfc4861/catalog.md#rfc4861-rval-6) | later | — | — | — | level 3: needs a crafted message |
| [RFC4861-RVAL-7](../../standard/rfc4861/catalog.md#rfc4861-rval-7) | later | — | — | — | level 3: needs a crafted message |
| [RFC4861-RVAL-8](../../standard/rfc4861/catalog.md#rfc4861-rval-8) | later | — | — | — | level 3: needs a crafted message |
| [RFC4861-RVAL-9](../../standard/rfc4861/catalog.md#rfc4861-rval-9) | later | — | — | — | level 3: needs a crafted message |
| [RFC4861-RVAL-10](../../standard/rfc4861/catalog.md#rfc4861-rval-10) | later | — | — | — | level 3: needs a crafted message |
| [RFC4861-RVAL-11](../../standard/rfc4861/catalog.md#rfc4861-rval-11) | selected | [Router Advertisement header and addressing](../../protocol/nd/checks/router-discovery.md#router-advertisement-header-and-addressing) | `Rfc4861RaHeader` | PASS | — |
| [RFC4861-RVAL-12](../../standard/rfc4861/catalog.md#rfc4861-rval-12) | later | — | — | — | level 3: needs a crafted message |
| [RFC4861-RVAL-13](../../standard/rfc4861/catalog.md#rfc4861-rval-13) | later | — | — | — | level 3: needs a crafted message |
| [RFC4861-RVAL-14](../../standard/rfc4861/catalog.md#rfc4861-rval-14) | later | — | — | — | level 3: needs a crafted message |
| [RFC4861-RVAL-15](../../standard/rfc4861/catalog.md#rfc4861-rval-15) | later | — | — | — | level 3: needs a crafted message |
| [RFC4861-RVAL-16](../../standard/rfc4861/catalog.md#rfc4861-rval-16) | later | — | — | — | level 3: needs a crafted message |
| [RFC4861-RVAL-17](../../standard/rfc4861/catalog.md#rfc4861-rval-17) | later | — | — | — | level 3: needs a crafted message |
| [RFC4861-RVAL-18](../../standard/rfc4861/catalog.md#rfc4861-rval-18) | later | — | — | — | level 3: needs a crafted message |
| [RFC4861-RCFG-1](../../standard/rfc4861/catalog.md#rfc4861-rcfg-1) | owed | — | — | — | a check of each router variable; the model has no parameter for AdvReachableTime and AdvRetransTimer, see [results.md](results.md#other-findings) |
| [RFC4861-RCFG-2](../../standard/rfc4861/catalog.md#rfc4861-rcfg-2) | later | — | — | — | not a behavior of the model: a right of the documents for other link layers |
| [RFC4861-RCFG-3](../../standard/rfc4861/catalog.md#rfc4861-rcfg-3) | later | — | — | — | level 4: a variable inside the node |
| [RFC4861-RCFG-4](../../standard/rfc4861/catalog.md#rfc4861-rcfg-4) | owed | — | — | — | a later level 2 pass: an interface that does not advertise |
| [RFC4861-RCFG-5](../../standard/rfc4861/catalog.md#rfc4861-rcfg-5) | selected | [Unsolicited Router Advertisements](../../protocol/nd/checks/router-discovery.md#unsolicited-router-advertisements) | `Rfc4861RaUnsolicited` | FAIL | not reached: [gap 6](results.md#gap-6-defect--the-first-periodic-advertisement-comes-after-198-to-600-seconds) keeps the test from the observation |
| [RFC4861-RCFG-6](../../standard/rfc4861/catalog.md#rfc4861-rcfg-6) | selected | [Unsolicited Router Advertisements](../../protocol/nd/checks/router-discovery.md#unsolicited-router-advertisements) | `Rfc4861RaUnsolicited` | FAIL | not reached: [gap 6](results.md#gap-6-defect--the-first-periodic-advertisement-comes-after-198-to-600-seconds) keeps the test from the observation |
| [RFC4861-RCFG-7](../../standard/rfc4861/catalog.md#rfc4861-rcfg-7) | selected | [Router Advertisement fields](../../protocol/nd/checks/router-discovery.md#router-advertisement-fields) | `Rfc4861RaFields` | PASS | — |
| [RFC4861-RCFG-8](../../standard/rfc4861/catalog.md#rfc4861-rcfg-8) | selected | [Router Advertisement fields](../../protocol/nd/checks/router-discovery.md#router-advertisement-fields) | `Rfc4861RaFields` | PASS | — |
| [RFC4861-RCFG-9](../../standard/rfc4861/catalog.md#rfc4861-rcfg-9) | covered | [Router Advertisement fields](../../protocol/nd/checks/router-discovery.md#router-advertisement-fields) | `Rfc4861RaMtuOption` | FAIL | fails: [gap 2](results.md#gap-2-defect--two-defaults-of-the-router-are-wrong) |
| [RFC4861-RCFG-10](../../standard/rfc4861/catalog.md#rfc4861-rcfg-10) | selected | [Router Advertisement fields](../../protocol/nd/checks/router-discovery.md#router-advertisement-fields) | `Rfc4861RaFields` | PASS | — |
| [RFC4861-RCFG-11](../../standard/rfc4861/catalog.md#rfc4861-rcfg-11) | selected | [Router Advertisement fields](../../protocol/nd/checks/router-discovery.md#router-advertisement-fields) | `Rfc4861RaFields` | PASS | — |
| [RFC4861-RCFG-12](../../standard/rfc4861/catalog.md#rfc4861-rcfg-12) | selected | [Router Advertisement fields](../../protocol/nd/checks/router-discovery.md#router-advertisement-fields) | `Rfc4861RaCurHopLimit` | FAIL | fails: [gap 2](results.md#gap-2-defect--two-defaults-of-the-router-are-wrong) |
| [RFC4861-RCFG-13](../../standard/rfc4861/catalog.md#rfc4861-rcfg-13) | selected | [Router Advertisement fields](../../protocol/nd/checks/router-discovery.md#router-advertisement-fields) | `Rfc4861RaFields` | PASS | — |
| [RFC4861-RCFG-14](../../standard/rfc4861/catalog.md#rfc4861-rcfg-14) | selected | [Prefix Information option](../../protocol/nd/checks/router-discovery.md#prefix-information-option) | `Rfc4861RaPrefixInformation` | PASS | — |
| [RFC4861-RCFG-15](../../standard/rfc4861/catalog.md#rfc4861-rcfg-15) | selected | [Prefix Information option](../../protocol/nd/checks/router-discovery.md#prefix-information-option) | `Rfc4861RaPrefixInformation` | PASS | — |
| [RFC4861-RCFG-16](../../standard/rfc4861/catalog.md#rfc4861-rcfg-16) | selected | [Prefix Information option](../../protocol/nd/checks/router-discovery.md#prefix-information-option) | `Rfc4861RaPrefixInformation` | PASS | — |
| [RFC4861-RCFG-17](../../standard/rfc4861/catalog.md#rfc4861-rcfg-17) | covered | [Prefix Information option](../../protocol/nd/checks/router-discovery.md#prefix-information-option) | `Rfc4861RaPrefixInformation` | PASS | — |
| [RFC4861-RCFG-18](../../standard/rfc4861/catalog.md#rfc4861-rcfg-18) | selected | [Prefix Information option](../../protocol/nd/checks/router-discovery.md#prefix-information-option) | `Rfc4861RaPrefixInformation` | PASS | — |
| [RFC4861-RCFG-19](../../standard/rfc4861/catalog.md#rfc4861-rcfg-19) | selected | [Prefix Information option](../../protocol/nd/checks/router-discovery.md#prefix-information-option) | `Rfc4861RaPrefixInformation` | PASS | — |
| [RFC4861-RCFG-20](../../standard/rfc4861/catalog.md#rfc4861-rcfg-20) | covered | [Prefix Information option](../../protocol/nd/checks/router-discovery.md#prefix-information-option) | `Rfc4861RaPrefixInformation` | PASS | — |
| [RFC4861-RCFG-21](../../standard/rfc4861/catalog.md#rfc4861-rcfg-21) | selected | [Prefix Information option](../../protocol/nd/checks/router-discovery.md#prefix-information-option) | `Rfc4861RaPrefixInformation` | PASS | — |
| [RFC4861-RCFG-22](../../standard/rfc4861/catalog.md#rfc4861-rcfg-22) | selected | [Prefix Information option](../../protocol/nd/checks/router-discovery.md#prefix-information-option) | `Rfc4861RaPrefixInformation` | PASS | — |
| [RFC4861-RCFG-23](../../standard/rfc4861/catalog.md#rfc4861-rcfg-23) | selected | [Address resolution failure](../../protocol/nd/checks/address-resolution.md#address-resolution-failure) | `Rfc4861AddressResolutionFailure` | PASS | — |
| [RFC4861-ADV-1](../../standard/rfc4861/catalog.md#rfc4861-adv-1) | owed | — | — | — | a later level 2 pass: an interface that does not advertise |
| [RFC4861-ADV-2](../../standard/rfc4861/catalog.md#rfc4861-adv-2) | owed | — | — | — | a later level 2 pass: a router that stops advertising, changes its link-local address or becomes a host, and two routers with inconsistent advertisements |
| [RFC4861-ADV-3](../../standard/rfc4861/catalog.md#rfc4861-adv-3) | selected | [All-routers group of a router](../../protocol/nd/checks/multicast.md#all-routers-group-of-a-router) | `Rfc4861AllRoutersGroup` | PASS | — |
| [RFC4861-ADV-4](../../standard/rfc4861/catalog.md#rfc4861-adv-4) | selected | [Router Advertisement header and addressing](../../protocol/nd/checks/router-discovery.md#router-advertisement-header-and-addressing) | `Rfc4861RaHeader` | PASS | — |
| [RFC4861-ADV-5](../../standard/rfc4861/catalog.md#rfc4861-adv-5) | selected | [Router Advertisement fields](../../protocol/nd/checks/router-discovery.md#router-advertisement-fields) | `Rfc4861RaFields` | PASS | — |
| [RFC4861-ADV-6](../../standard/rfc4861/catalog.md#rfc4861-adv-6) | selected | [Router Advertisement fields](../../protocol/nd/checks/router-discovery.md#router-advertisement-fields) | `Rfc4861RaFields` | PASS | — |
| [RFC4861-ADV-7](../../standard/rfc4861/catalog.md#rfc4861-adv-7) | selected | [Router Advertisement fields](../../protocol/nd/checks/router-discovery.md#router-advertisement-fields) | `Rfc4861RaCurHopLimit` | FAIL | fails: [gap 2](results.md#gap-2-defect--two-defaults-of-the-router-are-wrong) |
| [RFC4861-ADV-8](../../standard/rfc4861/catalog.md#rfc4861-adv-8) | selected | [Router Advertisement fields](../../protocol/nd/checks/router-discovery.md#router-advertisement-fields) | `Rfc4861RaFields` | PASS | — |
| [RFC4861-ADV-9](../../standard/rfc4861/catalog.md#rfc4861-adv-9) | selected | [Router Advertisement fields](../../protocol/nd/checks/router-discovery.md#router-advertisement-fields) | `Rfc4861RaFields` | PASS | — |
| [RFC4861-ADV-10](../../standard/rfc4861/catalog.md#rfc4861-adv-10) | covered | [Router Advertisement fields](../../protocol/nd/checks/router-discovery.md#router-advertisement-fields) | `Rfc4861RaFields` | PASS | — |
| [RFC4861-ADV-11](../../standard/rfc4861/catalog.md#rfc4861-adv-11) | selected | [Router Advertisement fields](../../protocol/nd/checks/router-discovery.md#router-advertisement-fields) | `Rfc4861RaMtuOption` | FAIL | fails: [gap 2](results.md#gap-2-defect--two-defaults-of-the-router-are-wrong) |
| [RFC4861-ADV-12](../../standard/rfc4861/catalog.md#rfc4861-adv-12) | selected | [Prefix Information option](../../protocol/nd/checks/router-discovery.md#prefix-information-option) | `Rfc4861RaPrefixInformation` | PASS | — |
| [RFC4861-ADV-13](../../standard/rfc4861/catalog.md#rfc4861-adv-13) | selected | [Prefix Information option](../../protocol/nd/checks/router-discovery.md#prefix-information-option) | `Rfc4861RaPrefixInformation` | PASS | — |
| [RFC4861-ADV-14](../../standard/rfc4861/catalog.md#rfc4861-adv-14) | selected | [Prefix Information option](../../protocol/nd/checks/router-discovery.md#prefix-information-option) | `Rfc4861RaPrefixInformation` | PASS | — |
| [RFC4861-ADV-15](../../standard/rfc4861/catalog.md#rfc4861-adv-15) | selected | [Prefix Information option](../../protocol/nd/checks/router-discovery.md#prefix-information-option) | `Rfc4861RaPrefixInformation` | PASS | — |
| [RFC4861-ADV-16](../../standard/rfc4861/catalog.md#rfc4861-adv-16) | selected | [Prefix Information option](../../protocol/nd/checks/router-discovery.md#prefix-information-option) | `Rfc4861RaPrefixInformation` | PASS | — |
| [RFC4861-ADV-17](../../standard/rfc4861/catalog.md#rfc4861-adv-17) | covered | [A router with Router Lifetime zero](../../protocol/nd/checks/router-discovery.md#a-router-with-router-lifetime-zero) | `Rfc4861RouterLifetimeZero` | PASS | observation 1 held |
| [RFC4861-ADV-18](../../standard/rfc4861/catalog.md#rfc4861-adv-18) | covered | [Router Advertisement header and addressing](../../protocol/nd/checks/router-discovery.md#router-advertisement-header-and-addressing) | `Rfc4861RaHeader` | PASS | a permission that the check accepts: it reads no option |
| [RFC4861-ADV-19](../../standard/rfc4861/catalog.md#rfc4861-adv-19) | selected | [Router Advertisement in answer to a solicitation](../../protocol/nd/checks/router-discovery.md#router-advertisement-in-answer-to-a-solicitation) | `Rfc4861RaSolicited` | FAIL | not reached: [gap 5](results.md#gap-5-defect--the-router-answers-only-its-first-solicitation) keeps the test from the observation |
| [RFC4861-ADV-20](../../standard/rfc4861/catalog.md#rfc4861-adv-20) | owed | — | — | — | a later level 2 pass: a Router Advertisement larger than the MTU, which a router splits and does not fragment |
| [RFC4861-ADV-21](../../standard/rfc4861/catalog.md#rfc4861-adv-21) | selected | [Router Advertisement header and addressing](../../protocol/nd/checks/router-discovery.md#router-advertisement-header-and-addressing) | `Rfc4861RaHeader` | PASS | — |
| [RFC4861-ADV-22](../../standard/rfc4861/catalog.md#rfc4861-adv-22) | selected | [Unsolicited Router Advertisements](../../protocol/nd/checks/router-discovery.md#unsolicited-router-advertisements) | `Rfc4861RaUnsolicited` | FAIL | not reached: [gap 6](results.md#gap-6-defect--the-first-periodic-advertisement-comes-after-198-to-600-seconds) keeps the test from the observation |
| [RFC4861-ADV-23](../../standard/rfc4861/catalog.md#rfc4861-adv-23) | selected | [Unsolicited Router Advertisements](../../protocol/nd/checks/router-discovery.md#unsolicited-router-advertisements) | `Rfc4861RaUnsolicited` | FAIL | fails: [gap 6](results.md#gap-6-defect--the-first-periodic-advertisement-comes-after-198-to-600-seconds) |
| [RFC4861-ADV-24](../../standard/rfc4861/catalog.md#rfc4861-adv-24) | owed | — | — | — | a later level 2 pass: a router that stops advertising, changes its link-local address or becomes a host, and two routers with inconsistent advertisements |
| [RFC4861-ADV-25](../../standard/rfc4861/catalog.md#rfc4861-adv-25) | owed | — | — | — | a later level 2 pass: a router that stops advertising, changes its link-local address or becomes a host, and two routers with inconsistent advertisements |
| [RFC4861-ADV-26](../../standard/rfc4861/catalog.md#rfc4861-adv-26) | owed | — | — | — | a later level 2 pass: a router that stops advertising, changes its link-local address or becomes a host, and two routers with inconsistent advertisements |
| [RFC4861-ADV-27](../../standard/rfc4861/catalog.md#rfc4861-adv-27) | owed | — | — | — | a later level 2 pass: a router that stops advertising, changes its link-local address or becomes a host, and two routers with inconsistent advertisements |
| [RFC4861-ADV-28](../../standard/rfc4861/catalog.md#rfc4861-adv-28) | owed | — | — | — | a later level 2 pass: a router that stops advertising, changes its link-local address or becomes a host, and two routers with inconsistent advertisements |
| [RFC4861-ADV-29](../../standard/rfc4861/catalog.md#rfc4861-adv-29) | selected | [Router Advertisement in answer to a solicitation](../../protocol/nd/checks/router-discovery.md#router-advertisement-in-answer-to-a-solicitation) | `Rfc4861RaSolicited` | FAIL | fails: [gap 5](results.md#gap-5-defect--the-router-answers-only-its-first-solicitation) |
| [RFC4861-ADV-30](../../standard/rfc4861/catalog.md#rfc4861-adv-30) | covered | [Router Advertisement in answer to a solicitation](../../protocol/nd/checks/router-discovery.md#router-advertisement-in-answer-to-a-solicitation) | `Rfc4861RaSolicited` | FAIL | not reached: [gap 5](results.md#gap-5-defect--the-router-answers-only-its-first-solicitation) keeps the test from the observation |
| [RFC4861-ADV-31](../../standard/rfc4861/catalog.md#rfc4861-adv-31) | selected | [Unsolicited Router Advertisements](../../protocol/nd/checks/router-discovery.md#unsolicited-router-advertisements) | `Rfc4861RaUnsolicited` | FAIL | not reached: [gap 6](results.md#gap-6-defect--the-first-periodic-advertisement-comes-after-198-to-600-seconds) keeps the test from the observation |
| [RFC4861-ADV-32](../../standard/rfc4861/catalog.md#rfc4861-adv-32) | selected | [Router Advertisement in answer to a solicitation](../../protocol/nd/checks/router-discovery.md#router-advertisement-in-answer-to-a-solicitation) | `Rfc4861RaSolicited` | FAIL | fails: [gap 5](results.md#gap-5-defect--the-router-answers-only-its-first-solicitation) |
| [RFC4861-ADV-33](../../standard/rfc4861/catalog.md#rfc4861-adv-33) | selected | [Router Advertisement in answer to a solicitation](../../protocol/nd/checks/router-discovery.md#router-advertisement-in-answer-to-a-solicitation) | `Rfc4861RaSolicited` | PASS | held up to 16.2 s, when the test stopped |
| [RFC4861-ADV-34](../../standard/rfc4861/catalog.md#rfc4861-adv-34) | selected | [Router Advertisement in answer to a solicitation](../../protocol/nd/checks/router-discovery.md#router-advertisement-in-answer-to-a-solicitation) | `Rfc4861RaSolicited` | FAIL | fails: [gap 5](results.md#gap-5-defect--the-router-answers-only-its-first-solicitation) |
| [RFC4861-ADV-35](../../standard/rfc4861/catalog.md#rfc4861-adv-35) | selected | [Router Advertisement in answer to a solicitation](../../protocol/nd/checks/router-discovery.md#router-advertisement-in-answer-to-a-solicitation) | `Rfc4861RaSolicited` | FAIL | fails: [gap 5](results.md#gap-5-defect--the-router-answers-only-its-first-solicitation) |
| [RFC4861-ADV-36](../../standard/rfc4861/catalog.md#rfc4861-adv-36) | selected | [Unsolicited Router Advertisements](../../protocol/nd/checks/router-discovery.md#unsolicited-router-advertisements) | `Rfc4861RaUnsolicited` | FAIL | not reached: [gap 6](results.md#gap-6-defect--the-first-periodic-advertisement-comes-after-198-to-600-seconds) keeps the test from the observation |
| [RFC4861-ADV-37](../../standard/rfc4861/catalog.md#rfc4861-adv-37) | later | — | — | — | level 3: needs a crafted message |
| [RFC4861-ADV-38](../../standard/rfc4861/catalog.md#rfc4861-adv-38) | later | — | — | — | level 3: needs a crafted message |
| [RFC4861-ADV-39](../../standard/rfc4861/catalog.md#rfc4861-adv-39) | later | — | — | — | level 3: needs a crafted message |
| [RFC4861-ADV-40](../../standard/rfc4861/catalog.md#rfc4861-adv-40) | later | — | — | — | level 3: needs a crafted message |
| [RFC4861-ADV-41](../../standard/rfc4861/catalog.md#rfc4861-adv-41) | later | — | — | — | level 3: needs a crafted message |
| [RFC4861-ADV-42](../../standard/rfc4861/catalog.md#rfc4861-adv-42) | owed | — | — | — | a later level 2 pass: a router that stops advertising, changes its link-local address or becomes a host, and two routers with inconsistent advertisements |
| [RFC4861-ADV-43](../../standard/rfc4861/catalog.md#rfc4861-adv-43) | owed | — | — | — | a later level 2 pass: a router that stops advertising, changes its link-local address or becomes a host, and two routers with inconsistent advertisements |
| [RFC4861-ADV-44](../../standard/rfc4861/catalog.md#rfc4861-adv-44) | owed | — | — | — | a later level 2 pass: a router that stops advertising, changes its link-local address or becomes a host, and two routers with inconsistent advertisements |
| [RFC4861-ADV-45](../../standard/rfc4861/catalog.md#rfc4861-adv-45) | owed | — | — | — | a later level 2 pass: a router that stops advertising, changes its link-local address or becomes a host, and two routers with inconsistent advertisements |
| [RFC4861-ADV-46](../../standard/rfc4861/catalog.md#rfc4861-adv-46) | selected | [Redirect from the first-hop router](../../protocol/nd/checks/redirect.md#redirect-from-the-first-hop-router) | `Rfc4861RedirectFirstHop` | PASS | — |
| [RFC4861-ADV-47](../../standard/rfc4861/catalog.md#rfc4861-adv-47) | owed | — | — | — | a later level 2 pass: a router that stops advertising, changes its link-local address or becomes a host, and two routers with inconsistent advertisements |
| [RFC4861-HOST-1](../../standard/rfc4861/catalog.md#rfc4861-host-1) | selected | [Hop limit without a router](../../protocol/nd/checks/parameters.md#hop-limit-without-a-router) | `Rfc4861HopLimitNoRouter` | FAIL | fails: [gap 1](results.md#gap-1-defect--the-hop-limit-of-a-host-is-a-constant) |
| [RFC4861-HOST-2](../../standard/rfc4861/catalog.md#rfc4861-host-2) | later | — | — | — | not a behavior of the model: a right of the documents for other link layers |
| [RFC4861-HOST-3](../../standard/rfc4861/catalog.md#rfc4861-host-3) | selected | [MTU from the router](../../protocol/nd/checks/parameters.md#mtu-from-the-router) | `Rfc4861MtuFromRouter` | FAIL | fails: [gap 3](results.md#gap-3-defect--the-host-fragments-to-the-mtu-of-the-interface-not-to-the-advertised-mtu) |
| [RFC4861-HOST-4](../../standard/rfc4861/catalog.md#rfc4861-host-4) | selected | [Hop limit without a router](../../protocol/nd/checks/parameters.md#hop-limit-without-a-router) | `Rfc4861HopLimitNoRouter` | FAIL | fails: [gap 1](results.md#gap-1-defect--the-hop-limit-of-a-host-is-a-constant) |
| [RFC4861-HOST-5](../../standard/rfc4861/catalog.md#rfc4861-host-5) | later | — | — | — | level 4: state inside the node |
| [RFC4861-HOST-6](../../standard/rfc4861/catalog.md#rfc4861-host-6) | later | — | — | — | level 4: a statistical test of the random delay; a check above tests the bound where one exists |
| [RFC4861-HOST-7](../../standard/rfc4861/catalog.md#rfc4861-host-7) | selected | [Address resolution failure](../../protocol/nd/checks/address-resolution.md#address-resolution-failure) | `Rfc4861AddressResolutionFailure` | PASS | — |
| [RFC4861-HOST-8](../../standard/rfc4861/catalog.md#rfc4861-host-8) | later | — | — | — | level 4: MLD never reports the all-nodes group, so only the state of the node shows the join |
| [RFC4861-HOST-9](../../standard/rfc4861/catalog.md#rfc4861-host-9) | owed | — | — | — | a later level 2 pass: three or more routers, for default router selection, and two routers that advertise different values of one parameter |
| [RFC4861-HOST-10](../../standard/rfc4861/catalog.md#rfc4861-host-10) | owed | — | — | — | a later level 2 pass: three or more routers, for default router selection, and two routers that advertise different values of one parameter |
| [RFC4861-HOST-11](../../standard/rfc4861/catalog.md#rfc4861-host-11) | owed | — | — | — | a later level 2 pass: a router variable of zero, which leaves the host value as it is, and a lifetime of all one bits, which is infinity |
| [RFC4861-HOST-12](../../standard/rfc4861/catalog.md#rfc4861-host-12) | selected | [The default router](../../protocol/nd/checks/router-discovery.md#the-default-router) | `Rfc4861DefaultRouter` | PASS | passes with one router; with two routers [gap 7](results.md#gap-7-defect--a-host-drops-every-advertisement-during-duplicate-address-detection) breaks it, see `Rfc4861RouterLifetimeZero` |
| [RFC4861-HOST-13](../../standard/rfc4861/catalog.md#rfc4861-host-13) | later | — | — | — | level 4: state inside the node |
| [RFC4861-HOST-14](../../standard/rfc4861/catalog.md#rfc4861-host-14) | owed | — | — | — | a later level 2 pass: a router that stops advertising, changes its link-local address or becomes a host, and two routers with inconsistent advertisements |
| [RFC4861-HOST-15](../../standard/rfc4861/catalog.md#rfc4861-host-15) | owed | — | — | — | a later level 2 pass: three or more routers, for default router selection, and two routers that advertise different values of one parameter |
| [RFC4861-HOST-16](../../standard/rfc4861/catalog.md#rfc4861-host-16) | selected | [Hop limit from the router](../../protocol/nd/checks/parameters.md#hop-limit-from-the-router) | `Rfc4861HopLimitFromRouter` | FAIL | fails: [gap 1](results.md#gap-1-defect--the-hop-limit-of-a-host-is-a-constant) |
| [RFC4861-HOST-17](../../standard/rfc4861/catalog.md#rfc4861-host-17) | later | — | — | — | level 4: a statistical test of the random delay; a check above tests the bound where one exists |
| [RFC4861-HOST-18](../../standard/rfc4861/catalog.md#rfc4861-host-18) | later | — | — | — | level 4: a statistical test of the random delay; a check above tests the bound where one exists |
| [RFC4861-HOST-19](../../standard/rfc4861/catalog.md#rfc4861-host-19) | selected | [Retransmission timer from the router](../../protocol/nd/checks/parameters.md#retransmission-timer-from-the-router) | `Rfc4861RetransTimerFromRouter` | FAIL | fails: [gap 4](results.md#gap-4-defect--address-resolution-waits-a-constant-not-retranstimer) |
| [RFC4861-HOST-20](../../standard/rfc4861/catalog.md#rfc4861-host-20) | later | — | — | — | level 4: state inside the node |
| [RFC4861-HOST-21](../../standard/rfc4861/catalog.md#rfc4861-host-21) | later | — | — | — | level 4: state inside the node |
| [RFC4861-HOST-22](../../standard/rfc4861/catalog.md#rfc4861-host-22) | later | — | — | — | level 4: state inside the node |
| [RFC4861-HOST-23](../../standard/rfc4861/catalog.md#rfc4861-host-23) | later | — | — | — | level 4: state inside the node |
| [RFC4861-HOST-24](../../standard/rfc4861/catalog.md#rfc4861-host-24) | selected | [MTU from the router](../../protocol/nd/checks/parameters.md#mtu-from-the-router) | `Rfc4861MtuFromRouter` | FAIL | fails: [gap 3](results.md#gap-3-defect--the-host-fragments-to-the-mtu-of-the-interface-not-to-the-advertised-mtu) |
| [RFC4861-HOST-25](../../standard/rfc4861/catalog.md#rfc4861-host-25) | selected | [On-link neighbor reached directly](../../protocol/nd/checks/on-link.md#on-link-neighbor-reached-directly) | `Rfc4861OnLinkNeighbor` | PASS | — |
| [RFC4861-HOST-26](../../standard/rfc4861/catalog.md#rfc4861-host-26) | selected | [Prefix with the on-link flag clear](../../protocol/nd/checks/on-link.md#prefix-with-the-on-link-flag-clear) | `Rfc5942OnLinkFlagClear` | PASS | — |
| [RFC4861-HOST-27](../../standard/rfc4861/catalog.md#rfc4861-host-27) | owed | — | — | — | a later level 2 pass: a router that changes the L flag of a prefix, advertises a prefix with Valid Lifetime zero, or refreshes it; a prefix length other than 64; a router whose lifetime runs out |
| [RFC4861-HOST-28](../../standard/rfc4861/catalog.md#rfc4861-host-28) | selected | [Prefix with the on-link flag clear](../../protocol/nd/checks/on-link.md#prefix-with-the-on-link-flag-clear) | `Rfc5942OnLinkFlagClear` | PASS | — |
| [RFC4861-HOST-29](../../standard/rfc4861/catalog.md#rfc4861-host-29) | later | — | — | — | level 3: needs a crafted message |
| [RFC4861-HOST-30](../../standard/rfc4861/catalog.md#rfc4861-host-30) | selected | [On-link neighbor reached directly](../../protocol/nd/checks/on-link.md#on-link-neighbor-reached-directly) | `Rfc4861OnLinkNeighbor` | PASS | — |
| [RFC4861-HOST-31](../../standard/rfc4861/catalog.md#rfc4861-host-31) | owed | — | — | — | a later level 2 pass: a router that changes the L flag of a prefix, advertises a prefix with Valid Lifetime zero, or refreshes it; a prefix length other than 64; a router whose lifetime runs out |
| [RFC4861-HOST-32](../../standard/rfc4861/catalog.md#rfc4861-host-32) | owed | — | — | — | a later level 2 pass: a router that changes the L flag of a prefix, advertises a prefix with Valid Lifetime zero, or refreshes it; a prefix length other than 64; a router whose lifetime runs out |
| [RFC4861-HOST-33](../../standard/rfc4861/catalog.md#rfc4861-host-33) | owed | — | — | — | a later level 2 pass: a router that changes the L flag of a prefix, advertises a prefix with Valid Lifetime zero, or refreshes it; a prefix length other than 64; a router whose lifetime runs out |
| [RFC4861-HOST-34](../../standard/rfc4861/catalog.md#rfc4861-host-34) | covered | [Prefix after its valid lifetime](../../protocol/nd/checks/on-link.md#prefix-after-its-valid-lifetime) | `Rfc5942PrefixLifetime` | PASS | — |
| [RFC4861-HOST-35](../../standard/rfc4861/catalog.md#rfc4861-host-35) | owed | — | — | — | a later level 2 pass: a router that changes the L flag of a prefix, advertises a prefix with Valid Lifetime zero, or refreshes it; a prefix length other than 64; a router whose lifetime runs out |
| [RFC4861-HOST-36](../../standard/rfc4861/catalog.md#rfc4861-host-36) | selected | [Prefix after its valid lifetime](../../protocol/nd/checks/on-link.md#prefix-after-its-valid-lifetime) | `Rfc5942PrefixLifetime` | PASS | — |
| [RFC4861-HOST-37](../../standard/rfc4861/catalog.md#rfc4861-host-37) | owed | — | — | — | a later level 2 pass: a router that changes the L flag of a prefix, advertises a prefix with Valid Lifetime zero, or refreshes it; a prefix length other than 64; a router whose lifetime runs out |
| [RFC4861-HOST-38](../../standard/rfc4861/catalog.md#rfc4861-host-38) | owed | — | — | — | a later level 2 pass: a router that changes the L flag of a prefix, advertises a prefix with Valid Lifetime zero, or refreshes it; a prefix length other than 64; a router whose lifetime runs out |
| [RFC4861-HOST-39](../../standard/rfc4861/catalog.md#rfc4861-host-39) | selected | [The default router](../../protocol/nd/checks/router-discovery.md#the-default-router) | `Rfc4861DefaultRouter` | PASS | — |
| [RFC4861-HOST-40](../../standard/rfc4861/catalog.md#rfc4861-host-40) | owed | — | — | — | a later level 2 pass: three or more routers, for default router selection, and two routers that advertise different values of one parameter |
| [RFC4861-HOST-41](../../standard/rfc4861/catalog.md#rfc4861-host-41) | owed | — | — | — | a later level 2 pass: three or more routers, for default router selection, and two routers that advertise different values of one parameter |
| [RFC4861-HOST-42](../../standard/rfc4861/catalog.md#rfc4861-host-42) | selected | [Router Solicitations on a link without a router](../../protocol/nd/checks/router-discovery.md#router-solicitations-on-a-link-without-a-router) | `Rfc4861RsNoRouter` | PASS | — |
| [RFC4861-HOST-43](../../standard/rfc4861/catalog.md#rfc4861-host-43) | covered | [Router Solicitation of a host that comes up](../../protocol/nd/checks/router-discovery.md#router-solicitation-of-a-host-that-comes-up) | `Rfc4861RsHostComesUp` | PASS | — |
| [RFC4861-HOST-44](../../standard/rfc4861/catalog.md#rfc4861-host-44) | selected | [Router Solicitation of a host that comes up](../../protocol/nd/checks/router-discovery.md#router-solicitation-of-a-host-that-comes-up) | `Rfc4861RsHostComesUp` | PASS | — |
| [RFC4861-HOST-45](../../standard/rfc4861/catalog.md#rfc4861-host-45) | selected | [Router Solicitation of a host that comes up](../../protocol/nd/checks/router-discovery.md#router-solicitation-of-a-host-that-comes-up) | `Rfc4861RsHostComesUp` | PASS | — |
| [RFC4861-HOST-46](../../standard/rfc4861/catalog.md#rfc4861-host-46) | selected | [Router Solicitation of a host that comes up](../../protocol/nd/checks/router-discovery.md#router-solicitation-of-a-host-that-comes-up) | `Rfc4861RsHostComesUp` | PASS | — |
| [RFC4861-HOST-47](../../standard/rfc4861/catalog.md#rfc4861-host-47) | later | — | — | — | level 4: a statistical test of the random delay; a check above tests the bound where one exists |
| [RFC4861-HOST-48](../../standard/rfc4861/catalog.md#rfc4861-host-48) | covered | [Router Solicitation of a host that comes up](../../protocol/nd/checks/router-discovery.md#router-solicitation-of-a-host-that-comes-up) | `Rfc4861RsHostComesUp` | PASS | a permission that the check accepts: it reads no delay |
| [RFC4861-HOST-49](../../standard/rfc4861/catalog.md#rfc4861-host-49) | covered | [Router Solicitation of a host that comes up](../../protocol/nd/checks/router-discovery.md#router-solicitation-of-a-host-that-comes-up) | `Rfc4861RsHostComesUp` | PASS | a permission that the check accepts: it reads no delay |
| [RFC4861-HOST-50](../../standard/rfc4861/catalog.md#rfc4861-host-50) | selected | [Router Solicitation of a host that comes up](../../protocol/nd/checks/router-discovery.md#router-solicitation-of-a-host-that-comes-up) | `Rfc4861RsHostComesUp` | PASS | — |
| [RFC4861-HOST-51](../../standard/rfc4861/catalog.md#rfc4861-host-51) | selected | [Router Solicitation of a host that comes up](../../protocol/nd/checks/router-discovery.md#router-solicitation-of-a-host-that-comes-up) | `Rfc4861RsHostComesUp` | PASS | — |
| [RFC4861-HOST-52](../../standard/rfc4861/catalog.md#rfc4861-host-52) | selected | [Router Solicitations on a link without a router](../../protocol/nd/checks/router-discovery.md#router-solicitations-on-a-link-without-a-router) | `Rfc4861RsNoRouter` | PASS | — |
| [RFC4861-AR-1](../../standard/rfc4861/catalog.md#rfc4861-ar-1) | selected | [On-link neighbor reached directly](../../protocol/nd/checks/on-link.md#on-link-neighbor-reached-directly) | `Rfc4861OnLinkNeighbor` | PASS | — |
| [RFC4861-AR-2](../../standard/rfc4861/catalog.md#rfc4861-ar-2) | covered | [Neighbor Solicitation and Advertisement fields](../../protocol/nd/checks/address-resolution.md#neighbor-solicitation-and-advertisement-fields) | `Rfc4861NsNaFields` | PASS | — |
| [RFC4861-AR-3](../../standard/rfc4861/catalog.md#rfc4861-ar-3) | later | — | — | — | level 4: state inside the node |
| [RFC4861-AR-4](../../standard/rfc4861/catalog.md#rfc4861-ar-4) | selected | [Groups joined before Duplicate Address Detection](../../protocol/nd/checks/multicast.md#groups-joined-before-duplicate-address-detection) | `Rfc4862GroupsBeforeDad` | FAIL | fails: [gap 9](results.md#gap-9-defect--a-node-does-not-join-its-solicited-node-groups) |
| [RFC4861-AR-5](../../standard/rfc4861/catalog.md#rfc4861-ar-5) | owed | — | — | — | a later level 2 pass: an address with an interface identifier of its own, which has its own solicited-node group, and the removal of an address |
| [RFC4861-AR-6](../../standard/rfc4861/catalog.md#rfc4861-ar-6) | selected | [Groups joined before Duplicate Address Detection](../../protocol/nd/checks/multicast.md#groups-joined-before-duplicate-address-detection) | `Rfc4862GroupsBeforeDad` | FAIL | fails: [gap 9](results.md#gap-9-defect--a-node-does-not-join-its-solicited-node-groups) |
| [RFC4861-AR-7](../../standard/rfc4861/catalog.md#rfc4861-ar-7) | owed | — | — | — | a later level 2 pass: an address with an interface identifier of its own, which has its own solicited-node group, and the removal of an address |
| [RFC4861-AR-8](../../standard/rfc4861/catalog.md#rfc4861-ar-8) | selected | [Address resolution of a host](../../protocol/nd/checks/address-resolution.md#address-resolution-of-a-host) | `Rfc4861AddressResolutionHost` | PASS | — |
| [RFC4861-AR-9](../../standard/rfc4861/catalog.md#rfc4861-ar-9) | selected | [Address resolution of a host](../../protocol/nd/checks/address-resolution.md#address-resolution-of-a-host) | `Rfc4861AddressResolutionHost` | PASS | — |
| [RFC4861-AR-10](../../standard/rfc4861/catalog.md#rfc4861-ar-10) | selected | [Address resolution of a host](../../protocol/nd/checks/address-resolution.md#address-resolution-of-a-host) | `Rfc4861AddressResolutionHost` | PASS | — |
| [RFC4861-AR-11](../../standard/rfc4861/catalog.md#rfc4861-ar-11) | selected | [Address resolution failure](../../protocol/nd/checks/address-resolution.md#address-resolution-failure) | `Rfc4861AddressResolutionFailure` | PASS | — |
| [RFC4861-AR-12](../../standard/rfc4861/catalog.md#rfc4861-ar-12) | selected | [Address resolution of a host](../../protocol/nd/checks/address-resolution.md#address-resolution-of-a-host) | `Rfc4861AddressResolutionHost` | PASS | — |
| [RFC4861-AR-13](../../standard/rfc4861/catalog.md#rfc4861-ar-13) | later | — | — | — | level 4: Neighbor Unreachability Detection, RFC 4861 §7.3, sends the unicast solicitations and uses these flags |
| [RFC4861-AR-14](../../standard/rfc4861/catalog.md#rfc4861-ar-14) | selected | [Address resolution of a host](../../protocol/nd/checks/address-resolution.md#address-resolution-of-a-host) | `Rfc4861AddressResolutionHost` | PASS | — |
| [RFC4861-AR-15](../../standard/rfc4861/catalog.md#rfc4861-ar-15) | selected | [Address resolution of a host](../../protocol/nd/checks/address-resolution.md#address-resolution-of-a-host) | `Rfc4861AddressResolutionHost` | PASS | — |
| [RFC4861-AR-16](../../standard/rfc4861/catalog.md#rfc4861-ar-16) | owed | — | — | — | a later level 2 pass: a burst of packets to one neighbor that is not resolved yet, a stream of packets that causes Redirects, for the rate limit, and two flows to one destination with different Flow Labels |
| [RFC4861-AR-17](../../standard/rfc4861/catalog.md#rfc4861-ar-17) | owed | — | — | — | a later level 2 pass: a burst of packets to one neighbor that is not resolved yet, a stream of packets that causes Redirects, for the rate limit, and two flows to one destination with different Flow Labels |
| [RFC4861-AR-18](../../standard/rfc4861/catalog.md#rfc4861-ar-18) | selected | [Address resolution of a host](../../protocol/nd/checks/address-resolution.md#address-resolution-of-a-host) | `Rfc4861AddressResolutionHost` | PASS | — |
| [RFC4861-AR-19](../../standard/rfc4861/catalog.md#rfc4861-ar-19) | selected | [Address resolution failure](../../protocol/nd/checks/address-resolution.md#address-resolution-failure) | `Rfc4861AddressResolutionFailure` | PASS | — |
| [RFC4861-AR-20](../../standard/rfc4861/catalog.md#rfc4861-ar-20) | selected | [Address resolution failure](../../protocol/nd/checks/address-resolution.md#address-resolution-failure) | `Rfc4861AddressResolutionFailure` | PASS | — |
| [RFC4861-AR-21](../../standard/rfc4861/catalog.md#rfc4861-ar-21) | selected | [Address resolution failure](../../protocol/nd/checks/address-resolution.md#address-resolution-failure) | `Rfc4861AddressResolutionFailure` | PASS | — |
| [RFC4861-AR-22](../../standard/rfc4861/catalog.md#rfc4861-ar-22) | selected | [Address resolution failure](../../protocol/nd/checks/address-resolution.md#address-resolution-failure) | `Rfc4861AddressResolutionFailure` | PASS | — |
| [RFC4861-AR-23](../../standard/rfc4861/catalog.md#rfc4861-ar-23) | selected | [Address resolution failure](../../protocol/nd/checks/address-resolution.md#address-resolution-failure) | `Rfc4861AddressResolutionFailure` | PASS | — |
| [RFC4861-AR-24](../../standard/rfc4861/catalog.md#rfc4861-ar-24) | selected | [Address resolution of a host](../../protocol/nd/checks/address-resolution.md#address-resolution-of-a-host) | `Rfc4861AddressResolutionHost` | PASS | — |
| [RFC4861-AR-25](../../standard/rfc4861/catalog.md#rfc4861-ar-25) | later | — | — | — | level 4: state inside the node |
| [RFC4861-AR-26](../../standard/rfc4861/catalog.md#rfc4861-ar-26) | later | — | — | — | level 4: state inside the node |
| [RFC4861-AR-27](../../standard/rfc4861/catalog.md#rfc4861-ar-27) | later | — | — | — | level 4: state inside the node |
| [RFC4861-AR-28](../../standard/rfc4861/catalog.md#rfc4861-ar-28) | later | — | — | — | level 4: state inside the node |
| [RFC4861-AR-29](../../standard/rfc4861/catalog.md#rfc4861-ar-29) | later | — | — | — | level 4: state inside the node |
| [RFC4861-AR-30](../../standard/rfc4861/catalog.md#rfc4861-ar-30) | covered | [Address resolution of a host](../../protocol/nd/checks/address-resolution.md#address-resolution-of-a-host) | `Rfc4861AddressResolutionHost` | PASS | — |
| [RFC4861-AR-31](../../standard/rfc4861/catalog.md#rfc4861-ar-31) | selected | [Address resolution of a host](../../protocol/nd/checks/address-resolution.md#address-resolution-of-a-host) | `Rfc4861AddressResolutionHost` | PASS | — |
| [RFC4861-AR-32](../../standard/rfc4861/catalog.md#rfc4861-ar-32) | selected | [Address resolution of a host](../../protocol/nd/checks/address-resolution.md#address-resolution-of-a-host) | `Rfc4861AddressResolutionHost` | PASS | — |
| [RFC4861-AR-33](../../standard/rfc4861/catalog.md#rfc4861-ar-33) | later | — | — | — | level 4: Neighbor Unreachability Detection, RFC 4861 §7.3, sends the unicast solicitations and uses these flags |
| [RFC4861-AR-34](../../standard/rfc4861/catalog.md#rfc4861-ar-34) | selected | [Address resolution of a host](../../protocol/nd/checks/address-resolution.md#address-resolution-of-a-host) | `Rfc4861AddressResolutionHost` | PASS | — |
| [RFC4861-AR-35](../../standard/rfc4861/catalog.md#rfc4861-ar-35) | selected | [Neighbor Solicitation and Advertisement fields](../../protocol/nd/checks/address-resolution.md#neighbor-solicitation-and-advertisement-fields) | `Rfc4861NsNaFields` | PASS | — |
| [RFC4861-AR-36](../../standard/rfc4861/catalog.md#rfc4861-ar-36) | owed | — | — | — | a later level 2 pass: a proxy |
| [RFC4861-AR-37](../../standard/rfc4861/catalog.md#rfc4861-ar-37) | selected | [Neighbor Solicitation and Advertisement fields](../../protocol/nd/checks/address-resolution.md#neighbor-solicitation-and-advertisement-fields) | `Rfc4861NsNaFields` | PASS | — |
| [RFC4861-AR-38](../../standard/rfc4861/catalog.md#rfc4861-ar-38) | selected | [Duplicate link-local address](../../protocol/nd/checks/autoconfiguration.md#duplicate-link-local-address) | `Rfc4862DuplicateLinkLocal` | PASS | — |
| [RFC4861-AR-39](../../standard/rfc4861/catalog.md#rfc4861-ar-39) | selected | [Neighbor Solicitation and Advertisement fields](../../protocol/nd/checks/address-resolution.md#neighbor-solicitation-and-advertisement-fields) | `Rfc4861NsNaFields` | PASS | — |
| [RFC4861-AR-40](../../standard/rfc4861/catalog.md#rfc4861-ar-40) | no check | — | — | — | the model declines anycast targets: "TODO: anycast target address handling is not implemented", `Ipv6NeighbourDiscovery.cc:2002` |
| [RFC4861-AR-41](../../standard/rfc4861/catalog.md#rfc4861-ar-41) | selected | [Address resolution of a host](../../protocol/nd/checks/address-resolution.md#address-resolution-of-a-host) | `Rfc4861AddressResolutionHost` | PASS | — |
| [RFC4861-AR-42](../../standard/rfc4861/catalog.md#rfc4861-ar-42) | later | — | — | — | level 3: needs a crafted message |
| [RFC4861-AR-43](../../standard/rfc4861/catalog.md#rfc4861-ar-43) | later | — | — | — | level 3: needs a crafted message |
| [RFC4861-AR-44](../../standard/rfc4861/catalog.md#rfc4861-ar-44) | selected | [Address resolution of a host](../../protocol/nd/checks/address-resolution.md#address-resolution-of-a-host) | `Rfc4861AddressResolutionHost` | PASS | — |
| [RFC4861-AR-45](../../standard/rfc4861/catalog.md#rfc4861-ar-45) | later | — | — | — | level 4: state inside the node |
| [RFC4861-AR-46](../../standard/rfc4861/catalog.md#rfc4861-ar-46) | later | — | — | — | level 4: state inside the node |
| [RFC4861-AR-47](../../standard/rfc4861/catalog.md#rfc4861-ar-47) | later | — | — | — | level 3: needs a crafted message |
| [RFC4861-AR-48](../../standard/rfc4861/catalog.md#rfc4861-ar-48) | later | — | — | — | level 3: needs a crafted message |
| [RFC4861-AR-49](../../standard/rfc4861/catalog.md#rfc4861-ar-49) | later | — | — | — | level 3: needs a crafted message |
| [RFC4861-AR-50](../../standard/rfc4861/catalog.md#rfc4861-ar-50) | later | — | — | — | level 3: needs a crafted message |
| [RFC4861-AR-51](../../standard/rfc4861/catalog.md#rfc4861-ar-51) | later | — | — | — | level 3: needs a crafted message |
| [RFC4861-AR-52](../../standard/rfc4861/catalog.md#rfc4861-ar-52) | later | — | — | — | level 3: needs a crafted message |
| [RFC4861-AR-53](../../standard/rfc4861/catalog.md#rfc4861-ar-53) | covered | [Neighbor Solicitation and Advertisement fields](../../protocol/nd/checks/address-resolution.md#neighbor-solicitation-and-advertisement-fields) | `Rfc4861NsNaFields` | PASS | — |
| [RFC4861-AR-54](../../standard/rfc4861/catalog.md#rfc4861-ar-54) | owed | — | — | — | a later level 2 pass: a change of the link-layer address of a node, for unsolicited advertisements |
| [RFC4861-AR-55](../../standard/rfc4861/catalog.md#rfc4861-ar-55) | later | — | — | — | level 4: state inside the node |
| [RFC4861-AR-56](../../standard/rfc4861/catalog.md#rfc4861-ar-56) | owed | — | — | — | a later level 2 pass: a router that stops advertising, changes its link-local address or becomes a host, and two routers with inconsistent advertisements |
| [RFC4861-AR-57](../../standard/rfc4861/catalog.md#rfc4861-ar-57) | owed | — | — | — | a later level 2 pass: a change of the link-layer address of a node, for unsolicited advertisements |
| [RFC4861-AR-58](../../standard/rfc4861/catalog.md#rfc4861-ar-58) | owed | — | — | — | a later level 2 pass: a change of the link-layer address of a node, for unsolicited advertisements |
| [RFC4861-AR-59](../../standard/rfc4861/catalog.md#rfc4861-ar-59) | owed | — | — | — | a later level 2 pass: a change of the link-layer address of a node, for unsolicited advertisements |
| [RFC4861-AR-60](../../standard/rfc4861/catalog.md#rfc4861-ar-60) | owed | — | — | — | a later level 2 pass: a change of the link-layer address of a node, for unsolicited advertisements |
| [RFC4861-AR-61](../../standard/rfc4861/catalog.md#rfc4861-ar-61) | owed | — | — | — | a later level 2 pass: a change of the link-layer address of a node, for unsolicited advertisements |
| [RFC4861-AR-62](../../standard/rfc4861/catalog.md#rfc4861-ar-62) | owed | — | — | — | a later level 2 pass: a change of the link-layer address of a node, for unsolicited advertisements |
| [RFC4861-AR-63](../../standard/rfc4861/catalog.md#rfc4861-ar-63) | later | — | — | — | level 3: needs a crafted message |
| [RFC4861-AR-64](../../standard/rfc4861/catalog.md#rfc4861-ar-64) | later | — | — | — | level 3: needs a crafted message |
| [RFC4861-AR-65](../../standard/rfc4861/catalog.md#rfc4861-ar-65) | owed | — | — | — | a later level 2 pass: a change of the link-layer address of a node, for unsolicited advertisements |
| [RFC4861-AR-66](../../standard/rfc4861/catalog.md#rfc4861-ar-66) | owed | — | — | — | a later level 2 pass: a change of the link-layer address of a node, for unsolicited advertisements |
| [RFC4861-AR-67](../../standard/rfc4861/catalog.md#rfc4861-ar-67) | owed | — | — | — | a later level 2 pass: a change of the link-layer address of a node, for unsolicited advertisements |
| [RFC4861-AR-68](../../standard/rfc4861/catalog.md#rfc4861-ar-68) | owed | — | — | — | a later level 2 pass: a change of the link-layer address of a node, for unsolicited advertisements |
| [RFC4861-ANY-1](../../standard/rfc4861/catalog.md#rfc4861-any-1) | no check | — | — | — | the model declines anycast targets: "TODO: anycast target address handling is not implemented", `Ipv6NeighbourDiscovery.cc:2002` |
| [RFC4861-ANY-2](../../standard/rfc4861/catalog.md#rfc4861-any-2) | no check | — | — | — | the model declines anycast targets: "TODO: anycast target address handling is not implemented", `Ipv6NeighbourDiscovery.cc:2002` |
| [RFC4861-ANY-3](../../standard/rfc4861/catalog.md#rfc4861-any-3) | no check | — | — | — | the model declines anycast targets: "TODO: anycast target address handling is not implemented", `Ipv6NeighbourDiscovery.cc:2002` |
| [RFC4861-ANY-4](../../standard/rfc4861/catalog.md#rfc4861-any-4) | no check | — | — | — | the model declines anycast targets: "TODO: anycast target address handling is not implemented", `Ipv6NeighbourDiscovery.cc:2002` |
| [RFC4861-ANY-5](../../standard/rfc4861/catalog.md#rfc4861-any-5) | owed | — | — | — | a later level 2 pass: a proxy |
| [RFC4861-ANY-6](../../standard/rfc4861/catalog.md#rfc4861-any-6) | owed | — | — | — | a later level 2 pass: a proxy |
| [RFC4861-ANY-7](../../standard/rfc4861/catalog.md#rfc4861-any-7) | owed | — | — | — | a later level 2 pass: a proxy |
| [RFC4861-ANY-8](../../standard/rfc4861/catalog.md#rfc4861-any-8) | owed | — | — | — | a later level 2 pass: a proxy |
| [RFC4861-ANY-9](../../standard/rfc4861/catalog.md#rfc4861-any-9) | owed | — | — | — | a later level 2 pass: a proxy |
| [RFC4861-ANY-10](../../standard/rfc4861/catalog.md#rfc4861-any-10) | owed | — | — | — | a later level 2 pass: a proxy |
| [RFC4861-RDVAL-1](../../standard/rfc4861/catalog.md#rfc4861-rdval-1) | later | — | — | — | level 3: needs a crafted message |
| [RFC4861-RDVAL-2](../../standard/rfc4861/catalog.md#rfc4861-rdval-2) | selected | [Redirect from the first-hop router](../../protocol/nd/checks/redirect.md#redirect-from-the-first-hop-router) | `Rfc4861RedirectFirstHop` | PASS | — |
| [RFC4861-RDVAL-3](../../standard/rfc4861/catalog.md#rfc4861-rdval-3) | later | — | — | — | level 3: needs a crafted message |
| [RFC4861-RDVAL-4](../../standard/rfc4861/catalog.md#rfc4861-rdval-4) | later | — | — | — | level 3: needs a crafted message |
| [RFC4861-RDVAL-5](../../standard/rfc4861/catalog.md#rfc4861-rdval-5) | later | — | — | — | level 3: needs a crafted message |
| [RFC4861-RDVAL-6](../../standard/rfc4861/catalog.md#rfc4861-rdval-6) | later | — | — | — | level 3: needs a crafted message |
| [RFC4861-RDVAL-7](../../standard/rfc4861/catalog.md#rfc4861-rdval-7) | later | — | — | — | level 3: needs a crafted message |
| [RFC4861-RDVAL-8](../../standard/rfc4861/catalog.md#rfc4861-rdval-8) | later | — | — | — | level 3: needs a crafted message |
| [RFC4861-RDVAL-9](../../standard/rfc4861/catalog.md#rfc4861-rdval-9) | later | — | — | — | level 3: needs a crafted message |
| [RFC4861-RDVAL-10](../../standard/rfc4861/catalog.md#rfc4861-rdval-10) | later | — | — | — | level 3: needs a crafted message |
| [RFC4861-RDVAL-11](../../standard/rfc4861/catalog.md#rfc4861-rdval-11) | later | — | — | — | level 3: needs a crafted message |
| [RFC4861-RDVAL-12](../../standard/rfc4861/catalog.md#rfc4861-rdval-12) | later | — | — | — | level 3: needs a crafted message |
| [RFC4861-RDVAL-13](../../standard/rfc4861/catalog.md#rfc4861-rdval-13) | later | — | — | — | level 3: needs a crafted message |
| [RFC4861-RDVAL-14](../../standard/rfc4861/catalog.md#rfc4861-rdval-14) | later | — | — | — | level 3: needs a crafted message |
| [RFC4861-RDR-1](../../standard/rfc4861/catalog.md#rfc4861-rdr-1) | selected | [Redirect from the first-hop router](../../protocol/nd/checks/redirect.md#redirect-from-the-first-hop-router) | `Rfc4861RedirectFirstHop` | PASS | — |
| [RFC4861-RDR-2](../../standard/rfc4861/catalog.md#rfc4861-rdr-2) | selected | [Redirect from the first-hop router](../../protocol/nd/checks/redirect.md#redirect-from-the-first-hop-router) | `Rfc4861RedirectFirstHop` | PASS | — |
| [RFC4861-RDR-3](../../standard/rfc4861/catalog.md#rfc4861-rdr-3) | selected | [Redirect from the first-hop router](../../protocol/nd/checks/redirect.md#redirect-from-the-first-hop-router) | `Rfc4861RedirectFirstHop` | PASS | — |
| [RFC4861-RDR-4](../../standard/rfc4861/catalog.md#rfc4861-rdr-4) | selected | [Redirect from the first-hop router](../../protocol/nd/checks/redirect.md#redirect-from-the-first-hop-router) | `Rfc4861RedirectFirstHop` | PASS | — |
| [RFC4861-RDR-5](../../standard/rfc4861/catalog.md#rfc4861-rdr-5) | selected | [Redirect to an on-link destination](../../protocol/nd/checks/redirect.md#redirect-to-an-on-link-destination) | `Rfc4861RedirectOnLink` | PASS | — |
| [RFC4861-RDR-6](../../standard/rfc4861/catalog.md#rfc4861-rdr-6) | selected | [Redirect from the first-hop router](../../protocol/nd/checks/redirect.md#redirect-from-the-first-hop-router) | `Rfc4861RedirectFirstHop` | PASS | — |
| [RFC4861-RDR-7](../../standard/rfc4861/catalog.md#rfc4861-rdr-7) | selected | [Redirect fields](../../protocol/nd/checks/redirect.md#redirect-fields) | `Rfc4861RedirectFields` | FAIL | fails: [gap 8](results.md#gap-8-defect--the-redirect-carries-no-option) |
| [RFC4861-RDR-8](../../standard/rfc4861/catalog.md#rfc4861-rdr-8) | selected | [Redirect of a large packet](../../protocol/nd/checks/redirect.md#redirect-of-a-large-packet) | `Rfc6980RedirectLargePacket` | FAIL | fails: [gap 8](results.md#gap-8-defect--the-redirect-carries-no-option) |
| [RFC4861-RDR-9](../../standard/rfc4861/catalog.md#rfc4861-rdr-9) | owed | — | — | — | a later level 2 pass: a burst of packets to one neighbor that is not resolved yet, a stream of packets that causes Redirects, for the rate limit, and two flows to one destination with different Flow Labels |
| [RFC4861-RDR-10](../../standard/rfc4861/catalog.md#rfc4861-rdr-10) | later | — | — | — | level 3: needs a crafted message |
| [RFC4861-RDH-1](../../standard/rfc4861/catalog.md#rfc4861-rdh-1) | selected | [Host follows a Redirect](../../protocol/nd/checks/redirect.md#host-follows-a-redirect) | `Rfc4861HostFollowsRedirect` | PASS | — |
| [RFC4861-RDH-2](../../standard/rfc4861/catalog.md#rfc4861-rdh-2) | selected | [Host follows a Redirect](../../protocol/nd/checks/redirect.md#host-follows-a-redirect) | `Rfc4861HostFollowsRedirect` | PASS | — |
| [RFC4861-RDH-3](../../standard/rfc4861/catalog.md#rfc4861-rdh-3) | later | — | — | — | level 4: state inside the node |
| [RFC4861-RDH-4](../../standard/rfc4861/catalog.md#rfc4861-rdh-4) | later | — | — | — | level 4: state inside the node |
| [RFC4861-RDH-5](../../standard/rfc4861/catalog.md#rfc4861-rdh-5) | later | — | — | — | level 4: state inside the node |
| [RFC4861-RDH-6](../../standard/rfc4861/catalog.md#rfc4861-rdh-6) | selected | [Redirect to an on-link destination](../../protocol/nd/checks/redirect.md#redirect-to-an-on-link-destination) | `Rfc4861RedirectOnLink` | PASS | — |
| [RFC4861-RDH-7](../../standard/rfc4861/catalog.md#rfc4861-rdh-7) | later | — | — | — | level 4: state inside the node |
| [RFC4861-RDH-8](../../standard/rfc4861/catalog.md#rfc4861-rdh-8) | later | — | — | — | level 4: state inside the node |
| [RFC4861-RDH-9](../../standard/rfc4861/catalog.md#rfc4861-rdh-9) | owed | — | — | — | a later level 2 pass: a burst of packets to one neighbor that is not resolved yet, a stream of packets that causes Redirects, for the rate limit, and two flows to one destination with different Flow Labels |
| [RFC4861-RDH-10](../../standard/rfc4861/catalog.md#rfc4861-rdh-10) | selected | [Host follows a Redirect](../../protocol/nd/checks/redirect.md#host-follows-a-redirect) | `Rfc4861HostFollowsRedirect` | PASS | — |

### RFC 4862

| Statement | Status | Check | Test | Verdict | Note |
| --- | --- | --- | --- | --- | --- |
| [RFC4862-CONF-1](../../standard/rfc4862/catalog.md#rfc4862-conf-1) | covered | [Link-local address and its Duplicate Address Detection](../../protocol/nd/checks/autoconfiguration.md#link-local-address-and-its-duplicate-address-detection) | `Rfc4862LinkLocalDad` | PASS | — |
| [RFC4862-CONF-2](../../standard/rfc4862/catalog.md#rfc4862-conf-2) | owed | — | — | — | a check with another value of DupAddrDetectTransmits, which the configurator offers as `dupAddrDetectTransmits` |
| [RFC4862-CONF-3](../../standard/rfc4862/catalog.md#rfc4862-conf-3) | selected | [Link-local address and its Duplicate Address Detection](../../protocol/nd/checks/autoconfiguration.md#link-local-address-and-its-duplicate-address-detection) | `Rfc4862LinkLocalDad` | PASS | — |
| [RFC4862-CONF-4](../../standard/rfc4862/catalog.md#rfc4862-conf-4) | selected | [Link-local address and its Duplicate Address Detection](../../protocol/nd/checks/autoconfiguration.md#link-local-address-and-its-duplicate-address-detection) | `Rfc4862LinkLocalDad` | PASS | — |
| [RFC4862-CONF-5](../../standard/rfc4862/catalog.md#rfc4862-conf-5) | selected | [Link-local address and its Duplicate Address Detection](../../protocol/nd/checks/autoconfiguration.md#link-local-address-and-its-duplicate-address-detection) | `Rfc4862LinkLocalDad` | PASS | — |
| [RFC4862-CONF-6](../../standard/rfc4862/catalog.md#rfc4862-conf-6) | later | — | — | — | level 4: state inside the node |
| [RFC4862-LL-1](../../standard/rfc4862/catalog.md#rfc4862-ll-1) | selected | [Duplicate Address Detection of a router](../../protocol/nd/checks/autoconfiguration.md#duplicate-address-detection-of-a-router) | `Rfc4862RouterDad` | PASS | observation 1 held |
| [RFC4862-LL-2](../../standard/rfc4862/catalog.md#rfc4862-ll-2) | selected | [Link-local address and its Duplicate Address Detection](../../protocol/nd/checks/autoconfiguration.md#link-local-address-and-its-duplicate-address-detection) | `Rfc4862LinkLocalDad` | PASS | — |
| [RFC4862-LL-3](../../standard/rfc4862/catalog.md#rfc4862-ll-3) | owed | — | — | — | a later level 2 pass: an interface that fails and comes back, for the link-local address formed again |
| [RFC4862-LL-4](../../standard/rfc4862/catalog.md#rfc4862-ll-4) | owed | — | — | — | a later level 2 pass: an interface that fails and comes back, for the link-local address formed again |
| [RFC4862-LL-5](../../standard/rfc4862/catalog.md#rfc4862-ll-5) | selected | [Link-local address and its Duplicate Address Detection](../../protocol/nd/checks/autoconfiguration.md#link-local-address-and-its-duplicate-address-detection) | `Rfc4862LinkLocalDad` | PASS | — |
| [RFC4862-LL-6](../../standard/rfc4862/catalog.md#rfc4862-ll-6) | later | — | — | — | an interface identifier of another length; no mockup of this level has one |
| [RFC4862-LL-7](../../standard/rfc4862/catalog.md#rfc4862-ll-7) | later | — | — | — | level 4: state inside the node |
| [RFC4862-DAD-1](../../standard/rfc4862/catalog.md#rfc4862-dad-1) | selected | [Duplicate Address Detection of a router](../../protocol/nd/checks/autoconfiguration.md#duplicate-address-detection-of-a-router) | `Rfc4862RouterDad` | FAIL | fails: [gap 10](results.md#gap-10-defect--a-router-does-not-test-its-configured-address) |
| [RFC4862-DAD-2](../../standard/rfc4862/catalog.md#rfc4862-dad-2) | selected | [Link-local address and its Duplicate Address Detection](../../protocol/nd/checks/autoconfiguration.md#link-local-address-and-its-duplicate-address-detection) | `Rfc4862LinkLocalDad` | PASS | — |
| [RFC4862-DAD-3](../../standard/rfc4862/catalog.md#rfc4862-dad-3) | owed | — | — | — | a later level 2 pass: other values of DupAddrDetectTransmits |
| [RFC4862-DAD-4](../../standard/rfc4862/catalog.md#rfc4862-dad-4) | no check | — | — | — | the model declines anycast targets: "TODO: anycast target address handling is not implemented", `Ipv6NeighbourDiscovery.cc:2002` |
| [RFC4862-DAD-5](../../standard/rfc4862/catalog.md#rfc4862-dad-5) | selected | [Global address and its Duplicate Address Detection](../../protocol/nd/checks/autoconfiguration.md#global-address-and-its-duplicate-address-detection) | `Rfc4862GlobalAddressDad` | PASS | — |
| [RFC4862-DAD-6](../../standard/rfc4862/catalog.md#rfc4862-dad-6) | selected | [Link-local address and its Duplicate Address Detection](../../protocol/nd/checks/autoconfiguration.md#link-local-address-and-its-duplicate-address-detection) | `Rfc4862LinkLocalDad` | PASS | — |
| [RFC4862-DAD-7](../../standard/rfc4862/catalog.md#rfc4862-dad-7) | covered | [Duplicate link-local address](../../protocol/nd/checks/autoconfiguration.md#duplicate-link-local-address) | `Rfc4862DuplicateLinkLocal` | PASS | — |
| [RFC4862-DAD-8](../../standard/rfc4862/catalog.md#rfc4862-dad-8) | later | — | — | — | level 3: needs a crafted message |
| [RFC4862-DAD-9](../../standard/rfc4862/catalog.md#rfc4862-dad-9) | selected | [Link-local address and its Duplicate Address Detection](../../protocol/nd/checks/autoconfiguration.md#link-local-address-and-its-duplicate-address-detection) | `Rfc4862LinkLocalDad` | PASS | — |
| [RFC4862-DAD-10](../../standard/rfc4862/catalog.md#rfc4862-dad-10) | later | — | — | — | level 3: needs a crafted message |
| [RFC4862-DAD-11](../../standard/rfc4862/catalog.md#rfc4862-dad-11) | selected | [Groups joined before Duplicate Address Detection](../../protocol/nd/checks/multicast.md#groups-joined-before-duplicate-address-detection) | `Rfc4862GroupsBeforeDad` | FAIL | fails: [gap 9](results.md#gap-9-defect--a-node-does-not-join-its-solicited-node-groups) |
| [RFC4862-DAD-12](../../standard/rfc4862/catalog.md#rfc4862-dad-12) | selected | [Link-local address and its Duplicate Address Detection](../../protocol/nd/checks/autoconfiguration.md#link-local-address-and-its-duplicate-address-detection) | `Rfc4862LinkLocalDad` | PASS | — |
| [RFC4862-DAD-13](../../standard/rfc4862/catalog.md#rfc4862-dad-13) | selected | [Link-local address and its Duplicate Address Detection](../../protocol/nd/checks/autoconfiguration.md#link-local-address-and-its-duplicate-address-detection) | `Rfc4862LinkLocalDad` | PASS | — |
| [RFC4862-DAD-14](../../standard/rfc4862/catalog.md#rfc4862-dad-14) | later | — | — | — | level 4: a statistical test of the random delay; a check above tests the bound where one exists |
| [RFC4862-DAD-15](../../standard/rfc4862/catalog.md#rfc4862-dad-15) | later | — | — | — | level 4: a statistical test of the random delay; a check above tests the bound where one exists |
| [RFC4862-DAD-16](../../standard/rfc4862/catalog.md#rfc4862-dad-16) | covered | [Groups joined before Duplicate Address Detection](../../protocol/nd/checks/multicast.md#groups-joined-before-duplicate-address-detection) | `Rfc4862GroupsBeforeDad` | FAIL | not reached: [gap 9](results.md#gap-9-defect--a-node-does-not-join-its-solicited-node-groups) keeps the test from the observation |
| [RFC4862-DAD-17](../../standard/rfc4862/catalog.md#rfc4862-dad-17) | owed | — | — | — | a later level 2 pass: two nodes that test one address at the same time, or a node that sees its own solicitation, and a duplicate global address |
| [RFC4862-DAD-18](../../standard/rfc4862/catalog.md#rfc4862-dad-18) | covered | [Address resolution of a host](../../protocol/nd/checks/address-resolution.md#address-resolution-of-a-host) | `Rfc4861AddressResolutionHost` | PASS | — |
| [RFC4862-DAD-19](../../standard/rfc4862/catalog.md#rfc4862-dad-19) | owed | — | — | — | a later level 2 pass: two nodes that test one address at the same time, or a node that sees its own solicitation, and a duplicate global address |
| [RFC4862-DAD-20](../../standard/rfc4862/catalog.md#rfc4862-dad-20) | owed | — | — | — | a later level 2 pass: two nodes that test one address at the same time, or a node that sees its own solicitation, and a duplicate global address |
| [RFC4862-DAD-21](../../standard/rfc4862/catalog.md#rfc4862-dad-21) | owed | — | — | — | a later level 2 pass: two nodes that test one address at the same time, or a node that sees its own solicitation, and a duplicate global address |
| [RFC4862-DAD-22](../../standard/rfc4862/catalog.md#rfc4862-dad-22) | owed | — | — | — | a later level 2 pass: two nodes that test one address at the same time, or a node that sees its own solicitation, and a duplicate global address |
| [RFC4862-DAD-23](../../standard/rfc4862/catalog.md#rfc4862-dad-23) | owed | — | — | — | a later level 2 pass: two nodes that test one address at the same time, or a node that sees its own solicitation, and a duplicate global address |
| [RFC4862-DAD-24](../../standard/rfc4862/catalog.md#rfc4862-dad-24) | owed | — | — | — | a later level 2 pass: two nodes that test one address at the same time, or a node that sees its own solicitation, and a duplicate global address |
| [RFC4862-DAD-25](../../standard/rfc4862/catalog.md#rfc4862-dad-25) | selected | [Duplicate link-local address](../../protocol/nd/checks/autoconfiguration.md#duplicate-link-local-address) | `Rfc4862DuplicateLinkLocal` | PASS | — |
| [RFC4862-DAD-26](../../standard/rfc4862/catalog.md#rfc4862-dad-26) | covered | [Address resolution of a host](../../protocol/nd/checks/address-resolution.md#address-resolution-of-a-host) | `Rfc4861AddressResolutionHost` | PASS | — |
| [RFC4862-DAD-27](../../standard/rfc4862/catalog.md#rfc4862-dad-27) | selected | [Duplicate link-local address](../../protocol/nd/checks/autoconfiguration.md#duplicate-link-local-address) | `Rfc4862DuplicateLinkLocal` | PASS | — |
| [RFC4862-DAD-28](../../standard/rfc4862/catalog.md#rfc4862-dad-28) | later | — | — | — | level 4: state inside the node |
| [RFC4862-DAD-29](../../standard/rfc4862/catalog.md#rfc4862-dad-29) | selected | [Duplicate link-local address](../../protocol/nd/checks/autoconfiguration.md#duplicate-link-local-address) | `Rfc4862DuplicateLinkLocal` | PASS | — |
| [RFC4862-DAD-30](../../standard/rfc4862/catalog.md#rfc4862-dad-30) | later | — | — | — | a duplicate link-local address that does not come from the hardware address; no mockup of this level has one |
| [RFC4862-GLOB-1](../../standard/rfc4862/catalog.md#rfc4862-glob-1) | selected | [Global address and its Duplicate Address Detection](../../protocol/nd/checks/autoconfiguration.md#global-address-and-its-duplicate-address-detection) | `Rfc4862GlobalAddressDad` | PASS | — |
| [RFC4862-GLOB-2](../../standard/rfc4862/catalog.md#rfc4862-glob-2) | selected | [Global address and its Duplicate Address Detection](../../protocol/nd/checks/autoconfiguration.md#global-address-and-its-duplicate-address-detection) | `Rfc4862GlobalAddressDad` | PASS | — |
| [RFC4862-GLOB-3](../../standard/rfc4862/catalog.md#rfc4862-glob-3) | covered | [Router Advertisement header and addressing](../../protocol/nd/checks/router-discovery.md#router-advertisement-header-and-addressing) | `Rfc4861RaHeader` | PASS | — |
| [RFC4862-GLOB-4](../../standard/rfc4862/catalog.md#rfc4862-glob-4) | selected | [Router Solicitations on a link without a router](../../protocol/nd/checks/router-discovery.md#router-solicitations-on-a-link-without-a-router) | `Rfc4861RsNoRouter` | PASS | — |
| [RFC4862-GLOB-5](../../standard/rfc4862/catalog.md#rfc4862-glob-5) | selected | [Prefix with the Autonomous flag clear](../../protocol/nd/checks/autoconfiguration.md#prefix-with-the-autonomous-flag-clear) | `Rfc4862AutonomousFlagClear` | PASS | — |
| [RFC4862-GLOB-6](../../standard/rfc4862/catalog.md#rfc4862-glob-6) | later | — | — | — | level 3: needs a crafted message |
| [RFC4862-GLOB-7](../../standard/rfc4862/catalog.md#rfc4862-glob-7) | later | — | — | — | level 3: needs a crafted message |
| [RFC4862-GLOB-8](../../standard/rfc4862/catalog.md#rfc4862-glob-8) | selected | [Global address and its Duplicate Address Detection](../../protocol/nd/checks/autoconfiguration.md#global-address-and-its-duplicate-address-detection) | `Rfc4862GlobalAddressDad` | PASS | — |
| [RFC4862-GLOB-9](../../standard/rfc4862/catalog.md#rfc4862-glob-9) | later | — | — | — | level 3: needs a crafted message |
| [RFC4862-GLOB-10](../../standard/rfc4862/catalog.md#rfc4862-glob-10) | later | — | — | — | an interface identifier of another length; no mockup of this level has one |
| [RFC4862-GLOB-11](../../standard/rfc4862/catalog.md#rfc4862-glob-11) | selected | [Global address and its Duplicate Address Detection](../../protocol/nd/checks/autoconfiguration.md#global-address-and-its-duplicate-address-detection) | `Rfc4862GlobalAddressDad` | PASS | — |
| [RFC4862-GLOB-12](../../standard/rfc4862/catalog.md#rfc4862-glob-12) | owed | — | — | — | a later level 2 pass: a deprecated address with an alternative, and the renewal of a known prefix under the two-hour rule |
| [RFC4862-GLOB-13](../../standard/rfc4862/catalog.md#rfc4862-glob-13) | owed | — | — | — | a later level 2 pass: a deprecated address with an alternative, and the renewal of a known prefix under the two-hour rule |
| [RFC4862-GLOB-14](../../standard/rfc4862/catalog.md#rfc4862-glob-14) | owed | — | — | — | a later level 2 pass: a deprecated address with an alternative, and the renewal of a known prefix under the two-hour rule |
| [RFC4862-GLOB-15](../../standard/rfc4862/catalog.md#rfc4862-glob-15) | owed | — | — | — | a later level 2 pass: a deprecated address with an alternative, and the renewal of a known prefix under the two-hour rule |
| [RFC4862-GLOB-16](../../standard/rfc4862/catalog.md#rfc4862-glob-16) | owed | — | — | — | a later level 2 pass: a deprecated address with an alternative, and the renewal of a known prefix under the two-hour rule |
| [RFC4862-GLOB-17](../../standard/rfc4862/catalog.md#rfc4862-glob-17) | owed | — | — | — | a later level 2 pass: a deprecated address with an alternative, and the renewal of a known prefix under the two-hour rule |
| [RFC4862-GLOB-18](../../standard/rfc4862/catalog.md#rfc4862-glob-18) | owed | — | — | — | a later level 2 pass: a deprecated address with an alternative, and the renewal of a known prefix under the two-hour rule |
| [RFC4862-GLOB-19](../../standard/rfc4862/catalog.md#rfc4862-glob-19) | owed | — | — | — | a later level 2 pass: a deprecated address with an alternative, and the renewal of a known prefix under the two-hour rule |
| [RFC4862-GLOB-20](../../standard/rfc4862/catalog.md#rfc4862-glob-20) | owed | — | — | — | a later level 2 pass: a deprecated address with an alternative, and the renewal of a known prefix under the two-hour rule |
| [RFC4862-GLOB-21](../../standard/rfc4862/catalog.md#rfc4862-glob-21) | owed | — | — | — | a later level 2 pass: a deprecated address with an alternative, and the renewal of a known prefix under the two-hour rule |
| [RFC4862-GLOB-22](../../standard/rfc4862/catalog.md#rfc4862-glob-22) | selected | [Address after its valid lifetime](../../protocol/nd/checks/autoconfiguration.md#address-after-its-valid-lifetime) | `Rfc4862AddressLifetime` | FAIL | fails: [gap 11](results.md#gap-11-defect--an-expired-address-stays-in-use) |
| [RFC4862-GLOB-23](../../standard/rfc4862/catalog.md#rfc4862-glob-23) | selected | [Address after its valid lifetime](../../protocol/nd/checks/autoconfiguration.md#address-after-its-valid-lifetime) | `Rfc4862AddressLifetime` | FAIL | fails: [gap 11](results.md#gap-11-defect--an-expired-address-stays-in-use) |
| [RFC4862-GLOB-24](../../standard/rfc4862/catalog.md#rfc4862-glob-24) | later | — | — | — | level 3: needs a crafted message |
| [RFC4862-CONS-1](../../standard/rfc4862/catalog.md#rfc4862-cons-1) | no check | — | — | — | the model has no DHCPv6 client; the flags themselves are checked in Router Advertisement fields |
| [RFC4862-CONS-2](../../standard/rfc4862/catalog.md#rfc4862-cons-2) | owed | — | — | — | a later level 2 pass: three or more routers, for default router selection, and two routers that advertise different values of one parameter |
| [RFC4862-CONS-3](../../standard/rfc4862/catalog.md#rfc4862-cons-3) | later | — | — | — | level 4: state inside the node |

### RFC 5942

| Statement | Status | Check | Test | Verdict | Note |
| --- | --- | --- | --- | --- | --- |
| [RFC5942-ONLINK-1](../../standard/rfc5942/catalog.md#rfc5942-onlink-1) | selected | [Prefix with the on-link flag clear](../../protocol/nd/checks/on-link.md#prefix-with-the-on-link-flag-clear) | `Rfc5942OnLinkFlagClear` | PASS | — |
| [RFC5942-ONLINK-2](../../standard/rfc5942/catalog.md#rfc5942-onlink-2) | selected | [On-link neighbor reached directly](../../protocol/nd/checks/on-link.md#on-link-neighbor-reached-directly) | `Rfc4861OnLinkNeighbor` | PASS | — |
| [RFC5942-ONLINK-3](../../standard/rfc5942/catalog.md#rfc5942-onlink-3) | selected | [Prefix after its valid lifetime](../../protocol/nd/checks/on-link.md#prefix-after-its-valid-lifetime) | `Rfc5942PrefixLifetime` | PASS | — |
| [RFC5942-ONLINK-4](../../standard/rfc5942/catalog.md#rfc5942-onlink-4) | owed | — | — | — | a later level 2 pass: a router that changes the L flag of a prefix, advertises a prefix with Valid Lifetime zero, or refreshes it; a prefix length other than 64; a router whose lifetime runs out |
| [RFC5942-ONLINK-5](../../standard/rfc5942/catalog.md#rfc5942-onlink-5) | selected | [No router and no on-link prefix](../../protocol/nd/checks/on-link.md#no-router-and-no-on-link-prefix) | `Rfc5942NoRouterNoOnLink` | PASS | — |
| [RFC5942-ONLINK-6](../../standard/rfc5942/catalog.md#rfc5942-onlink-6) | selected | [No router and no on-link prefix](../../protocol/nd/checks/on-link.md#no-router-and-no-on-link-prefix) | `Rfc5942NoRouterNoOnLink` | PASS | — |
| [RFC5942-ONLINK-7](../../standard/rfc5942/catalog.md#rfc5942-onlink-7) | later | — | — | — | level 4: the report goes to the application inside the host, and no link carries it |
| [RFC5942-ONLINK-8](../../standard/rfc5942/catalog.md#rfc5942-onlink-8) | selected | [Prefix with the on-link flag clear](../../protocol/nd/checks/on-link.md#prefix-with-the-on-link-flag-clear) | `Rfc5942OnLinkFlagClear` | PASS | — |
| [RFC5942-ONLINK-9](../../standard/rfc5942/catalog.md#rfc5942-onlink-9) | later | — | — | — | level 3: needs a crafted message |
| [RFC5942-ONLINK-10](../../standard/rfc5942/catalog.md#rfc5942-onlink-10) | later | — | — | — | level 3: needs a crafted message |

### RFC 6980

| Statement | Status | Check | Test | Verdict | Note |
| --- | --- | --- | --- | --- | --- |
| [RFC6980-FRAG-1](../../standard/rfc6980/catalog.md#rfc6980-frag-1) | selected | [Redirect of a large packet](../../protocol/nd/checks/redirect.md#redirect-of-a-large-packet) | `Rfc6980RedirectLargePacket` | PASS | observation 2 held |
| [RFC6980-FRAG-2](../../standard/rfc6980/catalog.md#rfc6980-frag-2) | later | — | — | — | level 3: needs a crafted message |
| [RFC6980-FRAG-3](../../standard/rfc6980/catalog.md#rfc6980-frag-3) | later | — | — | — | level 3: needs a crafted message |
| [RFC6980-FRAG-4](../../standard/rfc6980/catalog.md#rfc6980-frag-4) | later | — | — | — | level 3: needs a crafted message |
| [RFC6980-FRAG-5](../../standard/rfc6980/catalog.md#rfc6980-frag-5) | later | — | — | — | level 3: needs a crafted message |
| [RFC6980-FRAG-6](../../standard/rfc6980/catalog.md#rfc6980-frag-6) | later | — | — | — | level 3: needs a crafted message |

## The coverage debt: the checks this pass owes

Eighty-eight statements that the model claims have no check yet. Each needs a normal exchange
only, in a larger mockup or with a second path of messages, so each is level 2 work for a later
pass. The closing list of
[`checks.md`](../../protocol/nd/checks.md#statements-this-pass-wrote-no-check-for) holds the
same needs in the words of the standard.

| What the check needs | Statements |
| --- | --- |
| a router variable of zero, which leaves the host value as it is, and a lifetime of all one bits, which is infinity | RFC4861-RA-10, RA-20, RA-22, OPT-25, OPT-27, HOST-11 |
| a router with Router Lifetime zero that alone advertises a prefix | RFC4861-RA-18 |
| a change of the link-layer address of a node, for unsolicited advertisements | RFC4861-NA-5, NA-22, AR-54, AR-57, AR-58, AR-59, AR-60, AR-61, AR-62, AR-65, AR-66, AR-67, AR-68 |
| a proxy | RFC4861-NA-17, AR-36, ANY-5, ANY-6, ANY-7, ANY-8, ANY-9, ANY-10 |
| a router that changes the L flag of a prefix, advertises a prefix with Valid Lifetime zero, or refreshes it; a prefix length other than 64; a router whose lifetime runs out | RFC4861-OPT-20, HOST-27, HOST-31, HOST-32, HOST-33, HOST-35, HOST-37, HOST-38, RFC5942-ONLINK-4 |
| a deprecated address with an alternative, and the renewal of a known prefix under the two-hour rule | RFC4861-OPT-26, RFC4862-GLOB-12, GLOB-13, GLOB-14, GLOB-15, GLOB-16, GLOB-17, GLOB-18, GLOB-19, GLOB-20, GLOB-21 |
| a check of each router variable; the model has no parameter for AdvReachableTime and AdvRetransTimer, see [results.md](results.md#other-findings) | RFC4861-RCFG-1 |
| an interface that does not advertise | RFC4861-RCFG-4, ADV-1 |
| a router that stops advertising, changes its link-local address or becomes a host, and two routers with inconsistent advertisements | RFC4861-ADV-2, ADV-24, ADV-25, ADV-26, ADV-27, ADV-28, ADV-42, ADV-43, ADV-44, ADV-45, ADV-47, HOST-14, AR-56 |
| a Router Advertisement larger than the MTU, which a router splits and does not fragment | RFC4861-ADV-20 |
| three or more routers, for default router selection, and two routers that advertise different values of one parameter | RFC4861-HOST-9, HOST-10, HOST-15, HOST-40, HOST-41, RFC4862-CONS-2 |
| an address with an interface identifier of its own, which has its own solicited-node group, and the removal of an address | RFC4861-AR-5, AR-7 |
| a burst of packets to one neighbor that is not resolved yet, a stream of packets that causes Redirects, for the rate limit, and two flows to one destination with different Flow Labels | RFC4861-AR-16, AR-17, RDR-9, RDH-9 |
| a check with another value of DupAddrDetectTransmits, which the configurator offers as `dupAddrDetectTransmits` | RFC4862-CONF-2 |
| an interface that fails and comes back, for the link-local address formed again | RFC4862-LL-3, LL-4 |
| other values of DupAddrDetectTransmits | RFC4862-DAD-3 |
| two nodes that test one address at the same time, or a node that sees its own solicitation, and a duplicate global address | RFC4862-DAD-17, DAD-19, DAD-20, DAD-21, DAD-22, DAD-23, DAD-24 |

One of them will fail when its check exists, from what the code shows: RFC4861-RCFG-1, because
two router variables have no parameter
([results.md](results.md#other-findings)).

## Feature support

The rule of the guide, per feature: `supported` when every core check ran and passed,
`partial` when at least one passed and at least one failed or has no check, `not supported`
when every core check that ran failed, `untested` when no core check exists.

| Feature | Level | Support | Core statements that fail or have no check |
| --- | --- | --- | --- |
| [ND-F-MESSAGE-FORMAT](../../protocol/nd/features.md#nd-f-message-format) | mandatory | supported | — |
| [ND-F-OPTIONS](../../protocol/nd/features.md#nd-f-options) | mandatory | supported | — |
| [ND-F-MULTICAST-GROUPS](../../protocol/nd/features.md#nd-f-multicast-groups) | mandatory | partial | RFC4861-AR-4 fails ([gap 9](results.md#gap-9-defect--a-node-does-not-join-its-solicited-node-groups)); RFC4861-AR-5 is `owed`; RFC4861-HOST-8 is `later` |
| [ND-F-ROUTER-SOLICITATION](../../protocol/nd/features.md#nd-f-router-solicitation) | optional | partial | RFC4861-HOST-47 is `later` |
| [ND-F-ROUTER-ADVERTISEMENT](../../protocol/nd/features.md#nd-f-router-advertisement) | mandatory | partial | RFC4861-ADV-7, ADV-11, ADV-29 fail ([gap 2](results.md#gap-2-defect--two-defaults-of-the-router-are-wrong), [gap 5](results.md#gap-5-defect--the-router-answers-only-its-first-solicitation)); RFC4861-ADV-1 is `owed` |
| [ND-F-ADVERTISEMENT-TIMING](../../protocol/nd/features.md#nd-f-advertisement-timing) | mandatory | partial | RFC4861-ADV-23, ADV-32 fail ([gap 5](results.md#gap-5-defect--the-router-answers-only-its-first-solicitation), [gap 6](results.md#gap-6-defect--the-first-periodic-advertisement-comes-after-198-to-600-seconds)); RFC4861-ADV-22, ADV-36 are not reached ([gap 6](results.md#gap-6-defect--the-first-periodic-advertisement-comes-after-198-to-600-seconds)) |
| [ND-F-ROUTER-DISCOVERY](../../protocol/nd/features.md#nd-f-router-discovery) | mandatory | partial | RFC4861-RA-17 is not reached ([gap 7](results.md#gap-7-defect--a-host-drops-every-advertisement-during-duplicate-address-detection)); RFC4861-HOST-14, HOST-37 are `owed`; RFC4861-HOST-13 is `later` |
| [ND-F-PARAMETER-DISCOVERY](../../protocol/nd/features.md#nd-f-parameter-discovery) | optional | not supported | RFC4861-HOST-16, HOST-19, HOST-24 fail ([gap 1](results.md#gap-1-defect--the-hop-limit-of-a-host-is-a-constant), [gap 3](results.md#gap-3-defect--the-host-fragments-to-the-mtu-of-the-interface-not-to-the-advertised-mtu), [gap 4](results.md#gap-4-defect--address-resolution-waits-a-constant-not-retranstimer)); RFC4861-HOST-17 is `later` |
| [ND-F-ON-LINK-DETERMINATION](../../protocol/nd/features.md#nd-f-on-link-determination) | mandatory | partial | RFC4861-HOST-31, HOST-32 are `owed`; RFC5942-ONLINK-9, ONLINK-10 are `later` |
| [ND-F-ADDRESS-RESOLUTION](../../protocol/nd/features.md#nd-f-address-resolution) | mandatory | partial | RFC4861-AR-45 is `later` |
| [ND-F-RESOLUTION-FAILURE](../../protocol/nd/features.md#nd-f-resolution-failure) | mandatory | supported | — |
| [ND-F-UNSOLICITED-ADVERTISEMENT](../../protocol/nd/features.md#nd-f-unsolicited-advertisement) | optional | untested | RFC4861-AR-57, AR-58, AR-59, AR-60, AR-61 are `owed` |
| [ND-F-ANYCAST-AND-PROXY](../../protocol/nd/features.md#nd-f-anycast-and-proxy) | optional | untested | RFC4861-ANY-8 is `owed`; RFC4861-ANY-2, ANY-3 are `no check` |
| [ND-F-REDIRECT](../../protocol/nd/features.md#nd-f-redirect) | optional | supported | — |
| [ND-F-REDIRECT-PROCESSING](../../protocol/nd/features.md#nd-f-redirect-processing) | optional | partial | RFC4861-RDH-3, RDH-7 are `later` |
| [ND-F-MESSAGE-VALIDATION](../../protocol/nd/features.md#nd-f-message-validation) | mandatory | untested | RFC4861-RVAL-2, RVAL-10, RVAL-12, RDVAL-1, RDVAL-3, RDVAL-7, OPT-6 are `later` |
| [ND-F-ROUTER-ROLE-CHANGE](../../protocol/nd/features.md#nd-f-router-role-change) | optional | untested | RFC4861-ADV-25, ADV-27, ADV-28 are `owed` |
| [ND-F-ROUTER-CONSISTENCY](../../protocol/nd/features.md#nd-f-router-consistency) | optional | untested | RFC4861-ADV-42, ADV-43 are `owed` |
| [ND-F-LINK-LOCAL-ADDRESS](../../protocol/nd/features.md#nd-f-link-local-address) | mandatory | partial | RFC4862-LL-7 is `later` |
| [ND-F-DUPLICATE-ADDRESS-DETECTION](../../protocol/nd/features.md#nd-f-duplicate-address-detection) | mandatory | partial | RFC4862-DAD-11 fails ([gap 9](results.md#gap-9-defect--a-node-does-not-join-its-solicited-node-groups)); RFC4862-DAD-20, DAD-21 are `owed` |
| [ND-F-STATELESS-AUTOCONFIGURATION](../../protocol/nd/features.md#nd-f-stateless-autoconfiguration) | mandatory | partial | RFC4862-GLOB-12, GLOB-13, GLOB-15 are `owed`; RFC4862-GLOB-6, GLOB-7, GLOB-9 are `later` |
| [ND-F-ADDRESS-LIFETIME](../../protocol/nd/features.md#nd-f-address-lifetime) | mandatory | not supported | RFC4862-GLOB-22, GLOB-23 fail ([gap 11](results.md#gap-11-defect--an-expired-address-stays-in-use)); RFC4862-GLOB-16, GLOB-18 are `owed`; RFC4862-GLOB-24 is `later` |
| [ND-F-NO-FRAGMENTATION](../../protocol/nd/features.md#nd-f-no-fragmentation) | mandatory | partial | RFC6980-FRAG-2, FRAG-3, FRAG-4, FRAG-5, FRAG-6 are `later` |

Four features are supported, twelve partial, two not supported, and five untested. The two
features that are not supported are ND-F-PARAMETER-DISCOVERY, whose three core checks fail
on gaps 1, 3 and 4, and ND-F-ADDRESS-LIFETIME, whose two core checks fail on gap 11.

## Achieved level

**Level 2, reached.** Target: level 2, from
[`standards.md`](../../protocol/nd/standards.md#target-level).

The exit criterion of level 2 has two halves, and both hold:

| Half of the criterion | State | Evidence |
| --- | --- | --- |
| Every normal-path mandatory mechanism of the base documents appears as a feature | holds | the catalogs hold every normative statement of RFC 4861 §4, §6, §7.2 and §8, RFC 4862 §5, RFC 5942 §4 and §6 and RFC 6980 §5, 489 entries; the feature map has 23 features and places every entry |
| Every mandatory feature has a core check that ran and has a verdict | holds for the normal path | 14 of the 15 mandatory features have core checks that ran: 37 tests, 22 PASS, 15 FAIL |

The one mandatory feature without a core check has no normal path, so the criterion does not
reach it at this level, and the ledger says so rather than count it: ND-F-MESSAGE-VALIDATION is
the silent discard of a message that fails a validity check, and every such message is a
crafted one, which is level 3.

A level is a measure of how deeply the pass looked, not of how well the model did. Fifteen
tests fail, and eleven gaps of the model stand behind them; the level holds because each of
those checks ran.

| Level | State | What it needs |
| --- | --- | --- |
| 1, Survey | **reached** | — |
| 2, Core | **reached** | the 88 `owed` statements are the debt of the level; see [the coverage debt](#the-coverage-debt-the-checks-this-pass-owes) |
| 3, Edge | not started | the catalog half holds; the checks need crafted messages: the validation of every message, the receiver halves of the reserved fields and of the options, a fragmented ND message |
| 4, Dynamics | partial | the delays and intervals with a bound have a check with a stated window, but the random halves, the random ReachableTime, Neighbor Unreachability Detection and the state of the Neighbor Cache have no test |
| 5, Complete | not started | RFC 4429, RFC 7527, RFC 8028, RFC 9131 and the other documents of the register, and anycast, which the model declines |

Per feature, the level reached is level 2 for the eighteen features with a core check that ran,
level 1 for ND-F-MESSAGE-VALIDATION, whose checks are level 3, and level 1 for the four other
untested features, whose checks are owed or declined.

## Pass log

| Pass | Date | Level | Scope | Result |
| --- | --- | --- | --- | --- |
| 1 | 2026-09-23 | **1, reached** | RFC 4861, RFC 4862, RFC 5942 and RFC 6980 downloaded; the standards map; the claims of the model | no run; two obsolete claims (RFC 2461, RFC 2462), one claim outside the in-scope set (RFC 4429), one declined document with a stated reason (RFC 7527) |
| 2 | 2026-09-24 | **2, reached** | catalogs of RFC 4861 (403), RFC 4862 (70), RFC 5942 (10) and RFC 6980 (6); 23 features; 34 checks; 37 tests; the conformance matrix | 22 PASS, 15 FAIL, none declared; eleven gaps of the model; 88 statements owed |

The read record of pass 1 named commit `e360ca980e` of the wave 0 branch; its trees, src
`16dc528e10` and tests/protocol `6f0a6bdb05`, identify the code it read.

## Out of scope

The areas that the catalogs leave out, and why, are at the end of each catalog:
[RFC 4861](../../standard/rfc4861/catalog.md#out-of-scope-in-this-catalog),
[RFC 4862](../../standard/rfc4862/catalog.md#out-of-scope-in-this-catalog),
[RFC 5942](../../standard/rfc5942/catalog.md#out-of-scope-in-this-catalog),
[RFC 6980](../../standard/rfc6980/catalog.md#out-of-scope-in-this-catalog). The documents
outside the in-scope set are in [`standards.md`](../../protocol/nd/standards.md#in-scope-set).

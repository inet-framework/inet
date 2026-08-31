# Release rules

> **Kind:** rule · **Status:** current · **Seal:** by rule · **Owns:** `RR-*` · **Stands on:** [accepted-requirements.md](../requirement/accepted-requirements.md), [testing.md](testing.md)

What a release owes its users. INET is a model library that other people's published results depend
on: a simulation run against version 4.5 must still be reproducible after 4.6 ships, or the result
was never a result. These rules are the enforcement home of
[R-DIST-COMPAT](../requirement/accepted-requirements.md#r-dist-compat).

A rule has a stable identifier `RR-<AREA>`, and the identifier is the heading:
[RR-BREAK-MIGRATE](release.md#rr-break-migrate).

## Index

| Rule | Statement |
| --- | --- |
| [RR-VERSION](#rr-version) | A version number says what kind of change happened |
| [RR-KERNEL-VERSION](#rr-kernel-version) | Every release states the kernel version it needs |
| [RR-BREAK-MIGRATE](#rr-break-migrate) | A breaking change ships with its migration instructions |
| [RR-DEPRECATE-FIRST](#rr-deprecate-first) | A removal is deprecated for one release before it happens |
| [RR-WHATSNEW](#rr-whatsnew) | A user-visible change earns a WHATSNEW entry, written for a user |
| [RR-FINGERPRINT-NOTE](#rr-fingerprint-note) | A release that moves a trajectory says so |
| [RR-FEATURE-STABLE](#rr-feature-stable) | A feature identifier is part of the interface |
| [RR-RELEASE-GREEN](#rr-release-green) | A release is cut from a green tree with every feature built |

## The rules

### RR-VERSION

**The version number says what kind of change happened, and a user can tell from it whether their
work will still run.**

A patch release repairs defects and moves no trajectory that was correct. A minor release adds
models, parameters and features, and does not remove or rename anything. A major release may break,
and then every other rule in this document applies. The number is a promise, not a marketing choice.

*Enforced at T5 — human judgment at the release decision.*

### RR-KERNEL-VERSION

**Every release states the minimum OMNeT++ version it needs, and the versions it was tested on.**

INET does not own its kernel ([D-KERNEL](../design/decisions.md#d-kernel)), so the pairing is part of
what a release is. A user who cannot tell which kernel to install cannot install INET, and a result
whose kernel version is unknown is not reproducible.

*Enforced at T3 — the build checks the kernel version; T2 by the CI matrix.*

### RR-BREAK-MIGRATE

**A change that breaks an existing model ships with the instructions to repair it, in the same
release.**

Name what broke, what it becomes, and the mechanical steps. Where the change can be scripted, ship
the script — `_scripts/migrate/` exists for this. The migration guide under
`doc/src/migration-guide/` is the user-facing home; a breaking change that reaches a release without
an entry there is an unannounced break, and it will be found by a user in the middle of a study.

*Enforced at T4 — agent review of the release diff; T5 for whether the guidance is enough.*

### RR-DEPRECATE-FIRST

**A name, a parameter or a module is deprecated in one release before it is removed in the next.**

The deprecation says what to use instead and still works. This gives a user one release cycle to
move, with their own tests still passing, instead of a version bump that stops their work. A removal
that skips the deprecation step needs a stated reason — that it was never usable, or that keeping it
is actively harmful.

*Enforced at T3 — a deprecation attribute the compiler warns on; T4 for the removal review.*

### RR-WHATSNEW

**A user-visible change earns a `WHATSNEW` entry, written for the user and not for the reviewer.**

The entry says what a user can now do, or what changed under them — not which files moved. A reader
of `WHATSNEW` is deciding whether to upgrade, and what to check after they do. An entry that reads
like a commit subject does not help them decide.

*Enforced at T4 — agent review: does the release have an entry for every user-visible change?*

### RR-FINGERPRINT-NOTE

**A release that deliberately moves a simulation trajectory says which models moved and why.**

This is the release-scale form of [TR-BASELINE-PROVENANCE](testing.md#tr-baseline-provenance). A user
whose results differ after an upgrade must be able to find out, in one place, whether their model was
one of the ones that changed on purpose. Without this note the only honest answer to *why are my
numbers different?* is *nobody knows*.

*Enforced at T4 — agent review against the baseline commits in the release range.*

### RR-FEATURE-STABLE

**A feature identifier is part of the published interface, and renaming one is a breaking change.**

A feature id becomes a `-DINET_WITH_*` compile flag, a `.oppfeaturestate` entry and a line in a
user's build script. `NV-14` in [audit/naming-exceptions.md](../audit/naming-exceptions.md) is the
live example: four feature ids break the naming rules, and repairing them changes build flags, so the
rename must be coordinated with a release rather than done quietly.

*Enforced at T3 — `inet_featuretool`; T5 for the coordination.*

### RR-RELEASE-GREEN

**A release is cut from a tree where every test category passes and every feature builds.**

Not "green except for the known failures". A known failure that ships is a failure that a user will
find, and that the next contributor will assume is normal. If a failure is genuinely acceptable, it
is either repaired, disabled with an issue number under [TR-FLAKY](testing.md#tr-flaky), or the
release waits.

*Enforced at T2 — the full workflow set on the release commit.*

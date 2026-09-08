# Interface sweep — every C++ class named `I<Stem>`

> **Kind:** report · **Status:** snapshot 2026-09-07 · **Seal:** none · **Owns:** — · **Stands on:** [rule/naming.md](../../../rule/naming.md), [rule/architecture.md](../../../rule/architecture.md)

- **Scope:** every C++ class declared `class [INET_API] I<Capital>…` under `src/inet/`, generated
  `*_m.h` excluded — 226 classes.
- **Date:** 2026-09-07, commit `5b65a0023d`
- **Command:** `doc/project/enforcement/check-interfaces.sh --verbose`
- **Rules checked:** [NR-CPP-TYPE](../../../rule/naming.md#nr-cpp-type) (the `I` prefix is a
  promise), [AR-ORG-CONTRACT-PURITY](../../../rule/architecture.md#ar-org-contract-purity) (what the
  promise means: no method body).
- **Result:** **FAIL — 16 of 226 break the promise; 210 keep it; 9 carry a note.** 14 of the 16
  are architecture violations (`AV-CONTRACT-01…03`) and 2 are naming violations. A third naming
  violation, `IRadioSignal`, is one of the 9 notes: the gate cannot tell an enum holder over a
  non-interface from a marker that inherits its role, so it points a reviewer at it, and the reviewer
  — this sweep — decided it (`NV-19`). Seven further hits were sanctioned (`AS-03`, `AS-04`) and count
  as clean.

The rule was written down on 2026-09-07 and this sweep ran the same day, to find out whether it
describes INET or asks it to change. **It describes INET, 92 % of the way.** The user's recollection
— *"mostly followed, with some very small exceptions"* — is exact.

## What the rule is

A C++ class named `I<Stem>` is an interface. It holds pure virtual declarations, a virtual
destructor, signal identities, and type declarations — and **no method body**. Not a no-op default,
not a one-line forwarder, not a convenience. A default is not a small exception: it is the failure
mode. `IIndicatorFigure::getNumSeries() { return 1; }` was renamed to `getNumItems()` in
[#1125](../pull-request/pr-1125.md) with the body kept, and every implementation outside INET went on
compiling with its override silently never called again. A pure virtual would have made each one a
compile error.

Where a default belongs is `<Stem>Base`. INET pairs 58 of its interfaces with one already, and has
206 `*Base` classes. **There is no `*Default` suffix in INET** — the sweep looked — and the rule now
says not to introduce one: the Base *is* the default.

## The gate found its own faults before the code's

The first run reported 29. Four of those were the gate misreading C++, and a fifth was a rule the
gate cannot apply. Each is recorded here because the next gate will make the same mistakes:

| The gate read | as | It was |
| --- | --- | --- |
| the enumerators of `enum SourceType { MANUAL, … }` in `IRoute` | 17 data members | one type declaration |
| the fields of the nested `struct InInterface` in `IMulticastRoute`, and `struct Notification` in `IArp` | data members and non-virtual functions of the interface | members of a nested type, which the rule allows |
| the statements inside a multi-line default body in `IPrintableObject` | 8 data members and 12 non-virtual functions | one body, counted once on its opening line |
| the `~ICallback() {}` of a class nested in `IContention` | a non-virtual function of `IContention` | the nested class's own destructor |

A member body and a nested type are now opaque to the gate: their lines belong to them. A finding
without its line is not a finding, so `--verbose` prints the offending declaration under each hit.

The fifth: a body that holds only type declarations and no pure virtual of its own. `INetworkProtocol`
and `IPhysicalLayer` are that shape and are interfaces — they inherit their role and add a `typedef`.
`IRadioSignal` is that shape and is not — it extends `IPrintableObject`, which is itself `NV-19`.
Telling them apart means resolving base classes across files, which a line scanner does not do. So
the gate reports the shape as a **note** and leaves the decision to a reviewer, which is what the
Note severity in [audit/README.md](../../README.md) is for. Nine classes carry it; one of them is
`NV-19` by this reviewer's reading, and the other eight are markers.

## The findings, sorted

### Not interfaces at all — `NV-19`

Three classes carry the prefix and have no pure virtual. The name says the opposite of what the class
is.

| Class | What it is | Reads true as |
| --- | --- | --- |
| `IPrintableObject` | a mixin: eight virtuals, every one with a body | `PrintableObjectMixin` |
| `IScrambling` | a value type: two accessors | `Scrambling` |
| `IRadioSignal` | an enum holder over `IPrintableObject` — **a gate note, decided by review**: it has no pure virtual and its only base is not an interface | `RadioSignalPart`, or fold the enum into `ISignal` |

`IPrintableObject` is the one that matters: dozens of classes extend it.

### Default bodies — `AV-CONTRACT-01`, the hazard itself

| Class | Bodies | Where the default goes |
| --- | --- | --- |
| `IIndicatorFigure` | `getNumSeries() { return 1; }`, `refreshDisplay() {}` | an `IndicatorFigureBase` — none exists. **The live case**: see [#1125 F-2](../pull-request/pr-1125.md). |
| `SctpSocket::ICallback` | **12** no-op defaults, and `socketOptionsArrived() { delete indication; }` | a `SctpSocketCallbackBase` — none exists |
| `TcpSocket::ICallback` | `socketIcmpv4Error() {}`, `socketIcmpv6Error() {}` — the comment says *"backward compatible"* | `TcpAppBase`, which already implements the interface; five direct implementors would then get a compile error on each, which is the point |
| `ICongestionController` | `readParameters() {}`, `setStatistics() {}`, `setPath() {}` | a `CongestionControllerBase` — none exists |
| `QuicSocket::ICallback` | `socketNewToken() {}` | as for TCP |
| `IResolver` | `resolveExpression() { return ""; }` | a `ResolverBase` — none exists |

### A body the Base could take today — `AV-CONTRACT-02`

| Class | Body | The Base |
| --- | --- | --- |
| `IFunction` | `printOn()` | `FunctionBase` **exists** |
| `IIeee80211Band` | `printToStream()` | `Ieee80211BandBase` **exists** |
| `ITransmitStep`, `IReceiveStep` | `getType() { return TRANSMIT; }` / `RECEIVE` | a constant per kind; implement it in the concrete steps |

Three small commits.

### State in an interface — `AV-CONTRACT-03`

| Class | State |
| --- | --- |
| `IRadio`, `ITransmission` | `uint64_t& nextId = SIMULATION_SHARED_COUNTER(nextId)` — `RadioBase` and `TransmissionBase` both exist |
| `IMessageHandler` | `Router *router`, set by a constructor argument |
| `IScheduler` | `streamMap`, set by a constructor argument |

An interface with a constructor argument is a base class wearing the wrong name.

### Sanctioned — `AS-03`, `AS-04`

**`AS-03` — a body that computes only from the class's own pure virtuals.** The rule forbids a body
because a default body hides a break. A body whose only inputs are the class's own pure virtuals
cannot: rename the pure virtual and the body stops compiling. Six classes:

| Class | Body | Computes from |
| --- | --- | --- |
| `L3Socket::ICallback`, `Ipv4Socket::ICallback`, `Ipv6Socket::ICallback` | `socketDataArrived(INetworkSocket*)` | the pure `socketDataArrived(Ipv4Socket*)` — a narrowing shim |
| `IPacketComparatorFunction` | `less(cObject*, cObject*)` | the pure `comparePackets()` |
| `IL3AddressType` | `getAddressByteLength()` | the pure `getAddressBitLength()`, `+7 / 8` |
| `IIeee80211Mode` | `_getPreambleMode()` and two more | `const_cast` over the pure getters — **also `NV-08`**, the leading underscore |

The gate lists them by name; the shape is not cheap to prove by regular expression.

**`AS-04` — a named scalar constant of the contract's vocabulary.** `IEigrpPdm::UNSPEC_RECEIVER = 0`
and four more. A `static const int NAME = literal` is an enum value spelled differently. The gate
accepts the shape.

## What the sweep does not cover

- **NED `moduleinterface`** — 225 of them. A NED interface cannot carry a body, so the rule holds by
  construction there; what it *can* carry is a parameter default, and whether that is the same hazard
  is a question for another day.
- **A helper hiding under a non-`I` name.** `AR-ORG-CONTRACT-PURITY` still needs `T4` review for a
  contract that does not announce itself.
- **`IPsec` and `IPsecRule`** — excluded by name, under `NS-01`. They are not interfaces and the `I`
  is not the prefix; it is the acronym.

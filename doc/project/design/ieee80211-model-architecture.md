# IEEE 802.11 model architecture

> **Kind:** design · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [ieee80211.md](../domain/ieee80211.md), [architecture.md](../rule/architecture.md)

This document defines the kinds of IEEE 802.11 state, their ownership and lifetime, and the
communication contracts between components. It describes the architecture, not an inventory of
implemented PHY features.

**1. State has four meanings.**

| Kind | Meaning | Owner and writer | Readers |
|---|---|---|---|
| **Catalog** | Immutable mode definitions, rates, timing, and physical legality | Mode definitions have no runtime writer. Configuration selects the catalog; the MAC exposes that selection through a read-only provider. | Capability assembly, MAC timing, rate selection, PHY |
| **Capability** | What the configured implementation can transmit and receive | Each PHY/MAC component supplies its abilities through a typed contract. Initialization assembles the shared profile in the MIB. | Management, peer-capability derivation, feature procedures |
| **Control** | Chosen policy, enabled features, and operating parameters | The component making the decision: management for BSS policy/operation, PHY for radio control, algorithms for their policies | Components executing those decisions |
| **Status** | What has happened, been learned, or is currently effective | The component observing or completing the event: management for peers/association, PHY for radio state, algorithms for measurements | Management decisions, selection, and other declared consumers |

A catalog entry does not prove device support. An advertisement derives from capability and enabled
policy. A received advertisement is learned status, not access to the peer's actual implementation.
Status may influence control without changing intrinsic capability. Requested and effective settings
remain distinct when a procedure applies them at different times.

Classification follows meaning and role, not the information element carrying a value. The AP chooses
its BSS operation; a STA stores what it has accepted about that operation. These are different local
facts and need not change simultaneously.

**2. The decision owner controls changes; the MIB stores shared protocol facts.**

Management controls BSS and peer relationships. The MIB stores their committed view and enforces
structural consistency; it does not choose admission or channel-fallback policy. Discovery and
pending transactions remain in management. Rate estimates, backoff, retries, queues, and agreements
remain with their implementing algorithms. The PHY owns actual radio state.

Readers use typed queries. Required updates use the owner's transition operations, not mutable
references to its internals. Capability assembly queries component contracts, so replacement
implementations can supply different answers. Shared protocol state stays in inspectable simple
modules; the containing compound composes components. No contract requires a particular containing
interface implementation. See [component boundaries](../domain/ieee80211.md#ar-wlan-arch-boundaries).

**3. Every state contract includes readiness and lifetime.**

Catalog definitions are immutable; their configured selection and the prepared local capability
profile survive ordinary protocol stop/restart. Initialization makes dependencies available before
their first use through declared stages and typed preparation calls. Static dependencies are queried,
not distributed by a signal. Readiness cannot depend on sibling traversal order.

Control changes through explicit procedures. Transient status is updated, expired, or cleared by its
owner. BSS/peer status belongs to a specific relationship; teardown or replacement cannot leave it
applicable to a new relationship, even with the same address. Each owner defines its stop, crash,
and restart behavior. Unprepared, absent, unknown, and unsupported are distinct states.

Transaction snapshots are immutable history. They do not become another authority for current
operation. A scanning radio's tuned channel is distinct from the STA's accepted BSS channel.
Interface-wide reconfiguration requires an explicit coordinated transition across dependent state.

**4. Derived state follows its inputs.**

The peer record caches directional capability compatibility, derived only from local capabilities
and accepted peer capabilities. Changed inputs refresh it before use; unchanged inputs reuse it.
Current BSS operation is read separately. Capability compatibility does not establish permission to
use HT: relationship eligibility and applicable operation must also permit the choice.

A BSS width change therefore need not rebuild a capability intersection, but may change selection
or eligibility. Equal capability advertisements still require relationship and liveness processing.
Preserve exact support, directionality, and unknown information. Every additional cache must name
its owner, inputs, refresh rule, and lifetime; it has no independent writer. Version counters need
a concrete consumer.

**5. Signals announce completed facts to independent listeners.**

The deciding component completes all required state and bookkeeping before notification. Shared
BSS/peer changes are published at the MIB commit boundary; PHY and algorithm changes are published
by their owners. A listener must see a consistent committed view and must not finish the publisher's
transition. Listener order cannot determine correctness.

The state-notification contracts are below. These names describe notification kinds, not prescribed
C++ identifiers or a requirement for one signal per row.

| Notification | Publisher | What listeners learn |
|---|---|---|
| **BSS operation changed** | MIB commit boundary, following management's transition | The applicable BSS view was activated, updated, replaced, or cleared |
| **Peer state changed** | MIB commit boundary, following management's transition | A relationship, accepted peer capability, or eligibility changed or was removed |
| **Radio state changed** | PHY | Local channel/band, radio mode, listening configuration, or receive/transmit activity changed |
| **Protocol outcome** | Component completing or observing the procedure | An exchange completed or failed, association completed, or beacon loss was detected |

An operational-state notification is emitted for a meaningful committed change, not simply because
another identical advertisement arrived. Protocol outcomes may still occur without such a change.
Immutable catalogs and prepared capabilities need no routine change signal. Algorithm-private state
needs a notification only when a defined independent consumer requires it.

Concrete signals declare their source/scope, change condition, payload, and lifetime under
[AR-COM-NOTIFY](../rule/architecture.md#ar-com-notify). An immutable borrowed snapshot is allowed;
it creates no second writable authority. Commands, queries, and required coordination use typed
calls or protocol messages, rather than notifications.

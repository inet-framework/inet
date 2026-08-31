# Add a protocol

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [design/protocol-anatomy.md](../design/protocol-anatomy.md), [rule/naming.md](../rule/naming.md)

The path from a protocol stem to a merged model. What the parts *are* is
[design/protocol-anatomy.md](../design/protocol-anatomy.md); this is the order to build them in.

**Adding a protocol touches no core file.** If a step below makes you edit `common/`, a dispatcher or
a registry switch, stop: the extension point you need already exists, or the design is wrong
([AR-EXT-NOCORE](../rule/architecture.md#ar-ext-nocore),
[REJ-04](../design/rejected-designs.md#rej-04)).

## 1. Write down the vocabulary first

Take the stem `Foo`, and write every name before any code
([NR-NED-ROLE](../rule/naming.md#nr-ned-role)). It takes five minutes and it settles a dozen
decisions:

| Artifact | Name |
| --- | --- |
| package, directory, namespace | `foo`, `inet::foo` |
| module, interface | `Foo`, `IFoo` |
| header chunk, serializer | `FooHeader`, `FooHeaderSerializer` |
| dissector, printer | `FooProtocolDissector`, `FooProtocolPrinter` |
| request, indication tag | `FooReq`, `FooInd` |
| state, configurator | `FooTable`, `FooConfigurator` |
| protocol identity | `Protocol::foo` |
| feature ids | `Foo`, `FooExamples`, `FooTests` |
| example, tests | `examples/foo/`, `tests/<category>/foo` |

A name that will not fit the scheme is usually a sign that the thing is not what you think it is.

## 2. Decide the layer and the slot

Which layer does it belong to, and which interface-typed slot does it fill
([AR-ORG-DOMAINS](../rule/architecture.md#ar-org-domains))? A protocol that fits no existing slot
needs a new contract, and a new contract is a design decision — write it down in
[design/decisions.md](../design/decisions.md) before you write the interface.

## 3. Declare before you implement

Write `Foo.ned` first: the parameters with their units and defaults, the gates, the signals and the
statistics. NED is the single source of truth
([D-NED-TRUTH](../design/decisions.md#d-ned-truth)), and writing it first forces the external
interface to be decided before the implementation talks you into something else.

## 4. The content and its wire boundary

`FooHeader.msg`, then the serializer. **Both, in the same change**
([AR-PKT-DUAL](../rule/architecture.md#ar-pkt-dual)). A header without a serializer is a gap that
surfaces months later at an emulation or capture boundary.

What is metadata goes in a tag, not in the header
([AR-PKT-TAGS](../rule/architecture.md#ar-pkt-tags),
[REJ-05](../design/rejected-designs.md#rej-05)).

## 5. The behavior

Compose it from small modules where you can, and use the queueing contracts where the datapath is a
chain ([D-COMPOSE](../design/decisions.md#d-compose),
[D-QUEUEING](../design/decisions.md#d-queueing)). Cite the standard clause beside every normative
rule ([QR-CMT-STANDARD](../rule/quality.md#qr-cmt-standard)) — it is what makes the model reviewable
by anyone but you.

Register the protocol identity, and reach peers through the dispatcher rather than by wiring or by
path ([D-REGISTRY](../design/decisions.md#d-registry)).

## 6. The tooling

Dissector, printer, filter support. Without them the packet exists but no tool can describe it, and
nothing fails to tell you so ([AR-OBS-INTROSPECTION](../rule/architecture.md#ar-obs-introspection)).

## 7. The feature

Add `Foo`, `FooExamples` and `FooTests` to `.oppfeatures`, with the dependencies. Then **build with
the feature off**: that is the only check that the dependency direction is real
([TR-CI-FEATURE-MATRIX](../rule/testing.md#tr-ci-feature-matrix)).

## 8. The evidence

A test in the category that matches the claim
([TR-CAT-MATCH](../rule/testing.md#tr-cat-match)), in the same pull request
([TR-SHIP-WITH](../rule/testing.md#tr-ship-with)). A fingerprint alone does not establish that a new
protocol is correct — it only records whatever the new code does, bugs included
([TR-FP-NOT-ENOUGH](../rule/testing.md#tr-fp-not-enough)).

Where the protocol makes a claim about the real world, check it against something outside INET
([TR-VALIDATE-EXTERNAL](../rule/testing.md#tr-validate-external)).

## 9. The example and the documentation

`examples/foo/` so a user can run it. A written page belongs in `showcases/` if it demonstrates an
effect, and a chapter in `doc/src/users-guide/` if it needs explaining. Do not restate the NED
declaration in prose ([REJ-07](../design/rejected-designs.md#rej-07)).

## 10. Submit it

Divide it into commits by concern, shared components first
([rule/pull-request.md](../rule/pull-request.md)), and follow
[contribute-a-change.md](contribute-a-change.md) from step 6.

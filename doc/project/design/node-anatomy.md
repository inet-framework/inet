# What a network node is made of

> **Kind:** design · **Status:** current · **Seal:** by section · **Owns:** — · **Stands on:** [decisions.md](decisions.md), [rule/architecture.md](../rule/architecture.md)

The settled form of a node. How to configure and assemble one is
`doc/src/users-guide/ch-network-nodes.rst`; this document says what the structure is, and why it is
that shape.

## A node is a composition, not a type

`StandardHost` and `Router` are not classes with behavior. They are compound modules that name a set
of slots, and each slot is interface-typed with a replaceable default
([D-COMPOSE](decisions.md#d-compose), [D-CONTRACTS](decisions.md#d-contracts)). A user replaces an
implementation, adds one, or omits one — in configuration, without writing C++
([R-COMPOSE-NOCODE](../requirement/accepted-requirements.md#r-compose-nocode)).

This is why a node type is cheap to add and why one node type serves scenarios its author never
imagined.

## The layers, and what joins them

```
        applications
             │            ← socket-style API (D-SOCKETS)
        transport
             │            ← message dispatcher, by protocol and service (D-REGISTRY)
        network
             │
        link layer
             │
        physical layer
             │
        the medium
```

The joins are the design. A layer does not name the layer above or below it by path; it declares what
it provides, and a dispatcher routes by that declaration. Adding a protocol to a node therefore does
not rewire the node, and a module written for one node layout works in another —
[REJ-02](rejected-designs.md#rej-02) records what path-based lookup cost instead.

## What crosses the layers, and what does not

| Crosses | Does not cross |
| --- | --- |
| the packet, with its content | a pointer to a sibling module |
| tags, which are how a layer asks and answers | a path into another node's tree |
| a direct call, for same-instant coordination inside the node ([D-DIRECT](decisions.md#d-direct)) | a zero-time message standing in for that call |

## The aspects that attach to any node

Mobility, power, clock and the physical environment are not layers. They are orthogonal, and a node
gains one by adding a submodule, not by becoming a different type
([R-SCOPE-CROSSCUT](../requirement/accepted-requirements.md#r-scope-crosscut)). A clock is the
sharpest case: a node with one has every timer in it read that clock, with its drift, and no protocol
model changes.

## The services a node holds

An interface table, a routing table, and the registries. They are found by lookup, never by path
([AR-MOD-NODEBASE](../rule/architecture.md#ar-mod-nodebase)), which is what lets the node's internal
structure change without editing every module that reaches across it.

## Lifecycle

A node starts, shuts down, crashes and recovers through one lifecycle protocol, and the transitions
are scriptable at a simulation time
([D-COMPOSE](decisions.md#d-compose),
[AR-LIFE-OPERATIONS](../rule/architecture.md#ar-life-operations)). Initialization is a single global
multi-stage order that every model slots into, rather than each subsystem inventing its own
([AR-LIFE-STAGES](../rule/architecture.md#ar-life-stages)) — which is what makes a node assembled from
parts that never met still come up in a defined order.

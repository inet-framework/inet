# RFC 7641 (CoAP Observe) — catalog of checkable statements

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `RFC7641-*` · **Stands on:** [standards.md](../../protocol/coap/standards.md), [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

This document is the step 3 artifact of the standards test workflow for RFC 7641, Observing Resources in the Constrained Application Protocol (CoAP), September 2015. The catalog comes from the specification text only and contains no simulation model names and no code references.

Source: `standards/RFC/rfc7641.txt` (<https://www.rfc-editor.org/rfc/rfc7641.txt>, September 2015).

The family of CoAP documents is in [`standards.md`](../../protocol/coap/standards.md), the feature map is in [`features.md`](../../protocol/coap/features.md), and the workflow state lives in [`coverage.md`](../../model/coap/coverage.md).

## Index

| ID | Statement |
| --- | --- |
| [RFC7641-REG-1](#rfc7641-reg-1) | A `GET` with `Observe=0` registers the `(client endpoint, token)` observer and returns `2.05`/`2.03` carrying an `Observe` sequence number; absence of `Observe` in the response means no registration was created. |
| [RFC7641-REG-2](#rfc7641-reg-2) | Re-sending `GET` with `Observe=0` from the same `(client endpoint, token)` updates the existing entry without duplicating it, and capacity exhaustion falls back to a normal `GET` response without `Observe`. |
| [RFC7641-NOTIFY-1](#rfc7641-notify-1) | Resource state changes emit `2.05 Content` (or `2.03 Valid` on matching `ETag`) notifications carrying the registration Token and a 24-bit `Observe` option. |
| [RFC7641-ORDER-1](#rfc7641-order-1) | Clients accept notifications only when the 24-bit `Observe` value is newer under RFC 7641 §3.4 serial arithmetic or arrived strictly more than `128` seconds after the previous accepted notification. |
| [RFC7641-SUPER-1](#rfc7641-super-1) | A resource change during an unacknowledged `CON` notification supersedes the pending representation and updates `Observe` and Message ID on the next transmission; exhausting `MAX_RETRANSMIT` removes the observer. |
| [RFC7641-FRESH-1](#rfc7641-fresh-1) | When `Max-Age` plus a random jitter in `[5s, 15s]` expires without a new notification, the client re-registers with `GET` + `Observe=0` and the same Token. |
| [RFC7641-CANCEL-1](#rfc7641-cancel-1) | An observation is terminated by `GET` with `Observe=1`, by a client `RST` on a notification, or by a non-`2.xx` notification (`4.04` or `4.06`). |

## Registration, notification ordering, freshness, and cancellation

### RFC7641-REG-1

**A `GET` with `Observe=0` registers the `(client endpoint, token)` observer and returns `2.05`/`2.03` carrying an `Observe` sequence number; absence of `Observe` in the response means no registration was created.**

> "A client registers its interest in a resource by initiating an extended GET request to the server. In addition to returning the current representation of the target resource, the server adds the client to the list of observers of the resource and includes an Observe Option in the response." — §3.1 and §4.1, `rfc7641.txt:365-395`

- Strength: must. Class: wire.
- Check idea: send `GET` with `Observe=0` to an observable resource and to a non-observable resource; verify the observable resource returns `2.05` with an `Observe` option and emits notifications on state change, while the non-observable resource returns `2.05` without `Observe` and emits no notifications.

### RFC7641-REG-2

**Re-sending `GET` with `Observe=0` from the same `(client endpoint, token)` updates the existing entry without duplicating it, and capacity exhaustion falls back to a normal `GET` response without `Observe`.**

> "If an entry with a matching endpoint/token pair is already present in the list of observers, the server MUST NOT add a new entry but MUST replace or update the existing one." — §4.1, `rfc7641.txt:734-738`

- Strength: must. Class: wire.
- Check idea: send two `GET` requests with `Observe=0` and the same Token, trigger a resource update, and verify only one notification is sent; exceed `maxObservers` and verify the server returns `2.05` without `Observe`.

### RFC7641-NOTIFY-1

**Resource state changes emit `2.05 Content` (or `2.03 Valid` on matching `ETag`) notifications carrying the registration Token and a 24-bit `Observe` option.**

> "A notification is an additional response sent by the server in reply to the GET request with the token specified by the client and an Observe Option with a sequence number." — §3.2 and §4.2, `rfc7641.txt:410-445`

- Strength: must. Class: wire.
- Check idea: update an observed resource and verify notifications carry the client's registration Token, a 24-bit `Observe` sequence number (`0..16777215`), and `2.03 Valid` when the new representation matches an `ETag` supplied in the registration request.

### RFC7641-ORDER-1

**Clients accept notifications only when the 24-bit `Observe` value is newer under RFC 7641 §3.4 serial arithmetic or arrived strictly more than `128` seconds after the previous accepted notification.**

> "Let V1 be the value of the Observe Option in the last valid notification and V2 the value in the newly arrived notification, and let T1 and T2 be the client-local timestamps. V2 is newer than V1 if (V1 < V2 and V2 - V1 < 2^23) or (V1 > V2 and V1 - V2 > 2^23) or (T2 > T1 + 128 seconds)." — §3.4, `rfc7641.txt:505-535`

- Strength: must. Class: wire plus end-to-end.
- Check idea: reorder notifications on the link, test 24-bit wraparound across `2^24 - 1 -> 0`, and test the `128s` boundary (`<= 128s` rejected when serial is older, `> 128s` accepted).

### RFC7641-SUPER-1

**A resource change during an unacknowledged `CON` notification supersedes the pending representation and updates `Observe` and Message ID on the next transmission; exhausting `MAX_RETRANSMIT` removes the observer.**

> "When the state of an observed resource changes while the transmission of a notification for that resource is still in progress, the server SHOULD abort or update that transmission with the new representation, a new Observe Option value, and a new Message ID when retransmitting." — §4.5.2, `rfc7641.txt:965-1005`
>
> "If the transmission of a Confirmable notification fails after MAX_RETRANSMIT attempts, the server MUST remove the associated entry from the list of observers." — §4.5, `rfc7641.txt:945-955`

- Strength: must. Class: wire.
- Check idea: drop the first `CON` notification, update the resource before the retry timer fires, and verify the retry carries the newer payload, newer `Observe` value, and a new Message ID while preserving the retry backoff schedule; drop all retries and verify observer removal.

### RFC7641-FRESH-1

**When `Max-Age` plus a random jitter in `[5s, 15s]` expires without a new notification, the client re-registers with `GET` + `Observe=0` and the same Token.**

> "After Max-Age expires, the client waits for a randomized time between 5 and 15 seconds and, if no notification has arrived, sends a new GET request with the same token and an Observe Option set to 0 to re-register its interest." — §3.3.1, `rfc7641.txt:460-495`

- Strength: should. Class: wire.
- Check idea: observe a resource whose `Max-Age` is short (`10s`) with no further server updates and verify the client sends a re-registration `GET` (`Observe=0`, same Token) inside `[Max-Age + 5s, Max-Age + 15s]`.

### RFC7641-CANCEL-1

**An observation is terminated by `GET` with `Observe=1`, by a client `RST` on a notification, or by a non-`2.xx` notification (`4.04` or `4.06`).**

> "A client may deregister by sending a GET request that has the Token field set to the token of the observation to be cancelled and includes an Observe Option with the value set to 1 (deregister), or by rejecting a notification with a Reset message. In the event of an error (non-2.xx notification), the server removes the observer and omits the Observe Option." — §3.6, §4.2, and §4.3.1, `rfc7641.txt:575-625`

- Strength: must. Class: wire.
- Check idea: exercise explicit `Observe=1` deregistration, local cancellation answering the next notification with `RST`, and resource deletion (`4.04 Not Found` without `Observe` option), verifying that subsequent resource updates produce no further notifications.

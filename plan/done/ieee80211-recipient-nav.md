# IEEE 802.11 recipient responses and local NAV

Date: 2026-10-07.
Status: implementation and focused verification complete.

## Result and cause

Recipient CTS, ACK, and Basic Block Ack responses must retain their Duration/ID fields without a local NAV update. NAV records a reservation of the medium. Received reservations must retain their effect on channel access.

Tx previously passed every completed frame's duration to Rx. Rx extended its NAV timer. Both HCF and DCF refuse a CTS when Rx reports a busy medium. HCF means hybrid coordination function. DCF means distributed coordination function.

For example, a hypothetical recipient receives an RTS with a 500 us duration at 6 Mbps. Its CTS carries 440 us after subtraction of SIFS and CTS airtime. SIFS means short interframe space. A second RTS arrives 150 us later. The old local NAV prevents the second CTS. The correction permits the second CTS if no received reservation blocks it.

This fix applies to existing single protection and DCF. It does not require multiple protection, change response Duration/ID calculations, or add a new protection policy.

## Ownership and change

HCF and DCF identify whether each transmission belongs to the holder or recipient. Their holder callers pass `true` for `updateLocalNav`. Their recipient callers pass `false`. Both `ITx::transmitFrame()` overloads require this argument.

Tx retains the choice with the accepted frame through its delay and transmission. Tx saves the completed frame's choice before the completion callback. The callback can accept a new frame with a different choice. Tx uses the saved choice for the completed frame's NAV update.

For example, a completed recipient CTS retains `false` when its callback accepts a holder frame with `true`. The holder frame's choice does not change the completed CTS's choice.

Custom implementations and callers must add the argument. The migration guide gives the signatures, state lifetime, and callback order. WHATSNEW describes the user effect. The source change adds no timer, event, packet field, or serializer change.

## Independent commit

The fix commit contains the Tx contract, Tx implementation, four HCF/DCF caller changes, tests, migration instructions, release note, and causal fingerprints. The multiple-protection PR depends on this commit. Its feature plan belongs to that feature PR.

## Verification

The unit fixture `Ieee80211TxNavChoice_1.test` checks both overloads and the saved choice across the completion callback. The module fixture `Ieee80211RecipientNav_1.test` checks production HCF, DCF, Tx, Rx, and radio transmission under the existing default protection policy.

The module fixture supplies valid synthetic requests at the MAC reception boundary. The production MAC generates and transmits the responses. Run 0 selects HCF without Block Ack. Run 1 selects DCF. Run 2 selects HCF with Block Ack. All three runs use seed 0 and fixed 6 Mbps modes.

Runs 0 and 1 check two CTS responses 150 us apart, a fragment ACK, refusal after an unrelated received reservation, and a holder transmission. The response fields remain 440 us. Recipient transmissions cause no local NAV update. The holder transmission still calls Rx. This fixture does not prove physical reception of the synthetic requests or full end-to-end packet delivery.

Run 2 supplies an immediate ADDBA request for peer `02:00:00:00:00:02` and TID 5. HCF transmits an ACK and an ADDBA response. The fixture supplies the peer's ACK after the response transmission. It then supplies a Basic Block Ack request with a 500 us duration. The production Basic Block Ack response must retain a positive duration without a local NAV update. IEEE Std 802.11-2024, 9.2.5.6 defines the response duration as the request duration minus SIFS and response airtime.

The run checks the response type, receiver address, TID, starting sequence number, duration, and local NAV update count. It also checks that the medium is free before the request's reservation would expire. The fixture derives the expected duration from the fixed PHY mode. Synthetic requests and the peer's ACK enter at the MAC reception boundary. The fixture does not test peer PHY reception or data delivery under the agreement.

The fixture marks Rx busy before each synthetic frame in run 2. It restores the idle reception state after MAC delivery. This gives the immediate ACK time to complete before the queued ADDBA response obtains channel access.

The original parent used for the regression control, `c3ab2dda1322bfe3735ac420ff5532a5d203fb38`, fails the second-CTS assertion at 1.3 ms under both HCF and DCF. The current parent is `e04a0113b383d55e5b6dbbd4dde75d825a6f2756`. The corrected source passes both runs. This control separates the production regression from the overload unit test.

Run the focused checks from the repository root:

```sh
make MODE=debug -j8
./bin/inet_run_unit_tests -m debug --no-concurrent -f 'Ieee80211TxNavChoice_1\.test$'
./bin/inet_run_module_tests -m debug --no-concurrent -f 'Ieee80211RecipientNav_1\.test$'
make MODE=release -j4
./bin/inet_run_module_tests -m release --no-concurrent -f 'Ieee80211RecipientNav_1\.test$'
```

The original campaign passed debug and release library builds, the debug unit fixture, and both HCF/DCF production runs in both modes. The extended module fixture passes all three runs after a fresh debug build. Debug mode supplies the assertion evidence. The extended fixture did not run in release mode.

An additional negative control enables local NAV updates only for Basic Block Ack responses. Runs 0 and 1 pass. Run 2 fails at the local NAV update assertion at 1.713 ms. After source restoration, the final debug build and all three runs pass. The final change contains no runtime source edit.

The fix retains the 24 fingerprint rows from its original source commit. Their 37 changed values comprise 24 `tplx`, seven `~tNl`, and six `~tND` values. Seventeen rows change only the selected simulation events. Seven rows also change selected network packet ingredients. Graphical `tyf` values remain unchanged and unverified.

Earlier checkpoint comparisons attribute these values to recipient NAV exclusion before multiple protection exists. For example, an AP's ACK with a 140 us duration delays its next observed frame by 140 us before the correction. The correction removes that delay. A fingerprint difference alone does not establish correctness; production assertions and the checkpoint comparisons supply that evidence.

The baseline values require no further update during extraction. Verification must compare the same CSV time limits and checksum/FCS overrides. The separate PR must retain every row with its causal source change.

The selected 24 rows pass in both debug and release modes against the committed values. Each run retains its CSV time limit and checksum/FCS overrides. The PR description supplies the exact selection and commands. No baseline value changes during extraction.

The global architecture, naming, and interface gates exit 1 at both the pinned base and corrected source. The committed change adds no finding. The local naming gate also reports six existing ignored generated headers whose MSG inputs are absent. These files are outside the committed change. The scoped architecture, source-seal, commit, classification, and whitespace checks must pass before publication.

## Limits

This fix preserves the existing completion callback order and received NAV rules. It does not establish full TXNAV behavior, MAC stop/start safety, multiple protection, or new Block Ack support. TXNAV means the standard transmitted reservation timer. Basic Block Ack response callers receive the same explicit choice. The production fixture directly checks CTS, ACK, and Basic Block Ack responses.

The focused campaign uses one fixed seed for each coordination function. Full wireless fingerprint coverage, additional seeds, feature-off builds, and cross-platform behavior remain outside this verification scope.

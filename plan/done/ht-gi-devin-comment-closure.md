# HT/GI Devin comment closure

Status: planned. Verification evidence will be recorded after implementation.

## Implementation contract

- Invariant and owner: transmitter compatibility rejection must precede the radio/MAC transaction.
  The transmitter owns exact-tuple resolution; radio coordinates updates. Failures after PHY mutation
  remain fatal. A rejected preflight changes no catalog, current mode, peer cache or notification count.
- Entry/control path: both radio setters call `changeModeSet`; catalog-only changes preflight a const
  transmitter query before `beginModeSetChange`. The transmitter setter reuses the same query.
  Explicit changes keep their existing membership check and virtual setter dispatch.
- Affected artifacts: radio/transmitter C++ and transmitter declaration; management serializer masks;
  peer-mode selector citations; `IIeee80211Mode` and existing `Ieee80211ModeBase`; transition module
  and VHT management-element unit fixtures. HT/VHT duration overrides remain authoritative.
- Siblings/terminal paths: same/null catalogs retain existing transmitter semantics; explicit mode
  changes and post-mutation participant failures retain their contract. Serializer read/write masks
  stay symmetric. No lifecycle or packet ownership change.
- Boundaries: exact bitrate/bandwidth/NSS/GI tuple; null mode/catalog behavior unchanged. Duration
  units and arithmetic unchanged. VHT subtype placement remains identical to HT, with distinct bits;
  this does not add band/capability presence validation.
- Verification: debug and release library builds; filtered transition/failure/registration and VHT
  association modules; HT/GI/VHT mode, management-element and peer-selection units; the earlier
  capability/BSS evidence's focused units/modules/protocols and two legacy ad hoc fingerprints.
  Extend the transition fixture with rejection followed by successful explicit retry, and VHT codec
  coverage for request-operation rejection and independent HT/VHT presence.


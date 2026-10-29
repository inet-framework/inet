IEEE 802.11 HT A-MPDU PHY reception
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

HT receivers require a present error model to implement ``IIeee80211AmpduErrorModel``. This optional WLAN contract provides pure mode-support queries and probabilities for bounded PSDU ranges. ``Ieee80211ErrorModelBase`` supplies the implementation for NIST and Yans. Custom HT models outside this base must implement the C++ contract and its NED module interface. Non-HT models require no aggregate role.

An absent error model remains valid. The receiver retains its acquisition, sensitivity, attempt, and SNIR checks without error draws. This includes ordinary HT operation with ``Ieee80211UnitDiskRadio``. The aggregate profile covers scalar and dimensional HT20 reception; it adds no unit-disk aggregate support claim.

The receiver checks role presence at physical-layer initialization. Before the MIB update, the MAC supplies its exact mode set, channel widths, and spatial-stream limit for receive coverage validation. The model must support the modes that these inputs advertise, including HT20 short GI. Unsupported modes outside the effective stream limit cause no rejection. Runtime mode-set changes also check role presence. Any later capability-update caller must validate its exact inputs before the MIB update.

BerTable retains its current limit. Its NED type rejects the radio's HT operation string. The allowed ``n`` value reaches its initializer, which also rejects HT. Both failures precede the new role check. BerTable does not gain HT aggregate support. Supported HT configurations can use NIST or Yans.

The new role diagnostic applies to otherwise valid custom HT models without the contract. The current packet-level BerTable reception path also requires a transmission bit model. Scalar packet-level transmissions do not supply that model. This existing limitation remains outside the HT A-MPDU change.

The modeled HT-SIG header preserves all 48 bits through byte conversion. ``lengthField`` supplies HT Length. ``aggregation`` distinguishes an A-MPDU from a standalone MPDU, including singleton aggregates. The radio derives mode fields from the selected mode and calculates the CRC after field assignment. Decode preserves the received CRC. HEADER validation rejects invalid CRC and unsupported fields before delimiter or MPDU outcomes.

Separate-part reception retains its current abort; whole-signal reception supplies a failed container. DATA separately refuses modeled content shorter than a valid HT Length. This refusal retains the accepted header property but supplies no range outcomes. It represents malformed model input, not physical carrier loss. Decapsulation never uses an invalid header or insufficient content for length-based trailer removal. Ordinary valid HT duration inputs remain unchanged.

``Ieee80211AmpduInd`` carries copied PHY facts without reception or transmission pointers. Each range identifies a delimiter or complete MPDU by PSDU-relative byte offset, byte length, validity, and physical success. Spacing delimiters have four-byte ranges. Alignment padding has no range outcome or separate draw. MAC delimiter and FCS checks still control acceptance; the PHY indication alone grants no delivery or response permission.

The default ``Rx`` rejects containers whose PHY indication identifies an aggregate before standalone FCS or MAC header checks. This applies even when every range succeeds. The drop signal retains the PHY outcomes. Custom receive modules must keep aggregate containers separate from standalone frame parsing until they implement member acceptance. MPDU deaggregation can extract bounded recovered members from an incomplete layout. Extraction alone grants no MAC acceptance.

The medium cache retains each aggregate decision. PREAMBLE calculates no HEADER or DATA outcome. HEADER, DATA, WHOLE, and direct success queries reuse retained prerequisites and samples. WHOLE summarizes all ranges. Partial DATA failure preserves the aggregate bytes for the later MAC receive integration. This change alone does not enable MAC A-MPDU receipt or implicit Block Ack responses.

With a present model, aggregate ``ErrorRateInd`` fields contain ``NaN`` for unknown probabilities. The receiver does not calculate the former whole-PSDU estimate for these fields. Member outcomes are separate from probability estimates. Without a model, the receiver preserves the common base's tag presence and values. Ordinary MPDUs retain their current metadata. Custom readers of aggregate error-rate fields must accept unknown values.

NIST and Yans calculate each range probability from its byte length and the configured scalar SNIR reduction. The receive profile includes HT20 long GI and short GI. Each range uses an independent draw when its success probability lies strictly between zero and one. This model does not represent correlated FEC errors or interference changes within one member. Alignment padding contributes duration but receives no separate outcome. The model checks establish no new error-accuracy or goodput claim.

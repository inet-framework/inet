# Implementation plan: error decisions and corruption in packet-level radio reception

> **Kind:** procedure · **Status:** approved · **Seal:** none · **Owns:** — · **Stands on:** [implementation plan procedure](../../doc/project/guide/write-an-implementation-plan.md), [IEEE 802.11 rules](../../doc/project/domain/ieee80211.md), [Packet design](../../doc/project/design/packet-anatomy.md)

Checkout and revision: each pull request of section 6 has its own worktree, with the name of its branch: `inet-physicallayer-snir-interval`, `inet-physicallayer-part-error-decisions`, and `inet-physicallayer-content-errors`. The base is `master` at `91219b0fa7`. The last worktree still holds the implementation of draft 3 (PR #1443, head `b703ae8ccf`).

Plan revision: draft 4, 2026-11-10. Draft 2 (2026-11-05) gave errors in the content only to the `chunk` corruption mode. Draft 3 (2026-11-09) gave all four corruption modes one rule, and PR #1443 implemented it in one pull request. A review of that implementation on 2026-11-10 found two defects, one shift of the model, and some interfaces that a later change would redo (section 9). The maintainer also asked for a smaller first part. Draft 4 repairs these items and divides the work into three pull requests (D16). Section 8 lists the changed decisions.

Approval: the maintainer approved draft 2 on 2026-11-05 and draft 3 on 2026-11-09, with D1, D10, D11, D14, D15, D16, and D17. On 2026-11-10, the maintainer agreed to the repairs of the review and to three pull requests. The maintainer approved draft 4 on 2026-11-10, with its changed decisions (D6, D11, D12, D16), its new decisions (D18 to D24), the risk R6, and the commits S1, P0 to P4, and Q1 to Q5.

## 1. Problem and intended behavior

The four corruption modes of a flat receiver do not follow one rule. They draw at different times, and they keep their outcomes in different objects:

| Mode | When the draws occur | What the decision keeps | When the representation changes |
| --- | --- | --- | --- |
| `packet` | one draw at the decision of each part that the radio asks | a success flag | at the result: a copy with a packet-level bit error |
| `chunk` | one draw at the decision; at the result, one draw for each chunk, again and again until some error occurs | a success flag | at the result: incorrect copies of the chunks with errors |
| `byte`, `bit` | one draw at the decision; at the result, one draw for each byte or bit of the packet, again and again until some error occurs | a success flag | at the result: a raw packet with flipped values and no incorrect flag |

The chunk, byte, and bit modes sample two times: the decision and the corruption are two independent samples. All modes use the SNIR of the whole reception.

IEEE 802.11 also needs errors in place. In this hypothetical case, one HT PPDU carries an A-MPDU with four MPDUs, and a hidden station starts to transmit during the last MPDU. A real receiver keeps the first three MPDUs. INET uses one SNIR for the whole reception, so all four MPDUs fail together or succeed together. [PR #1363](https://github.com/inet-framework/inet/pull/1363) needs the partial result for its A-MPDU work.

The intended behavior follows one rule in all modes:

- **Region:** the unit of one decision. In the `packet` mode, a region is a whole signal part. In the `chunk`, `byte`, and `bit` modes, a region is a chunk of a part. A part without packet content is one region.
- **Error decision:** the decision of each part (HEADER, DATA) decides each of its regions one time. A region fails if its SNIR is not above the threshold of the receiver, or else if one draw against its error rate fails. Both use the SNIR of the time of that region. The decision stores the failed regions with their bit error rates.
- **Corruption:** the result applies the stored failed regions to the representation of the mode. It makes no new decision. The `packet` mode gives a packet-level bit error. The `chunk` mode gives an incorrect copy of each failed chunk. The `byte` and `bit` modes flip bytes or bits in each failed chunk, as its bit error rate gives them, and mark the chunk incorrect.

In the case above, the chunks of the first three MPDUs keep their high SNIR and stay correct, and the chunks of the fourth MPDU fail.

## 2. Current implementation and evidence

The facts below come from the source of master before this branch.

| Source | Current behavior | Consequence |
| --- | --- | --- |
| [FlatReceiverBase.cc](../../src/inet/physicallayer/wireless/common/base/packetlevel/FlatReceiverBase.cc) `computeIsReceptionSuccessful()` | Applies the SNIR threshold of `SnirReceiverBase`. Then it draws one number against `computePacketErrorRate(snir, part)` with `dblrand()` of the receiver. A rate of 0 or 1 draws no number. | The decision comes before the content. |
| [FlatReceiverBase.cc](../../src/inet/physicallayer/wireless/common/base/packetlevel/FlatReceiverBase.cc) `computeReceivedPacket()` | A success duplicates the transmitted packet. A failure calls `errorModel->computeCorruptedPacket(snir)`. It adds an `ErrorRateInd` with the rates of the WHOLE part. | The content follows the decision. |
| [ErrorModelBase.cc](../../src/inet/physicallayer/wireless/common/base/packetlevel/ErrorModelBase.cc) `computeCorruptedPacket()`, `corruptBytes()`, `corruptBits()`, `hasProbabilisticError()` | Applies `corruptionMode` with the BER of the WHOLE part. The byte and bit modes call `peekAllAsBytes()` or `peekAllAsBits()` and draw for each unit with `dblrand()` of the error model. A `while (!isCorrupted)` loop repeats the sampling until some error occurs; its TODO comment says that the decision is already made. The result has no incorrect flag. | The corruption is a second sample. A declared checksum does not find the flipped values. |
| [ErrorModelBase.h](../../src/inet/physicallayer/wireless/common/base/packetlevel/ErrorModelBase.h) | `enum class CorruptionMode` with `CM_UNDEFINED`, `CM_PACKET`, `CM_CHUNK`, `CM_BYTE`, `CM_BIT` belongs to `ErrorModelBase`. | A receiver cannot read the mode through `IErrorModel`. |
| [Ieee80211ErrorModelBase.cc](../../src/inet/physicallayer/wireless/ieee80211/packetlevel/errormodel/Ieee80211ErrorModelBase.cc) | `computeCorruptedPacket()` refuses every mode except `packet`. NIST and Yans return NaN from `computeBitErrorRate()`. | IEEE 802.11 has only whole-packet errors. |
| [ErrorModelBase.cc](../../src/inet/physicallayer/wireless/common/base/packetlevel/ErrorModelBase.cc) `getScalarSnir()`, [ISnir.h](../../src/inet/physicallayer/wireless/common/contract/packetlevel/ISnir.h) | Only the SNIR of the whole reception exists. `ScalarSnir` and `DimensionalSnir` can evaluate an interval. | S1 adds the interval to the contract. |
| [SnirReceiverBase.cc](../../src/inet/physicallayer/wireless/common/base/packetlevel/SnirReceiverBase.cc) | Compares the SNIR of the whole reception with `snirThreshold` for every part. | A short dip anywhere fails every part. |
| [RadioMedium.cc](../../src/inet/physicallayer/wireless/common/medium/RadioMedium.cc), [Radio.cc](../../src/inet/physicallayer/wireless/common/radio/packetlevel/Radio.cc) | `computeReceptionResult()` uses only the WHOLE decision. `isReceptionSuccessful()` computes a decision without the cache (#1446 repairs it). With separate parts, the cached SNIR misses later interferers (#1446 repairs it). | The decisions of separate parts must share one sample, with the current SNIR. |
| [ReceiverBase.cc](../../src/inet/physicallayer/wireless/common/base/packetlevel/ReceiverBase.cc) `computeIsReceptionAttempted()` | The result depends on the receptions in progress at the time of the query. | For a query after the start of the reception, the flag does not depend on the part. With separate parts, `Radio` stops a reception after a part that is not successful. Thus the chain of the parts can read `isReceptionSuccessful()` (A14). |
| [CommunicationCacheBase.cc](../../src/inet/physicallayer/wireless/common/base/packetlevel/CommunicationCacheBase.cc) | Keeps one decision for each radio, transmission, and part, and one result for each reception. | A part decision can keep the errors of its part until the result. |
| [Ieee80211NistErrorModel.cc](../../src/inet/physicallayer/wireless/ieee80211/packetlevel/errormodel/Ieee80211NistErrorModel.cc), [Ieee80211YansErrorModel.cc](../../src/inet/physicallayer/wireless/ieee80211/packetlevel/errormodel/Ieee80211YansErrorModel.cc) | `getDataSuccessRate()` adds the 16 SERVICE bits for non-HT OFDM modes (P1 moves them). | The whole-part rate counts the SERVICE, tail, and pad bits. Each region gets its share of them (D6). |
| [Ieee80211Radio.cc](../../src/inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Radio.cc), [ApskRadio.cc](../../src/inet/physicallayer/wireless/apsk/packetlevel/ApskRadio.cc), IEEE 802.15.4 | The IEEE 802.11 packet holds the PHY header chunk, the PSDU, and a tail and pad chunk. The APSK packet holds an `ApskPhyHeader`, the payload, and padding. The IEEE 802.15.4 packet holds no header chunk. No shared interface gives these ranges. | Each flat receiver names the range of its parts. |
| [Rx.cc](../../src/inet/linklayer/ieee80211/mac/Rx.cc) `isFcsOk()`, [Packet.h](../../src/inet/common/packet/Packet.h) | Rx drops a frame if `hasBitError()` is true or the content is incorrect. `Packet::hasBitError()` includes the incorrect flags of the content. | A single MPDU needs no MAC change. |
| Packet API (sealed) | Master `7ec0d4d3f3` keeps the flags of chunks in parts, conversions, and merges. | An ordinary insertion keeps the flags of the regions. |
| Configurations | [examples/wireless/corruptionmode](../../examples/wireless/corruptionmode/omnetpp.ini) runs the four modes with `ApskRadio`. Most wireless configurations use the default `packet` mode. [showcases/wireless/throughput](../../showcases/wireless/throughput/omnetpp.ini) uses `separateReceptionParts`. | The change of the packet mode moves the baselines of every wireless configuration with errors. |

IEEE Std 802.11-2024 gives these requirements for its first user:

- 19.3.21 and Figure 19-27: an HT-SIG CRC failure gives PHY-RXEND.indication(FormatViolation). After a valid header, the PHY decodes the symbols until the end of the PSDU.
- 8.3.5.15.2: the RXERROR values contain no error for the content of the PSDU. The MAC finds such errors.
- 9.7.1, 9.7.2, and Annex O.2: each A-MPDU subframe has a delimiter with a CRC, each MPDU has its own FCS, and after an invalid delimiter the receiver moves forward 4 octets.
- 10.3.2.3.7: "EIFS shall not be invoked for an A-MPDU if one or more of its frames are received correctly."

Thus the PHY must give the PSDU with its errors in place, and a header failure stays a failure of the whole reception.

## 3. Scope and acceptance criteria

1. **A1, one decision rule:** in all four modes, the decision of the HEADER part and the decision of the DATA part each decide their regions one time. A region fails if the threshold check of the receiver fails for its time interval, or else if one draw against its error rate fails. The threshold check is a condition, not a probability, and draws no number. The decision stores the failed regions with their bit error rates. The WHOLE decision takes the result and the failed regions of the DATA decision and draws nothing.
2. **A2, regions:** the `packet` mode has one region for each part. The `chunk`, `byte`, and `bit` modes have one region for each chunk of the part. A part without packet content is one region.
3. **A3, time and SNIR:** each region uses the SNIR of its own time interval, with `snirMode`, `snirOffset`, and `snirThresholdMode`.
4. **A4, header rule:** if the HEADER decision is not successful, or the packet does not hold the complete range of a part, the DATA part draws nothing, and the received packet has a packet-level bit error, in all modes.
5. **A5, corruption:** the result applies the stored failed regions without a new decision. `packet`: a packet-level bit error. `chunk`: an incorrect copy of each failed chunk. `byte`, `bit`: each failed chunk becomes a raw chunk with flipped units, marked incorrect. The flipped units follow the bit error rate of the region, and at least one unit is flipped. The other chunks stay unchanged, and the transmitted packet stays unchanged.
6. **A6, draws:** the decisions draw in the generator of the receiver, at most once for each region, and only for a reception that the receiver attempts. The byte and bit corruption draws in the generator of the error model, at most one draw more than the number of flipped units of each failed region. Later queries of the same decision draw nothing.
7. **A7, separate parts:** with `separateReceptionParts = true`, the decisions occur at the ends of their parts and make the same draws. Their SNIR includes the interferers that started before the decision.
8. **A8, IEEE 802.11:** the HEADER range is the PHY header chunk, and the DATA range is the PSDU that the length field gives. All four modes work with the NIST, Yans, and BER table models.
9. **A9, APSK and IEEE 802.15.4:** `ApskReceiver` and `Ieee802154NarrowbandReceiver` use the same rule.
10. **A10, no error model:** a flat receiver without an error model keeps its current behavior.
11. **A11, baselines:** each commit updates the fingerprints that it moves and names the statistics baselines that it moves. The packet mode moves the baselines of every wireless configuration with errors. A statistics baseline gets an update only if its test passes before the change (D17).
12. **A12, scope:** the series changes no MAC code except #1447, and no file in `common/packet/`.
13. **A13, rate consistency (new in draft 4):** at a constant SNIR, the product of (1 − region rate) over all regions of the header and the data is equal to 1 − the whole-part rate of the error model. This holds for each error model and each corruption mode. Thus the packet mode keeps the reception probability of master at a constant SNIR, and only the number of the draws changes. The two region rates also agree: the region rate is equal to 1 − (1 − region bit error rate)^length.
14. **A14, the chain of the parts (new in draft 4):** each part reads `isReceptionSuccessful()` of the decision of the part before it. `Radio` reads the same flag with separate parts.

These items are follow-up work:

- **F1:** A-MPDU member acceptance, the EIFS rule for A-MPDUs, the NAV from members, and Block Ack. PR #1363 owns them.
- **F2:** HT-SIG fields and the HT-SIG CRC check. PR #1363 owns them.
- **F3:** smaller regions for raw content. With `sendRawBytes`, each part is one chunk and thus one region.
- **F4:** a symbol-exact time map for IEEE 802.11, layered receivers, correlated FEC errors, and the exact positions of the SERVICE, tail, and pad bits (D6).
- **F5:** the whole-part rate of `StochasticErrorModel` reads a bit model that flat packet-level transmissions do not have, when `packetErrorRate` is not set.
- **F6:** for a flat receiver with an error model, `computeIsReceptionSuccessful()` of `IReceiver` answers only the threshold check. The decision is the authority. No code in INET calls that function for such a receiver.
- **F7:** in the `chunk` mode, a failed padding chunk of APSK gives a decision that is not successful and a packet without an error flag, as on master.

## 4. Proposed design and reasons

### 4.1 Responsibilities

| Component | Responsibility |
| --- | --- |
| `ISnir` | The SNIR of a time interval (S1) |
| `SnirReceiverBase` | The threshold check of a time interval (S1) |
| Error model | The rate and the bit error rate of a region (P1), its corruption mode, the representation of the stored errors (`computeCorruptedPacket()`), and the draws of the byte and bit corruption |
| `FlatReceiverBase` | The regions of each part, their time, and the decision draws; the order of the parts; the header rule |
| A concrete flat receiver | The packet range of its HEADER and DATA parts (`computePartRange()`) |
| `RadioMedium` | The cache: one decision for each part with its failed regions, and one result for each reception; the interference, noise, and SNIR, which a new overlapping transmission removes (#1446) |
| `IReceptionDecision` | The flags of a part decision and the failed regions of its part |
| MAC | Its current checks of the content |

A PHY can refine the generic behavior at three points: the range of a part, the time map of a region, and the threshold check of a time interval. The series needs only the first point for IEEE 802.11.

### 4.2 Data structures

```cpp
// A region of the transmitted packet that an error decision found erroneous (ErrorRegion.h).
struct INET_API ErrorRegion
{
    b offset;               // in the transmitted packet
    b length;
    double bitErrorRate;    // the error rate of one bit of the region, at the SNIR of the time of the region
};
```

A decision gives the failed regions of its part. No other structure holds the errors: the success flag of the decision tells whether the part was received (D12).

```cpp
// IReceptionDecision
virtual const std::vector<ErrorRegion>& getErrorRegions() const = 0;

// ReceptionDecision
ReceptionDecision(const IReception *reception, IRadioSignal::SignalPart part, bool isReceptionPossible,
        bool isReceptionAttempted, bool isReceptionSuccessful, const std::vector<ErrorRegion>& errorRegions = {});
```

A flat receiver names the bits of a part with a value (D21):

```cpp
// FlatReceiverBase, protected
struct PartRange
{
    b length;               // the number of bits of the part
    bool isInPacket;        // false if the packet does not hold the bits of the part as content
    b offset;               // the offset of the bits in the transmitted packet, if the packet holds them
};
```

### 4.3 Interfaces

`ISnir` (S1):

```cpp
// the SNIR of a time interval inside the reception; an interval with startTime == endTime gives the value at that time
virtual double getMin(simtime_t startTime, simtime_t endTime) const = 0;
virtual double getMax(simtime_t startTime, simtime_t endTime) const = 0;
virtual double getMean(simtime_t startTime, simtime_t endTime) const = 0;
```

`SnirReceiverBase` (S1) overloads the name of the existing question with a time interval (D18). The function of `IReceiver` calls the overload with the limits of the reception, so it keeps its values:

```cpp
virtual bool computeIsReceptionSuccessful(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part,
        const IInterference *interference, const ISnir *snir, simtime_t startTime, simtime_t endTime) const;
```

`IErrorModel`:

```cpp
class INET_API IErrorModel : public virtual IPrintableObject
{
  public:
    enum class CorruptionMode { CM_UNDEFINED = -1, CM_PACKET, CM_CHUNK, CM_BYTE, CM_BIT };   // moved from ErrorModelBase (Q2)

    virtual CorruptionMode getCorruptionMode() const = 0;                                      // Q2

    // unchanged, also for ErrorRateInd
    virtual double computePacketErrorRate(const ISnir *snir, IRadioSignal::SignalPart part) const = 0;
    virtual double computeBitErrorRate(const ISnir *snir, IRadioSignal::SignalPart part) const = 0;
    virtual double computeSymbolErrorRate(const ISnir *snir, IRadioSignal::SignalPart part) const = 0;

    // the rates of a region of the HEADER or DATA part (P1): the region has `length` content bits,
    // and the transmitter sends it from startTime to endTime
    virtual double computePacketErrorRate(const ISnir *snir, IRadioSignal::SignalPart part,
            b length, simtime_t startTime, simtime_t endTime) const = 0;       // at least one error in the region
    virtual double computeBitErrorRate(const ISnir *snir, IRadioSignal::SignalPart part,
            b length, simtime_t startTime, simtime_t endTime) const = 0;       // the error rate of one bit of the region

    // the representation: applies the stored failed regions; replaces computeCorruptedPacket(snir) (Q2, Q3)
    virtual Packet *computeCorruptedPacket(const Packet *packet, const std::vector<ErrorRegion>& regions) const = 0;
};
```

`FlatReceiverBase`, protected:

```cpp
// the bits of the HEADER or DATA part; no value if the packet does not hold the complete part (P2)
virtual std::optional<PartRange> computePartRange(const ITransmission *transmission, IRadioSignal::SignalPart part) const;
// the time in which the transmitter sends the bits [offset, offset + length) of a part of partLength bits (P2)
virtual std::pair<simtime_t, simtime_t> computeRegionInterval(const IReception *reception, IRadioSignal::SignalPart part,
        b offset, b length, b partLength) const;
// decides each region of the part one time; true if no region failed (P3); Q3 adds the last parameter
virtual bool computeIsPartSuccessful(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part,
        const IInterference *interference, const ISnir *snir, std::vector<ErrorRegion>& errorRegions) const;
```

`ErrorModelBase`, protected (Q2): `computeCorruptedChunk(packet, region)` gives the corrupted form of one failed region in the `chunk`, `byte`, and `bit` modes. `corruptUnits(numUnits, unitErrorRate, corruptUnit)` flips the units of one failed region.

These functions go away:

- `IErrorModel`: `computeCorruptedPacket(snir)`.
- `ErrorModelBase`: `corruptPacket()`, `corruptChunks()`, `corruptBytes()`, `corruptBits()`, `computeCorruptedPacket(packet, ber)`, `hasProbabilisticError()`, and the retry loop.
- `FlatReceiverBase`: the override of `computeIsReceptionSuccessful()` with its error-model draw.
- `Ieee80211ErrorModelBase`: the refusal of the non-packet modes.

### 4.4 The reception steps

The example is an IEEE 802.11 PPDU with the PHY header chunk H, the PSDU chunks D1, D2, D3, and the tail and pad chunk T. The steps are the same in all modes. Only the regions of 3b and 3c and the corruption of step 4 depend on the mode.

| Step | When | Component | Action | Stored |
| --- | --- | --- | --- | --- |
| 1 | the transmission starts | `RadioMedium::addTransmission()` | Computes the arrivals, and removes the cached interference, noise, and SNIR of the overlapped receptions (#1446). | arrival, listening |
| 2 | the arrival starts | `Radio::startReception()` | Decides whether to attempt the reception. | nothing |
| 3 | the arrival ends | `Radio::endReception()` → `RadioMedium::getReceptionDecision(WHOLE)` | Computes the interference, noise, and SNIR, and asks the receiver for the WHOLE decision. | interference, noise, SNIR |
| 3a | same time | `FlatReceiverBase`, HEADER decision | Does the threshold check of the preamble interval. No draw (D7). | — |
| 3b | same time | `FlatReceiverBase`, HEADER decision | Decides the regions of the header. | HEADER decision with its failed regions |
| 3c | same time | `FlatReceiverBase`, DATA decision | Only if the HEADER decision is successful: decides the regions of the PSDU. | DATA decision with its failed regions |
| 3d | same time | `FlatReceiverBase`, WHOLE decision | Takes the success flag and the failed regions of the DATA decision. No draw. | WHOLE decision with the failed DATA regions |
| 4 | same time | `RadioMedium::receivePacket()` → `FlatReceiverBase::computeReceivedPacket(snir, decisions)` | Gives the failed regions of the WHOLE decision that it receives to `computeCorruptedPacket()`. Adds the tags. | the result, until `receivePacket()` copies it |
| 5 | same time | `Radio`, then the MAC | Decapsulates and sends up. The MAC checks `hasBitError()`. | — |

| Mode | 3b, 3c: regions and draws (receiver) | 4: corruption |
| --- | --- | --- |
| `packet` | one region for each part (H; D1+D2+D3): the threshold check, then one draw | a copy with a packet-level bit error; no draws |
| `chunk` | one region for each chunk (H; D1, D2, D3): the threshold check, then one draw each | an incorrect copy of each failed chunk; no draws |
| `byte`, `bit` | the same as `chunk` | each failed chunk becomes a raw chunk with flipped units, marked incorrect; draws in the error model |

With separate parts, the radio asks for the HEADER decision at the end of the header and for the DATA decision at the end of the data. The draws stay the same.

### 4.5 Pseudo code

```cpp
const IReceptionDecision *FlatReceiverBase::computeReceptionDecision(listening, reception, part, interference, snir) const
{
    if (errorModel == nullptr)
        return NarrowbandReceiverBase::computeReceptionDecision(...);         // unchanged (A10)
    bool possible = computeIsReceptionPossible(listening, reception, part);
    bool attempted = possible && computeIsReceptionAttempted(listening, reception, part, interference);
    bool successful = false;
    std::vector<ErrorRegion> errorRegions;
    if (attempted) {                                                          // no draw without an attempt (A6)
        switch (part) {
            case PREAMBLE:                                                    // the threshold check only (D7)
                successful = isPreambleSuccessful(...);
                break;
            case HEADER:
                successful = isPreambleSuccessful(...) && computeIsPartSuccessful(..., HEADER, ..., errorRegions);
                break;
            case DATA:                                                        // the header rule (A4, A14); the decision comes from the cache
                successful = medium->getReceptionDecision(radio, listening, transmission, HEADER)->isReceptionSuccessful()
                        && computeIsPartSuccessful(..., DATA, ..., errorRegions);
                break;
            case WHOLE: {                                                     // no draw; keeps the regions that the result applies
                auto data = medium->getReceptionDecision(radio, listening, transmission, DATA);
                successful = data->isReceptionSuccessful();
                errorRegions = data->getErrorRegions();
                break;
            }
        }
    }
    return new ReceptionDecision(reception, part, possible, attempted, successful, errorRegions);
}

// isPreambleSuccessful: true for a preamble without duration, else
// computeIsReceptionSuccessful(listening, reception, PREAMBLE, interference, snir, preambleStartTime, preambleEndTime)

bool FlatReceiverBase::computeIsPartSuccessful(listening, reception, part, interference, snir, errorRegions) const
{
    auto range = computePartRange(transmission, part);
    if (!range)
        return false;                                                         // the packet does not hold the complete part (A4)
    bool successful = true;
    // one region for a part without content in the packet and for the packet mode, else the chunks of the part
    for (auto [offset, length] : regions(transmission, *range, errorModel->getCorruptionMode())) {
        auto [startTime, endTime] = computeRegionInterval(reception, part, offset in the part, length, range->length);
        bool failed;
        if (!computeIsReceptionSuccessful(listening, reception, part, interference, snir, startTime, endTime))
            failed = true;                                                    // the threshold check: a condition, no draw
        else {
            double rate = errorModel->computePacketErrorRate(snir, part, length, startTime, endTime);
            failed = rate == 1 || (rate != 0 && !(dblrand() > rate));         // one draw
        }
        if (failed) {
            successful = false;
            if (range->isInPacket)                                            // Q3
                errorRegions.push_back({offset, length, errorModel->computeBitErrorRate(snir, part, length, startTime, endTime)});
        }
    }
    return successful;
}

// an overload of ReceiverBase, which computeReceptionResult() calls with the decisions that it receives (Q1, Q3)
Packet *FlatReceiverBase::computeReceivedPacket(snir, decisions) const
{
    const auto& regions = the failed regions of the WHOLE decision in decisions;
    if (regions.empty())                                                     // also without an error model
        return ReceiverBase::computeReceivedPacket(snir, decisions);         // the transmitted packet, with a bit error if a decision failed
    packet = errorModel->computeCorruptedPacket(transmittedPacket, regions);
    clear the tags and add the protocol tag, as ReceiverBase::computeReceivedPacket() does;
    return packet;
}

Packet *ErrorModelBase::computeCorruptedPacket(const Packet *packet, const std::vector<ErrorRegion>& regions) const
{
    // the caller owns the received packet, so the function does not enter the module
    copy = packet->dup();
    if (corruptionMode == CM_PACKET) {                     // a packet-level bit error
        copy->setBitError(true); return copy;
    }
    content = a new SequenceChunk;
    position = b(0);
    for (const auto& region : regions) {                   // the content between the regions stays as it is
        if (region.offset > position)
            content->insertAtBack(packet->peekAt(position, region.offset - position, flags));
        content->insertAtBack(computeCorruptedChunk(packet, region));
        position = region.offset + region.length;
    }
    if (position < packet->getTotalLength())
        content->insertAtBack(packet->peekAt(position, packet->getTotalLength() - position, flags));
    copy->replaceAll(content, flags);
    return copy;
}

Ptr<const Chunk> ErrorModelBase::computeCorruptedChunk(const Packet *packet, const ErrorRegion& region) const
{
    switch (corruptionMode) {
        case CM_CHUNK:                                     // an incorrect copy, no draws
            chunk = packet->peekAt(region.offset, region.length, flags);
            corrupted = chunk->dupShared(); corrupted->markIncorrect(); return corrupted;
        case CM_BYTE:                                      // the bytes of the region; bits if the length is not whole bytes (D13)
        case CM_BIT:
            units = the bytes or bits of packet->peekAt<BytesChunk or BitsChunk>(region.offset, region.length, flags);
            unitErrorRate = region.bitErrorRate for bits, and 1 - (1 - region.bitErrorRate)^8 for bytes;
            corruptUnits(units.size(), unitErrorRate, [&] (int i) { flip units[i]; });
            raw = a new BytesChunk or BitsChunk of units; raw->markIncorrect();
            return raw;
    }
}

// Flips the units of a failed region with the unit error rate p. The region failed, so at least one unit
// is flipped: the positions are sampled under that condition, without a retry loop.
void ErrorModelBase::corruptUnits(int n, double p, const std::function<void(int)>& corruptUnit) const
{
    if (p >= 1)
        corruptUnit(i) for every unit;                     // no draws
    else if (p <= 0)
        corruptUnit(intuniform(0, n - 1));                 // the region failed by the threshold check, and its rate is 0
    else {
        double regionRate = -expm1(n * log1p(-p));         // at least one error in the region; can round to 1
        // the first error, in [0, n), under the condition of at least one error
        double k = min(floor(log1p(-dblrand() * regionRate) / log1p(-p)), n - 1);
        while (k < n) {
            corruptUnit(k);
            k += 1 + floor(log1p(-dblrand()) / log1p(-p)); // the geometric gap to the next error
        }
    }
}
```

P3 has this decision code with one region for each part and without the failed regions. Q3 adds the regions of the mode and the failed regions.

The corruption uses the bit error rate of the region, not its region rate (D11). The region rate of a long chunk rounds to exactly 1 above about 37 expected errors. A unit rate that comes from such a region rate is 1, and all units flip. The bit error rate keeps its value, so a failed chunk of 8000 bits with a bit error rate of 0.5 % gets about 40 flips. The sampling is exact for independent unit errors. For a unit rate between 0 and 1, it draws one number more than the number of flipped units.

### 4.6 Region error rates of the models (P1)

Each region stands for its share of its part: its share of the time, and its share of the bits that the whole-part rate of the model counts (D6). N is the number of bits that the whole-part rate counts for the part, and L is the number of content bits of the part in the packet. A region of `length` content bits thus counts `length`·N/L bits. This rule gives A13 for each division of a part into regions.

| Model | HEADER region rate | DATA region rate |
| --- | --- | --- |
| `StochasticErrorModel` | With `packetErrorRate`: 1 − (1 − PER)^(region duration / (header duration + data duration)). Otherwise: 1 − (1 − BER)^length. | Same |
| `ApskErrorModel`, `Ieee802154ErrorModel` | 1 − (1 − BER)^length, with the BER of the modulation at the region SNIR. N is equal to L. | Same |
| `Ieee80211ErrorModelBase` (NIST, Yans) | 1 − h^(length / L). h is `getHeaderSuccessRate()` for the header length of the mode at the region SNIR, and L is the length of the PHY header chunk. | 1 − s^(length·N / L). s is `getDataSuccessRate()` for one bit at the region SNIR, N is the data length that the whole-part rate counts, and L is the PSDU length. |
| `Ieee80211BerTableErrorModel` | 0, as in its whole-part model | 1 − (1 − p)^(length / L), with a maximum of 1. p is the table PER for the PSDU length at the region SNIR. |

The region bit error rate is the rate of one content bit: 1 − (1 − region rate)^(1 / length). Each model computes it from its own values, not from a rounded region rate: the BER of the modulation, 1 − s^(N / L), or the exponent of its region rate divided by `length`.

The region rate of a PREAMBLE or WHOLE part is a usage error. `getDataSuccessRate()` of NIST and Yans no longer adds the SERVICE bits. `Ieee80211ErrorModelBase` adds them for OFDM modes, so the whole-part rates keep their values. The DATA formula needs a success rate that is a power of the bit count. A13 has a test for each IEEE 802.11 mode family (R6).

The generic `computeRegionInterval()` divides the time of a part in proportion to the content bits. For IEEE 802.11, the DATA part also carries the SERVICE, tail, and pad bits, so the map is at most one OFDM symbol from the real position. The mode objects give the part times, so the map adds no duration formula ([AR-WLAN-PHY-AUTHORITY](../../doc/project/domain/ieee80211.md#ar-wlan-phy-authority)).

### 4.7 Alternatives that this plan does not use

- **The received content in the decision (draft 2):** the decision of each part kept the received chunks of the part. The decision then changes the representation, and the modes do not follow one rule.
- **Draws at the result:** the decision and the content would be two samples that can disagree, as in the byte and bit modes of master.
- **One draw for the whole reception in the packet mode (D1 of draft 2):** the packet mode would stay an exception, and with separate parts it would draw for each asked part. The maintainer chose the uniform rule and the baseline update.
- **Units as regions in the byte and bit modes:** a decision for each byte or bit would need a threshold check and an SNIR for each unit. The region is the chunk, and the corruption samples the units.
- **Random units after a threshold failure:** the flips would not follow the error rate (D11).
- **The approach of PR #1363:** only HT aggregates get chunk errors, with the SNIR of the whole reception.
- **The region rate in the failed region (draft 3):** the rate of a long region rounds to 1, and then all its units flip (D11).
- **A structure for the errors of a part (draft 3):** `PartErrors` held two flags that together repeat the success flag of the decision (D12).
- **A threshold function with its own name (draft 3):** `computeIsSnirAboveThreshold()` tells the caller how `SnirReceiverBase` decides, and repeats the mode switch of `computeIsReceptionSuccessful()` (D18).
- **Regions that count only content bits (D6 of draft 3):** the SERVICE, tail, and pad bits would be in no draw. An ACK at 6 Mbps would draw against 112 bits and not 144, so the packet mode would lose fewer frames than master.
- **A region rate without a length:** a rate of a time interval alone cannot give the bit error rate of a region.
- **The regions from the error model:** the receiver would not read the corruption mode. But a PHY must be able to refine the regions, for example to MPDUs, and the receiver is specific to the PHY.

### 4.8 Speed

This section is an estimate from the code. Nothing was measured.

| Mode | Work for each received frame | Expected effect |
| --- | --- | --- |
| `packet` | Master: one SNIR value of the whole reception, which the threshold and the rate share, and one rate. Draft 3: about five interval SNIR values (the preamble, and the threshold and the rate of each part), two rates, up to two draws, and two more decisions in the cache. | Small with the scalar model, because an interval value is a maximum over a noise function with few steps. Visible with the dimensional model, because each interval value partitions the SNIR function. |
| `chunk` | Two interval values and one rate for each chunk. A frame without errors costs only a copy at the result. | Small with the scalar model; with many chunks and the dimensional model, visible |
| `byte`, `bit` | Master serializes the whole packet, draws once for each unit, and repeats this until some error occurs, on average 1 / (1 − (1 − BER)^n) times. Draft 3 serializes only the failed chunks, and draws once for each flip, plus one. | Much faster. A small BER after a threshold failure no longer almost stops the simulation. |

Two changes remove most of the extra cost (D14). Neither change moves a result:

- **S1:** `ScalarSnir` and `DimensionalSnir` keep the values of the last interval query. The threshold and the rate of a region ask for the same interval, and both `snirThresholdMode` and `snirMode` are `"min"` by default. Thus the second query uses the kept value.
- **#1446:** the removal of the cached SNIR in `RadioMedium::addTransmission()` goes through all transmissions of the cache for each radio. In the default mode, it never finds a cached SNIR, but it still costs one lookup for each radio and live transmission. The loop thus asks `ICommunicationCache::computeInterferingTransmissions(radio, startTime, endTime)`. In `CommunicationCacheBase`, that function uses the interval tree of the radio. The loop keeps its strict overlap check. `setCachedInterval()` already put the new transmission into the tree, so the loop still skips it.

## 5. Affected components and compatibility

**Simulation results:** S1 changes no result. P3 decides the header and the data separately in all modes, each with the SNIR of its own time. At a constant SNIR, the reception probability stays the same (A13); the number of the draws changes. Thus the fingerprints and statistics of every wireless configuration with errors change (P3). P3 also changes the cache counters of the radio medium in configurations without errors, because the radio medium computes three decisions for each reception instead of one. Q3 changes the results of the `chunk`, `byte`, and `bit` modes again. A statistics baseline gets an update only if its test passes before the change (D17).

**C++ API:** an implementation outside INET must follow these changes. `ISnir` gets three pure virtual functions (S1). `IErrorModel` gets the region overloads of `computePacketErrorRate()` and `computeBitErrorRate()` (P1). `IReceptionDecision` gets `getErrorRegions()`, and `ReceptionDecision` gets a constructor parameter (Q1). `IErrorModel` gets `getCorruptionMode()` and `computeCorruptedPacket(packet, regions)`, and `CorruptionMode` moves to it (Q2). The functions of section 4.3 go away (P3, Q3).

**Configuration:** `corruptionMode` keeps its values and its default. IEEE 802.11 supports all four values after Q3.

**Documentation:** each changing commit carries its release note under `doc/whatsnew/`. Each pull request has its own section file in `doc/src/migration-guide/development/`: `physicallayer-snir-interval.rst`, `physicallayer-part-error-decisions.rst`, and `physicallayer-content-errors.rst`. The users guide section "Error Models" in [ch-physicallayer.rst](../../doc/src/users-guide/ch-physicallayer.rst) describes the rule of section 1.

**PR #1363:** after pull request 3, the PR can drop its commit `4d9ddb1fe6` and the generic part of `eb7e4157e3`, and set `corruptionMode = "chunk"`.

## 6. Future commits and dependencies

Four repairs that this series needs are pull requests of their own (D16). They landed on master before this series; master `91219b0fa7` contains them:

| Pull request | Commits | Why this series needs it |
| --- | --- | --- |
| #1445 | `ieee80211: fix: do not count the signal field of HT and VHT in the preamble` | The region times of HT and VHT need correct part durations. |
| #1446 | `physicallayer: fix: compute the SNIR of a reception again after a new interferer`, `physicallayer: change: compute each part decision once` | The decisions of separate parts need an SNIR that contains the later interferers, and one sample of each part. |
| #1447 | `AckingMac: fix: check the bit error before the header is read` | The chunk mode marks the MAC header chunk incorrect, and AckingMac then stops the simulation. |
| #1448 | `physicallayer: fix: initialize the base of the stochastic and BER table error models` | The tests and the BER table model need the corruption mode of the model. |

The work itself is three pull requests, each on the one before it (D16). Each pull request is useful alone ([PR-REQ-TOPIC](../../doc/project/rule/pull-request.md#pr-req-topic)):

| Pull request | Branch | What it gives alone | Baselines that move |
| --- | --- | --- | --- |
| 1 | `topic/physicallayer-snir-interval` | The SNIR and the threshold check of a time interval, for each model of a part of a signal | none |
| 2 | `topic/physicallayer-part-error-decisions` | The header and the data are decided separately, each with the SNIR of its own time. A late interferer no longer fails the header. | the fingerprints, statistics, and charts of the wireless configurations with errors |
| 3 | `topic/physicallayer-content-errors` (#1443) | Errors in place for the `chunk`, `byte`, and `bit` modes, also for IEEE 802.11. PR #1363 needs this one. | the fingerprints of `examples/wireless/corruptionmode` |

| Commit and proposed subject | Pull request | Dependencies |
| --- | --- | --- |
| `S1: physicallayer: add: SNIR of a time interval` | 1 | none |
| `P0: plan: add: define error decisions and corruption for packet-level reception` | 2 | none |
| `P1: physicallayer: add: error rates of signal regions` | 2 | S1 |
| `P2: physicallayer: add: packet ranges and region times of signal parts` | 2 | P0 |
| `P3: physicallayer: change: decide the header and the data of a reception separately` | 2 | S1, P1, P2 |
| `P4: plan: change: record the results of the part decisions` | 2 | P3 |
| `Q1: physicallayer: add: failed regions of a reception decision` | 3 | P3 |
| `Q2: physicallayer: add: corruption of failed signal regions` | 3 | Q1 |
| `Q3: physicallayer: change: decide the errors of each region before the corruption` | 3 | Q1, Q2 |
| `Q4: plan: change: record the results of the error decision series` | 3 | Q3 |
| `Q5: plan: location: move the error decision plan to done` | 3 | Q4 |

S1, P1, P2, Q1, and Q2 only prepare. P3 and Q3 are the only commits that move behavior. Q3 extends `computeIsPartSuccessful()` of P3 from one region for each part to the regions of the mode. No other commit changes code that an earlier commit of this plan adds. Each commit marks its entry done when it is complete.

These commits change the model of the physical layer, not a protocol exchange, so protocol tests and protocol reviewer documents do not apply. Unit tests and module tests check the mechanism at the PHY boundary ([AR-WLAN-QUAL-TESTS](../../doc/project/domain/ieee80211.md#ar-wlan-qual-tests)).

### S1 (pull request 1)

Status: done. Purpose and scope: the interval overloads of `ISnir` in `ScalarSnir`, `DimensionalSnir`, and `UnitDiskSnir`. An interval with the same start and end gives the value at that time (D23). The whole-reception getters keep their cached values. `DimensionalSnir` keeps its SNIR function, so that many interval queries do not divide the functions again. `ScalarSnir` and `DimensionalSnir` also keep the values of the last interval query (section 4.8). The interval overload of `SnirReceiverBase::computeIsReceptionSuccessful()`, which the function of `IReceiver` calls with the limits of the reception (D18). Release note `doc/whatsnew/incompatible/physicallayer-snir-interval.txt` and the migration guide section. The pull request has one commit and no plan, as the four repairs.

Verification: a new module test `tests/module/SnirInterval_1.test` runs one reception with an interferer over its last part, with the scalar and the dimensional analog model. It prints the minimum, maximum, and mean SNIR of an interval before the interferer, an interval across its start, an interval inside it, and one point of time. It also prints the threshold check of each interval. The fingerprints must keep all values; this checks that the function of `IReceiver` keeps its values.

### P0 (pull request 2)

P0 adds this plan. It changes no source.

### P1

Status: done. Purpose and scope: the region overloads of `IErrorModel::computePacketErrorRate()` and `computeBitErrorRate()`, the interval `ErrorModelBase::getScalarSnir()`, the protected `checkRegionPart()`, the region rates of section 4.6, and the SERVICE move. Each model gives only the bit error rate of a region, and `ErrorModelBase` computes the region rate from it, so the two rates agree by construction. `Ieee80211ErrorModelBase::computeDataLength()` gives the bits that the data part counts, for the whole-part rate and for the region rates. No receiver calls the region rates yet. Release note `doc/whatsnew/incompatible/physicallayer-region-error-rates.txt` and the migration guide section.

Verification:

- A new unit test `tests/unit/RegionErrorRates_1.test` uses a small subclass of `StochasticErrorModel` with a fixed BER, without a network. It checks that the region rate is 1 − (1 − BER)^length, that the limits of the BER give 0 and 1, that the two region rates agree, that a region of 8000 bits with a BER of 0.5 % has the rate 1 and keeps its bit error rate, and that the PREAMBLE and WHOLE parts have no regions.
- A new module test `tests/module/RegionErrorRates_2.test` checks A13. At the end of a reception, a checker gives the error model of the receiver an SNIR with one value at all times, for 141 values from −20 dB to 50 dB. It divides the header and the data into 1, 2, and 7 regions, and compares the product of (1 − region rate) with 1 − the whole-part rate: the difference must be at most 1e-9, and the relative difference of the success rates at most 1e-6. Its receivers: APSK with a preamble, the stochastic model with `packetErrorRate` and a preamble, IEEE 802.15.4, and IEEE 802.11 with DSSS, HR/DSSS, ERP-OFDM, HT, and VHT modes for NIST, ERP-OFDM and HT for Yans, and the BER table model. All of them give no difference, so the success rates of all these modes are powers of the bit count (R6). The stochastic model with only `bitErrorRate` has no run, because its whole-part rate stops (F5).
- The fingerprints must keep all values; this checks the SERVICE move.

### P2

Status: done. Purpose and scope: `FlatReceiverBase::PartRange`, `computePartRange()`, which refuses by default, and `computeRegionInterval()`, without a caller. The part ranges of `ApskReceiver` (the header and the data with the padding), `Ieee802154NarrowbandReceiver` (a header that the packet does not hold, and the whole packet as the data), and `Ieee80211Receiver` (the PHY header chunk, and the PSDU that the length field gives; a PSDU longer than the remaining content gives no value). A part without bits has no region. The migration guide section.

Verification: the fingerprints must keep all values. The module tests of P3 check the ranges and the region times.

### P3

Status: done. Purpose and scope: `FlatReceiverBase::computeReceptionDecision()` with the chain of the parts and `computeIsPartSuccessful()` (section 4.5), with one region for each part in all four modes. The decision draws only for an attempted reception. Remove the override of `computeIsReceptionSuccessful()` with its error-model draw. The corruption of a failed reception stays as on master until Q3. Update all fingerprints that move. Release note `doc/whatsnew/incompatible/physicallayer-part-error-decisions.txt`, the users guide paragraph, and the migration guide section.

Verification:

- A new module test `tests/module/PartErrorDecisions_ApskScalarRadio_1.test` sends one UDP packet with `ApskScalarRadio` and prints the decisions and the draws of the receiver. Its runs: late interference, with and without separate parts; interference over the header; an error model with the rate 0 for the header and 0.5 for the data, without and with separate parts; and a reception that the receiver does not attempt, which must draw nothing. For the last run, the listener asks for the decision of the second frame when its arrival starts, because the attempted flag depends on the time of the query.
- A new module test `tests/module/PartErrorDecisions_Ieee80211Radio_1.test` sends a PSDU without a MAC, so that no backoff or retry changes the timing. It runs ERP-OFDM, HT, and HR/DSSS modes, the NIST, Yans, and BER table models, and the dimensional radio with late interference, and ERP-OFDM without interference and with interference from the header on.
- The full fingerprint run shows which configurations move. Each moved configuration must be a wireless configuration with errors in the reception, or a configuration of `examples/wireless/corruptionmode`.
- The statistical comparison of section 7 (D22).

Implementation facts: `ErrorModelBase` computes the region rate as 1 − (1 − BER)^length in the numerical form of the whole-part rates, so a very small bit error rate gives exactly 0 and draws no number, as on master. When the header rate is exactly 0, the data draws the same random number against the same rate as the whole signal on master. Thus most IEEE 802.11 configurations keep their fingerprints, and P3 moves 147 fingerprints of 49 configurations, where draft 3 moved 217 of 78.

### P4

Status: done. P4 records the executed results of pull requests 1 and 2 in section 9. The plan stays in `plan/pending/`.

### Q1 (pull request 3)

Status: done. Purpose and scope: `ErrorRegion` in its own header, `IReceptionDecision::getErrorRegions()`, the regions of `ReceptionDecision`, and the overload `ReceiverBase::computeReceivedPacket(snir, decisions)`, which `computeReceptionResult()` calls and which gives the same packet as before. No decision has a region yet. Release note and the migration guide section.

Verification: the fingerprints must keep all values.

### Q2

Status: open. Purpose and scope: the error model side of the region decisions, without a caller. The enum `CorruptionMode` moves to `IErrorModel`, and `IErrorModel` gets `getCorruptionMode()` and `computeCorruptedPacket(packet, regions)`. `ErrorModelBase` gives a packet-level bit error in the `packet` mode, an incorrect copy of each failed chunk in the `chunk` mode, and flipped bytes or bits in the `byte` and `bit` modes, through the protected `computeCorruptedChunk()` and `corruptUnits()` (sections 4.5 and D11, D13). The old `computeCorruptedPacket(snir)` stays until Q3. Release note and the migration guide section.

Verification: a new unit test `tests/unit/RegionCorruption_1.test` checks that `computeCorruptedPacket()` in the `chunk` mode marks exactly the chunks of the given regions, that the content between the regions keeps its bytes and flags, and that the `packet` mode only sets the packet-level bit error. A new module test `tests/module/UnitCorruption_1.test` runs `corruptUnits()` in a small network of two error models and a tester, because the error model draws from the random number generator of its module:

- With the unit rate 1, every unit flips and nothing is drawn.
- With the unit rate 0, exactly one unit flips.
- With other rates, each sample flips at least one unit, and the positions stay in the region.
- Over 10000 samples, the mean number of flips is within 2 % of n·p / (1 − (1 − p)^n). One case has 8000 units and p = 0.005, where the region rate rounds to 1.
- In the byte and bit modes, a failed region becomes a raw chunk with the flipped units, marked incorrect; a region that is not whole bytes uses bits in the byte mode; the input stays unchanged.

The fingerprints must keep all values.

### Q3

Status: open. Purpose and scope: `computeIsPartSuccessful()` decides one region for each chunk in the `chunk`, `byte`, and `bit` modes and gives the failed regions that the packet holds, with their bit error rates. The decisions keep them, and the override `FlatReceiverBase::computeReceivedPacket(snir, decisions)` gives the regions of the WHOLE decision to the error model. Remove the old path: `IErrorModel::computeCorruptedPacket(snir)`; `corruptPacket()`, `corruptChunks()`, `corruptBytes()`, `corruptBits()`, `computeCorruptedPacket(packet, ber)`, `hasProbabilisticError()`, and the retry loop of `ErrorModelBase`; the `bool` override of `FlatReceiverBase::computeReceivedPacket()`; the refusal of `Ieee80211ErrorModelBase`. Update the fingerprints that move. Release notes, the users guide paragraph, and the migration guide section.

Verification:

- A new module test `tests/module/ContentErrors_ApskScalarRadio_1.test` prints the flag of each chunk of the received frame and the draws of the receiver. Its runs: the chunk mode under late interference, with and without separate parts; interference over the header; an error model that gives the rates 0, 1, and 0.5 to the regions in turn, without and with separate parts; and the byte and bit modes under late interference. In the byte and bit modes, the number of flipped units must be below the number of units.
- A new module test `tests/module/ContentErrors_Ieee80211Radio_1.test` sends a PSDU of three chunks without a MAC. It runs ERP-OFDM, HT, and HR/DSSS modes, the NIST, Yans, and BER table models, the dimensional radio, and the chunk, byte, and bit modes. The PSDU bound has no test, because `TapRadioMedium` changes received packets after the decisions.
- A new module test `tests/module/ConcurrentRadio_7.test` runs two overlapping receptions in the chunk mode with the dimensional radio.
- The reception tests check that no module deletes a packet that it does not own.
- The full fingerprint run must move only the configurations of `examples/wireless/corruptionmode`.

### Q4 and Q5

Q4 records the executed results, the design facts, and the gaps. Q5 moves the plan to `plan/done/` without a content change ([PR-SPLIT-MOVE](../../doc/project/rule/pull-request.md#pr-split-move)).

## 7. Verification methods and expected results

Follow [run-the-gates.md](../../doc/project/guide/run-the-gates.md). Each pull request has its own worktree, with the name of its branch. Build in that worktree with its own environment, and make sure that `INET_ROOT` points to it. A fresh debug build is necessary after each commit. The proposed commands run from the worktree root:

```sh
make MODE=debug -j8
opp_run_unit_tests -m debug
opp_run_module_tests -m debug
opp_run_protocol_tests -m debug
opp_run_fingerprint_tests -m debug
```

Each commit must build alone. A full fingerprint run follows S1, P2, P3, Q2, and Q3. Only P3 and Q3 can move fingerprints, and only the configurations that their messages name. The protocol suite must give the same unexpected failures as master. On master `91219b0fa7`, the protocol suite has 67 unexpected failures, `examples/ipv6/mipv6 -c Handover` fails its fingerprint, and 22 fingerprint configurations need disabled features. Update the moved fingerprints with the procedure of [change-a-baseline.md](../../doc/project/guide/change-a-baseline.md). A release build runs on S1, P3, and Q3. `check-pull-request.sh` runs on each pull request with its description. No step measures the speed; section 4.8 is an estimate.

The statistical and chart suites run in the copy of the GitHub job on P3 and Q3. Each baseline that moves gets an update only if its test passes on the parent (D17). The statistics and chart commits must reach the masters of their repositories together with their pull request (R3).

The statistical comparison checks that P3 keeps the distributions (D22). It runs one time, with release builds of master and of P3 in the copy of the GitHub job, and its files stay outside the repository:

- **E1, no interference:** one transmitter sends 20000 broadcast UDP packets of 100 bytes to one receiver, at a distance that gives a loss ratio of about 30 %. The runs: IEEE 802.11 ERP-OFDM at 6 Mbps and at 54 Mbps with the NIST model, HR/DSSS at 11 Mbps, and `ApskScalarRadio`. Expected: the loss ratio of P3 is equal to the loss ratio of master within three standard deviations of the binomial distribution.
- **E2, an interferer over the end of each frame:** a hidden transmitter overlaps the last third of each frame. Expected: with the `"min"` SNIR modes, the loss ratios are equal, because the minimum SNIR of the data is the minimum SNIR of the whole signal. With the `"mean"` modes, they differ, because the data no longer sees the SNIR of the preamble and the header. The record gives both ratios.

## 8. Risks, unresolved decisions, and approval scope

### Decisions

- **D1, the packet mode (changed in draft 3, agreed 2026-11-09):** the packet mode follows the uniform rule: one region for each part, and no draw for the WHOLE decision. Draft 2 kept the single draw for the whole reception. The cost is the update of the baselines of every wireless configuration with errors.
- **D2, a generic API (kept):** the threshold and the rate apply to each region with the SNIR of its time. A PHY can refine the range, the time map, and the threshold check. Draft 3 states that the threshold check is a condition, not a probability.
- **D3, the sealed packet API (done):** the repair landed on master as `7ec0d4d3f3`.
- **D4, the byte and bit modes (changed in draft 3):** they join the uniform rule: P3 for the decisions, and Q3 for the corruption. Draft 2 kept their old behavior.
- **D5, the random number generators (changed in draft 3):** the decisions draw in the generator of the receiver. The byte and bit corruption draws in the generator of the error model, as master does.
- **D6, the SERVICE, tail, and pad bits (changed in draft 4):** each region carries its share of the bits that the whole-part rate counts (section 4.6). Draft 3 put them in no region; then the packet mode lost fewer frames than master at the same SNIR. The positions of these bits in time stay an estimate (F4).
- **D7, the preamble (kept):** only the threshold over its interval applies.
- **D8, the SNIR cache with separate parts (answered 2026-11-05):** repair it before this series, in #1446.
- **D9, the header check of AckingMac (answered 2026-11-05):** repair it before this series, in #1447.
- **D15, the HT and VHT part durations (answered 2026-11-09):** repair them before this series, in #1445.
- **D16, the order of the work (changed in draft 4):** the four repairs are pull requests of their own, each with its own regression test. The work is three pull requests: the SNIR of a time interval, the separate decisions of the header and the data, and the errors in place. In each pull request, the commits that prepare come first, and one commit moves the behavior. This order needs no temporary condition on the mode. Draft 3 had one pull request with one behavior commit for all four modes.
- **D10, incorrect flags in the byte and bit modes (new, agreed 2026-11-09):** a raw chunk with flipped units is marked incorrect, so that a declared checksum also finds the errors.
- **D11, the units of a failed region (changed in draft 4):** the flips follow the bit error rate of the region, and at least one unit is flipped. A threshold failure with the rate 0 flips one unit. Draft 3 computed the unit rate from the region rate, which rounds to 1 for a long region with many expected errors; then all units flipped.
- **D12, errors in the decision (changed in draft 4):** a part decision keeps the failed regions of its part, not its received content, and `IReceptionDecision` gives them. The success flag of the decision tells whether the part was received; draft 3 had a separate structure for that. The corruption of a region is a protected function of `ErrorModelBase`.
- **D13, regions that are not whole bytes (new, approved with draft 3):** in the `byte` mode, a failed region whose length is not a whole number of bytes uses its bits as units.
- **D14, speed (new, agreed 2026-11-09):** S1 keeps the values of the last interval query, and #1446 visits only the overlapping receptions (section 4.8). Neither change moves a result.
- **D17, statistics baselines (new, user rule 2026-11-09):** a statistics baseline gets an update only if its test passes on the parent commit. A test that already fails on master keeps its baseline, also if this series moves its results. Its update waits for the repair of the earlier failure, because an update now would hide that failure under the reason of this series.
- **D18, the threshold check (new in draft 4):** `SnirReceiverBase` overloads `computeIsReceptionSuccessful()` with a time interval, and the function of `IReceiver` calls the overload. The name gives the question, not the method, and one function holds the threshold rule.
- **D19, rate consistency (new in draft 4):** A13 is an acceptance criterion with its own test for each error model. `StochasticErrorModel` with `packetErrorRate` divides by the duration of the header and the data, without the preamble. The BER table model divides the table value of the PSDU among the regions.
- **D20, the chain of the parts (new in draft 4):** each part reads `isReceptionSuccessful()` of the part before it, and a decision draws only for an attempted reception (A6, A14).
- **D21, the range of a part (new in draft 4):** `computePartRange()` returns an optional `PartRange` value. Draft 3 had a bool result, two output parameters, and the offset −1 as a special value.
- **D22, the statistical comparison (new in draft 4):** the experiment of section 7 compares the loss ratios of master and P3. Draft 3 checked only which baselines moved.
- **D23, an interval of one point (new in draft 4):** the interval functions of `ISnir` give the value at that time. A region can be shorter than the resolution of the simulation time.
- **D24, exports (new in draft 4):** `ErrorRegion` has `INET_API`, as most structures of INET at namespace level.

### Risks

- **R1:** an operation after `decapsulate()` can read a part of a flagged chunk without `PF_ALLOW_INCORRECT`. The module tests run the whole reception path.
- **R2:** the full fingerprint run after P3 moves many configurations. A configuration without errors in the reception must not move; such a movement is a defect.
- **R3:** the statistical job of CI compares with the master of the statistics repository. Thus the statistics and chart commits of P3 and Q3 must reach those masters together with their pull requests. The tests that keep their old baselines under D17 fail before and after.
- **R4:** the WHOLE decision calls the radio medium during its own computation. `ReceiverBase` already does this for the attempted flag.
- **R5:** in the `byte` and `bit` modes, a chunk type without a serializer stops the simulation at its first failed region. Master has the same limit for the whole packet.
- **R6:** the DATA region rate of IEEE 802.11 needs a success rate that is a power of the bit count. If the A13 test fails for a mode, that mode needs another formula, and the plan needs a new decision.
- **R7:** the fingerprint values of P3 differ from the values of draft 3, because a decision no longer draws without an attempt and the rates count more bits. The statistics and chart commits of draft 3 are thus stale, and P3 makes them again.
- **R8:** the WHOLE and DATA decisions read the decision of the part before them from the cache of the radio medium. A caller outside the radio medium, such as the network configurator, must give its own WHOLE decision to `computeReceptionResult()`.

The earlier approval covered draft 3, the commits C0 to C7 of that draft, and the four pull requests #1445 to #1448. The approval of draft 4 covers its changed and new decisions and the commits S1, P0 to P4, and Q1 to Q5. It does not cover F1 to F7.

## 9. Results

Status: pull requests 1 and 2 are done on their branches, not merged. Pull request 3 is open. Master `91219b0fa7` contains the four repairs #1445 to #1448.

**Pull request 1, `topic/physicallayer-snir-interval` (S1).** All runs use OMNeT++ 6.x on master `91219b0fa7`:

- The debug and release builds have no warnings. The unit suite gives 153 PASS, and the module suite 842 PASS with `SnirInterval_1`.
- The protocol suite gives the same 67 unexpected failures as master, test by test. The fingerprint suite gives 1767 PASS, so S1 moves no fingerprint; `examples/ipv6/mipv6 -c Handover` fails as on master, and 22 configurations need features that the build does not have.
- `SnirInterval_1` gives 33.93 dB before the interferer and −12.04 dB under it, which agree with a calculation by hand. Without the rule for an interval of one point, those intervals give an infinite minimum and a mean that is not a number.
- `check-pull-request.sh` passes with the description. `check-source-seals.sh`, `check-architecture.sh`, and `check-naming.sh` pass, and `check-interfaces.sh` reports the same 16 old violations as master.

**Pull request 2, `topic/physicallayer-part-error-decisions` (P0 to P4).** It stands on S1:

- Each commit builds alone without warnings. The fingerprint suite on P2 gives 1767 PASS, so P1 and P2 move no fingerprint; this checks the SERVICE move.
- On P3, the debug and release builds have no warnings. The unit suite gives 154 PASS and the module suite 845 PASS, with `RegionErrorRates_1`, `RegionErrorRates_2`, and the two `PartErrorDecisions` tests. The protocol suite gives the same 67 unexpected failures as master. The fingerprint suite gives 1767 PASS with the updated store.
- A13: `RegionErrorRates_2` finds no difference for APSK, the stochastic model with `packetErrorRate`, IEEE 802.15.4, and IEEE 802.11 with DSSS, HR/DSSS, ERP-OFDM, HT, and VHT modes and the NIST, Yans, and BER table models. So the DATA formula holds for all these modes (R6).
- P3 moves 147 fingerprints of 49 configurations: 135 of 45 wireless configurations with a flat radio and an error model, and the 12 of the four configurations of `examples/wireless/corruptionmode`. The update tool also changed the three entries of `examples/ipv6/mipv6 -c Handover`, which fails on master; the store keeps their old values.
- Statistics and charts in the copy of the GitHub job (ubuntu 24.04, clang 16, release build, OMNeT++ `omnetpp-6.x` `66ab32fb91`, opp_repl `github_ci` `39e15bc`), with the rule D17. With P3, 141 statistical tests fail that pass on master: 97 change only in the cache counters of the radio medium, and 44 change their trajectory, all of them with moved fingerprints. The branch `topic/physicallayer-part-error-decisions` of the statistics repository updates them in `71f398c` and `8d75f17`. Then the suite gives 845 PASS and the same 81 unexpected failures as master. 6 charts fail that pass on master: two of coexistence, one of ieee802154, two of power, and BMac of sensornetwork. The same branch of the media repository updates them in `0c0005b`, and then the chart suite gives the 19 failures of master. These commits are local (R3).
- Not updated (D17): the fingerprints of `showcases/visualizer/canvas/networkpathactivity -c ChangingPaths`, `showcases/visualizer/canvas/spectrum -c PowerDensityMap`, and `tutorials/configurator -c Step9` move with P3, but their statistical tests already fail on master.
- E1 (D22), master against P3, 20000 broadcast packets in each run, a sweep of the distance for ERP-OFDM at 6 Mbps and 54 Mbps, HR/DSSS at 11 Mbps, and `ApskScalarRadio`: 58 of the 84 runs have a loss ratio between 0.5 % and 99.5 %. Their z values have the mean −0.01 and the standard deviation 0.80; the largest is 2.48, and one is above 2. At 54 Mbps, master and P3 give the same counts in all runs, because the header rate is exactly 0. So P3 keeps the loss ratios.
- E2 (D22), a hidden interferer over the last third of each ERP-OFDM frame at 54 Mbps, with six interferer powers: with the default `"min"` SNIR modes, master and P3 give the same counts in all runs. With the `"mean"` modes, P3 loses more frames, for example 45.9 % against 11.9 % with the strongest interferer, because the SNIR of the preamble and the header no longer raises the mean SNIR of the data. The release note of P3 tells this.
- `check-pull-request.sh` passes with the description; its one note asks about the two kinds of fingerprint runs of P3, and the P3 message answers it. The other gates give the results of pull request 1.

Facts found during pull requests 1 and 2:

- `ErrorModelBase` must compute the region rate in the numerical form of the whole-part rates, 1 − (1 − BER)^length. A form that keeps very small rates above 0 draws a number where master draws none, and moves fingerprints without a change of a probability.
- When the header rate is exactly 0, the data draws the same random number against the same rate as the whole signal on master. Thus most IEEE 802.11 configurations keep their fingerprints.
- The attempted flag of a decision depends on the time of the query: for the frame of an interferer, a query at the end of its arrival gives true, when the receiver no longer receives the first frame. A test of a reception without an attempt must ask at the start of the arrival.
- `StochasticErrorModel` needs all three of its rates, because the receiver reports the bit and symbol error rates of each reception (F5).
- Tests must not run in a worktree while a suite runs there: the single runs replace the log and the work folders of the suite.

**The implementation of draft 3.** PR #1443 holds it at `b703ae8ccf`, which the reference `refs/backup/physicallayer-content-errors-pr1443-v4` keeps. The record of its checks is in the plan of that commit. In short: each commit built alone; the unit, module, and protocol suites gave the results of master with the new tests; the behavior commit moved 217 fingerprints, 141 statistics baselines, and 6 charts. The earlier references `-draft2`, `-draft3`, and `-pr1443-v1` to `-pr1443-v3` stay until the landing. The statistics commit `e8f63b9` for #1446 is on the statistics master. The statistics commits `07c613c` and `155109b` and the chart commit `96688de` of draft 3 are local and stale (R7).

**The review of 2026-11-10.** Two reviews read the source of draft 3. Their findings, and the place of each repair:

| Finding | Repair |
| --- | --- |
| The byte and bit modes flipped all units of a long failed region, because the region rate rounds to 1. `examples/wireless/corruptionmode` showed it, and the record of draft 3 took it as a fact. | D11, Q2 |
| `StochasticErrorModel.packetErrorRate` was no longer the packet error rate: 0.5 gave 0.467 for a long IEEE 802.11a frame and 0.357 for an ACK. | D19, P1 |
| The DATA draws of IEEE 802.11 did not count the SERVICE, tail, and pad bits. | D6, P1 |
| The BER table model gave each short chunk the rate of the smallest table size. | D19, P1 |
| A decision drew also for a reception without an attempt; only the gate of `Radio` prevented it. | D20, P3 |
| A region shorter than the time resolution got an empty interval. | D23, S1 |
| `computeIsSnirAboveThreshold()`, `PartErrors`, the form of `computePartRange()`, and the casts to `ReceptionDecision`. | D18, D12, D21 |
| No check compared the distributions before and after the change. | A13, D22 |

The review found these areas clean: the SNIR interval functions, the part ranges, the corruption math below a rate of 1, the removed functions, the path of the network configurator, and the reads of the decision cache.

**Facts from the implementation of draft 3 that the new commits must keep:**

- The network configurator computes the error rate of a wireless link with a transmission outside the radio medium and its own WHOLE decision. The received packet thus reads its regions from the WHOLE decision that it receives, not from the cache (R8).
- `ErrorModelBase::computeCorruptedPacket()` must not enter the module of the error model: the radio medium deletes the received packet and warns about a packet that another module owns. The reception tests check that no such warning occurs.
- The error model draws from the random number generator of its module, so the test of `corruptUnits()` is a module test.
- The test of #1447 starts its interferer during the MAC header, after the PHY header. With an interferer over the PHY header, the whole frame fails with a packet-level bit error, and the MAC header chunk stays correct.
- The update tool of the fingerprint store did not leave out `examples/ipv6/mipv6 -c Handover`; restore its entries after an update.

Gaps of draft 3 that stay open: `check-cpp.sh` did not run, because `clang-tidy` is not installed on this machine. The PSDU bound of `Ieee80211Receiver` and the EIFS rule after a frame with incorrect content have no test. No step measured the speed; section 4.8 is an estimate.

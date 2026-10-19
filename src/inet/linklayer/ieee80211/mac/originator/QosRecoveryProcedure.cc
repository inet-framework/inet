//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/originator/QosRecoveryProcedure.h"

#include "inet/common/stlutils.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211DataSequenceTag_m.h"
#include "inet/linklayer/ieee80211/mac/contract/IRtsPolicy.h"

namespace inet {
namespace ieee80211 {

Define_Module(QosRecoveryProcedure);

QosRecoveryProcedure::MsduRetryRecord *QosRecoveryProcedure::findMsduRetryRecord(Packet *packet)
{
    auto tag = packet->findTag<Ieee80211DataSequenceTag>();
    if (!tag)
        return nullptr;
    auto it = msduRetryRecords.find(tag->getAllocationIndex());
    return it == msduRetryRecords.end() ? nullptr : &it->second;
}

void QosRecoveryProcedure::registerMsdu(Packet *packet, MsduRetryContract contract, bool lifetimeOnly)
{
    Enter_Method("registerMsdu");
    auto allocation = packet->getTag<Ieee80211DataSequenceTag>()->getAllocationIndex();
    auto result = msduRetryRecords.emplace(allocation, MsduRetryRecord{contract});
    ASSERT(result.second || result.first->second.contract == contract);
    result.first->second.lifetimeOnly = lifetimeOnly;
}

bool QosRecoveryProcedure::hasMsduRetryRecord(Packet *packet)
{
    return findMsduRetryRecord(packet) != nullptr;
}

void QosRecoveryProcedure::msduTransmissionFailed(Packet *packet, bool rts)
{
    Enter_Method("msduTransmissionFailed");
    auto record = findMsduRetryRecord(packet);
    ASSERT(record);
    if (record->contract == MsduRetryContract::IEEE80211_2024) {
        // IEEE Std 802.11-2024, 10.23.2.12.1: one count includes associated RTS and internal collisions.
        record->frameCount++;
        controlFrameTransmissionFailed();
    }
    else if (rts || packet->getByteLength() <= rtsThreshold) {
        // IEEE Std 802.11-2016, 10.22.2.11.1: the MSDU retains both counts across fragment changes.
        record->shortCount++;
        incrementStationSrc();
    }
    else {
        record->longCount++;
        incrementStationLrc();
    }
}

void QosRecoveryProcedure::msduAckReceived(Packet *packet, bool finalMember)
{
    Enter_Method("msduAckReceived");
    auto record = findMsduRetryRecord(packet);
    ASSERT(record);
    if (record->contract == MsduRetryContract::BASIC_2016) {
        if (packet->getByteLength() <= rtsThreshold) {
            record->shortCount = 0;
            resetStationSrc();
        }
        else {
            record->longCount = 0;
            resetStationLrc();
        }
    }
    else
        resetStationSrc();
    resetContentionWindow();
    if (finalMember)
        retireMsdu(packet);
}

bool QosRecoveryProcedure::isMsduRetryLimitReached(Packet *packet)
{
    auto record = findMsduRetryRecord(packet);
    ASSERT(record);
    // IEEE Std 802.11-2016, 10.24.3: the accepted Basic agreement limits the MSDU by lifetime.
    if (record->lifetimeOnly)
        return false;
    return record->contract == MsduRetryContract::IEEE80211_2024 ? record->frameCount >= shortRetryLimit :
        record->shortCount >= shortRetryLimit || record->longCount >= longRetryLimit;
}

void QosRecoveryProcedure::retireMsdu(Packet *packet)
{
    Enter_Method("retireMsdu");
    auto tag = packet->findTag<Ieee80211DataSequenceTag>();
    if (tag)
        msduRetryRecords.erase(tag->getAllocationIndex());
}

inline std::ostream& operator<<(std::ostream& os, const std::pair<Tid, SequenceControlField>& p) { return os << p.first << "#" << p.second; }

//
// Contention window management
// ============================
//
// The CW shall take the next value in the series every time an
// unsuccessful attempt to transmit an MPDU causes either STA retry
// counter to increment, until the CW reaches the value of aCWmax.
//
// The CW shall be reset to aCWmin after [...] when SLRC reaches
// dot11LongRetryLimit, or when SSRC reaches dot11ShortRetryLimit.
//
void QosRecoveryProcedure::initialize(int stage)
{
    if (stage == INITSTAGE_LAST) {
        auto rtsPolicy = check_and_cast<IRtsPolicy *>(getModuleByPath(par("rtsPolicyModule")));
        cwCalculator = check_and_cast<ICwCalculator *>(getModuleByPath(par("cwCalculatorModule")));
        rtsThreshold = rtsPolicy->getRtsThreshold();
        shortRetryLimit = par("shortRetryLimit");
        longRetryLimit = par("longRetryLimit");
        emit(contentionWindowChangedSignal, cwCalculator->getCw());
        WATCH(rtsThreshold);
        WATCH(stationLongRetryCounter);
        WATCH(stationShortRetryCounter);
        WATCH(shortRetryCounter);
        WATCH(longRetryCounter);
    }
}

void QosRecoveryProcedure::incrementStationSrc()
{
    stationShortRetryCounter++;
    EV_INFO << "Incremented station SRC: stationShortRetryCounter = " << stationShortRetryCounter << ".\n";
    if (stationShortRetryCounter == shortRetryLimit) // 9.3.3 Random backoff time
        resetContentionWindow();
    else
        incrementContentionWindow();
}

void QosRecoveryProcedure::incrementStationLrc()
{
    stationLongRetryCounter++;
    EV_INFO << "Incremented station LRC: stationLongRetryCounter = " << stationLongRetryCounter << ".\n";
    if (stationLongRetryCounter == longRetryLimit) // 9.3.3 Random backoff time
        resetContentionWindow();
    else
        incrementContentionWindow();
}

QosRecoveryProcedure::RetryKey QosRecoveryProcedure::getRetryKey(const Ptr<const Ieee80211DataHeader>& header) const
{
    // IEEE Std 802.11-2024, 10.23.2.12.1: disposal resolves only this DATA's retry state.
    return {header->getReceiverAddress(), {header->getTid(),
        SequenceControlField(header->getSequenceNumber().get(), header->getFragmentNumber())}};
}

void QosRecoveryProcedure::incrementCounter(const Ptr<const Ieee80211DataHeader>& header, std::map<RetryKey, int>& retryCounter)
{
    auto id = getRetryKey(header);
    if (containsKey(retryCounter, id))
        retryCounter[id]++;
    else
        retryCounter[id] = 1;
}

//
// The SSRC shall be reset to 0 [...] when a frame with a group address in the
// Address1 field is transmitted. The SLRC shall be reset to 0 when [...] a
// frame with a group address in the Address1 field is transmitted.
//
void QosRecoveryProcedure::multicastFrameTransmitted()
{
    resetStationLrc();
    resetStationSrc();
}

//
// The SSRC shall be reset to 0 when a CTS frame is received in response to an RTS
// frame, when a BlockAck frame is received in response to a BlockAckReq frame, when
// an ACK frame is received in response to the transmission of a frame of length greater*
// than dot11RTSThreshold containing all or part of an MSDU or MMPDU, [...]
// The SLRC shall be reset to 0 when an ACK frame is received in response to transmission
// of a frame containing all or part of an MSDU or MMPDU of [...]
//
// Note: * This is obviously wrong.
//
void QosRecoveryProcedure::ctsFrameReceived()
{
    resetStationSrc();
}

void QosRecoveryProcedure::blockAckFrameReceived()
{
    resetStationSrc();
    resetContentionWindow();
}

void QosRecoveryProcedure::controlFrameTransmissionFailed()
{
    // IEEE Std 802.11-2024, 10.23.2.2: control failures update EDCA backoff without DATA retry feedback.
    if (stationShortRetryCounter < shortRetryLimit) {
        stationShortRetryCounter++;
        incrementContentionWindow();
    }
    else {
        resetStationSrc();
        resetContentionWindow();
    }
}

void QosRecoveryProcedure::implicitBlockAckMemberFailed(Packet *packet, const Ptr<const Ieee80211DataHeader>& header)
{
    incrementCounter(header, packet->getByteLength() >= rtsThreshold ? longRetryCounter : shortRetryCounter);
}

//
// This SRC and the SSRC shall be reset when a MAC frame of length less than or equal
// to dot11RTSThreshold succeeds for that MPDU of type Data or MMPDU.

// This LRC and the SLRC shall be reset when a MAC frame of length greater than dot11RTSThreshold
// succeeds for that MPDU of type Data or MMPDU.
//
void QosRecoveryProcedure::ackFrameReceived(Packet *packet, const Ptr<const Ieee80211DataHeader>& ackedHeader)
{
    if (findMsduRetryRecord(packet)) {
        msduAckReceived(packet, !ackedHeader->getMoreFragments());
        return;
    }
    auto id = getRetryKey(ackedHeader);
    if (packet->getByteLength() >= rtsThreshold) {
        resetStationLrc();
        auto it = longRetryCounter.find(id);
        if (it != longRetryCounter.end())
            longRetryCounter.erase(it);
    }
    else {
        auto it = shortRetryCounter.find(id);
        resetStationSrc();
        if (it != shortRetryCounter.end())
            shortRetryCounter.erase(it);
    }
    //
    // The CW shall be reset to aCWmin after every successful attempt to transmit a frame containing
    // all or part of an MSDU or MMPDU
    //
    resetContentionWindow();
}

//
// After dropping a frame because it reached its retry limit we need to clear the
// retry counters
//
void QosRecoveryProcedure::retryLimitReached(Packet *packet, const Ptr<const Ieee80211DataHeader>& header)
{
    EV_WARN << "Retry limit reached for " << *packet << ".\n";
    dropFrame(header);
    retireMsdu(packet);
    emit(retryLimitReachedSignal, packet);
}

void QosRecoveryProcedure::dropFrame(const Ptr<const Ieee80211DataHeader>& header)
{
    auto id = getRetryKey(header);
    // RTS failure can leave an SRC entry even when the protected DATA uses an LRC entry.
    shortRetryCounter.erase(id);
    longRetryCounter.erase(id);
}

//
// The SRC for an MPDU of type Data or MMPDU and the SSRC shall be incremented every
// time transmission of a MAC frame of length less than or equal to dot11RTSThreshold
// fails for that MPDU of type Data or MMPDU.

// The LRC for an MPDU of type Data or MMPDU and the SLRC shall be incremented every time
// transmission of a MAC frame of length greater than dot11RTSThreshold fails for that MPDU
// of type Data or MMPDU.
//
void QosRecoveryProcedure::dataFrameTransmissionFailed(Packet *packet, const Ptr<const Ieee80211DataHeader>& failedHeader)
{
    if (findMsduRetryRecord(packet)) {
        msduTransmissionFailed(packet, false);
        return;
    }
    if (packet->getByteLength() >= rtsThreshold) {
        incrementStationLrc();
        incrementCounter(failedHeader, longRetryCounter);
    }
    else {
        incrementStationSrc();
        incrementCounter(failedHeader, shortRetryCounter);
    }
}

//
// If the RTS transmission fails, the SRC for the MSDU or MMPDU and the SSRC are incremented.
//
void QosRecoveryProcedure::rtsFrameTransmissionFailed(const Ptr<const Ieee80211DataHeader>& protectedHeader)
{
    incrementStationSrc();
    incrementCounter(protectedHeader, shortRetryCounter);
}

//
// Retries for failed transmission attempts shall continue until the SRC for the MPDU of type
// Data or MMPDU is equal to dot11ShortRetryLimit or until the LRC for the MPDU of type Data
// or MMPDU is equal to dot11LongRetryLimit. When either of these limits is reached, retry attempts
// shall cease, and the MPDU of type Data (and any MSDU of which it is a part) or MMPDU shall be discarded.
//
bool QosRecoveryProcedure::isRetryLimitReached(Packet *packet, const Ptr<const Ieee80211DataHeader>& failedHeader)
{
    if (findMsduRetryRecord(packet))
        return isMsduRetryLimitReached(packet);
    if (packet->getByteLength() >= rtsThreshold)
        return getRc(packet, failedHeader, longRetryCounter) >= longRetryLimit;
    else
        return getRc(packet, failedHeader, shortRetryCounter) >= shortRetryLimit;
}

int QosRecoveryProcedure::getRetryCount(Packet *packet, const Ptr<const Ieee80211DataHeader>& header)
{
    if (auto record = findMsduRetryRecord(packet))
        return record->contract == MsduRetryContract::IEEE80211_2024 ? record->frameCount :
            packet->getByteLength() > rtsThreshold ? record->longCount : record->shortCount;
    if (packet->getByteLength() >= rtsThreshold)
        return getRc(packet, header, longRetryCounter);
    else
        return getRc(packet, header, shortRetryCounter);
}

void QosRecoveryProcedure::incrementContentionWindow()
{
    auto oldCw = cwCalculator->getCw();
    cwCalculator->incrementCw();
    auto newCw = cwCalculator->getCw();
    if (oldCw != newCw)
        emit(contentionWindowChangedSignal, cwCalculator->getCw());
}

void QosRecoveryProcedure::resetContentionWindow()
{
    auto oldCw = cwCalculator->getCw();
    cwCalculator->resetCw();
    auto newCw = cwCalculator->getCw();
    if (oldCw != newCw)
        emit(contentionWindowChangedSignal, newCw);
}

bool QosRecoveryProcedure::isRtsFrameRetryLimitReached(Packet *packet, const Ptr<const Ieee80211DataHeader>& protectedHeader)
{
    if (findMsduRetryRecord(packet))
        return isMsduRetryLimitReached(packet);
    return getRc(packet, protectedHeader, shortRetryCounter) >= shortRetryLimit;
}

int QosRecoveryProcedure::getRc(Packet *packet, const Ptr<const Ieee80211DataHeader>& header, std::map<RetryKey, int>& retryCounter)
{
    auto id = getRetryKey(header);
    auto it = retryCounter.find(id);
    if (it != retryCounter.end())
        return it->second;
    else
        throw cRuntimeError("The retry counter entry doesn't exist for message id: %" PRId64, packet->getId());
}

bool QosRecoveryProcedure::isMulticastFrame(Packet *packet, const Ptr<const Ieee80211MacHeader>& header)
{
    if (auto oneAddressHeader = dynamicPtrCast<const Ieee80211OneAddressHeader>(header))
        return oneAddressHeader->getReceiverAddress().isMulticast();
    return false;
}

} /* namespace ieee80211 */
} /* namespace inet */


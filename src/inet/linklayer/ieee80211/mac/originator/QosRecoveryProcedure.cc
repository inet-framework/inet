//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/originator/QosRecoveryProcedure.h"

#include "inet/common/stlutils.h"
#include "inet/linklayer/ieee80211/mac/contract/IRtsPolicy.h"

namespace inet {
namespace ieee80211 {

void QosRecoveryProcedure::beginBlockAckRecovery(MacAddress peer, Tid tid, const std::vector<Packet *>& originals)
{
    Enter_Method("beginBlockAckRecovery");
    auto& recovery = blockAckRecoveries[{peer, tid}];
    for (auto original : originals)
        recovery.covered.emplace(original->getId(), getRetryKey(original->peekAtFront<Ieee80211DataHeader>()));
}

void QosRecoveryProcedure::cancelBlockAckRecovery(MacAddress peer, Tid tid)
{
    Enter_Method("cancelBlockAckRecovery");
    blockAckRecoveries.erase({peer, tid});
}

std::set<int64_t> QosRecoveryProcedure::finishBlockAckRecovery(MacAddress peer, Tid tid)
{
    Enter_Method("finishBlockAckRecovery");
    std::set<int64_t> covered;
    auto it = blockAckRecoveries.find({peer,tid});
    if (it != blockAckRecoveries.end()) {
        for (const auto& entry : it->second.covered) covered.insert(entry.first);
        blockAckRecoveries.erase(it);
    }
    return covered;
}

bool QosRecoveryProcedure::hasBlockAckRecovery(MacAddress peer, Tid tid) const
{
    return blockAckRecoveries.count({peer, tid}) != 0;
}

std::string QosRecoveryProcedure::describeBlockAckRecoveries() const
{
    std::ostringstream out;
    for (const auto& entry : blockAckRecoveries) {
        out << entry.first.first << "/TID=" << int(entry.first.second) << " expirations=" << entry.second.failures << " covered=";
        for (const auto& covered : entry.second.covered) out << covered.first << " ";
        out << "\n";
    }
    return out.str();
}

std::set<int64_t> QosRecoveryProcedure::blockAckRecoveryExpired(MacAddress peer, Tid tid, const std::set<int64_t>& live)
{
    Enter_Method("blockAckRecoveryExpired");
    auto it = blockAckRecoveries.find({peer, tid});
    if (it == blockAckRecoveries.end())
        return {};
    auto& recovery = it->second;
    for (auto covered = recovery.covered.begin(); covered != recovery.covered.end();)
        if (!live.count(covered->first)) covered = recovery.covered.erase(covered); else ++covered;
    if (recovery.covered.empty()) {
        blockAckRecoveries.erase(it);
        return {};
    }
    if (++recovery.failures < shortRetryLimit)
        return {};
    std::set<int64_t> result;
    for (const auto& covered : recovery.covered) result.insert(covered.first);
    blockAckRecoveries.erase(it);
    return result;
}

Define_Module(QosRecoveryProcedure);

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
        WATCH_EXPR("blockAckRecoveries", describeBlockAckRecoveries());
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

//
// This SRC and the SSRC shall be reset when a MAC frame of length less than or equal
// to dot11RTSThreshold succeeds for that MPDU of type Data or MMPDU.

// This LRC and the SLRC shall be reset when a MAC frame of length greater than dot11RTSThreshold
// succeeds for that MPDU of type Data or MMPDU.
//
void QosRecoveryProcedure::ackFrameReceived(Packet *packet, const Ptr<const Ieee80211DataHeader>& ackedHeader)
{
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
    emit(retryLimitReachedSignal, packet);
}

void QosRecoveryProcedure::dropFrame(const Ptr<const Ieee80211DataHeader>& header)
{
    auto id = getRetryKey(header);
    auto procedure = blockAckRecoveries.find({header->getReceiverAddress(), header->getTid()});
    if (procedure != blockAckRecoveries.end()) {
        for (auto it = procedure->second.covered.begin(); it != procedure->second.covered.end();)
            if (!(it->second < id) && !(id < it->second)) it = procedure->second.covered.erase(it); else ++it;
        if (procedure->second.covered.empty()) blockAckRecoveries.erase(procedure);
    }
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
    if (packet->getByteLength() >= rtsThreshold) {
        incrementStationLrc();
        incrementCounter(failedHeader, longRetryCounter);
    }
    else {
        incrementStationSrc();
        incrementCounter(failedHeader, shortRetryCounter);
    }
}

void QosRecoveryProcedure::ampduMemberTransmissionFailed(Packet *packet, const Ptr<const Ieee80211DataHeader>& header)
{
    // Per-MPDU retry state is independent of the one PPDU contention outcome.
    incrementCounter(header, packet->getByteLength() >= rtsThreshold ? longRetryCounter : shortRetryCounter);
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
    if (packet->getByteLength() >= rtsThreshold)
        return getRc(packet, failedHeader, longRetryCounter) >= longRetryLimit;
    else
        return getRc(packet, failedHeader, shortRetryCounter) >= shortRetryLimit;
}

int QosRecoveryProcedure::getRetryCount(Packet *packet, const Ptr<const Ieee80211DataHeader>& header)
{
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


//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// Contributor: Miguel González-López (per-receiver retry accounting and Block Ack recovery)
//


#include "inet/linklayer/ieee80211/mac/originator/QosRecoveryProcedure.h"

#include "inet/common/stlutils.h"
#include "inet/linklayer/ieee80211/mac/contract/IRtsPolicy.h"

namespace inet {
namespace ieee80211 {

Define_Module(QosRecoveryProcedure);

inline std::ostream& operator<<(std::ostream& os, const std::pair<Tid, SequenceControlField>& p) { return os << p.first << "#" << p.second; }

void QosRecoveryProcedure::initialize(int stage)
{
    if (stage == INITSTAGE_LAST) {
        auto rtsPolicy = check_and_cast<IRtsPolicy *>(getModuleByPath(par("rtsPolicyModule")));
        cwCalculator = check_and_cast<IEdcaCwCalculator *>(getModuleByPath(par("cwCalculatorModule")));
        rtsThreshold = rtsPolicy->getRtsThreshold();
        shortRetryLimit = par("shortRetryLimit");
        longRetryLimit = par("longRetryLimit");
        WATCH(rtsThreshold);
        WATCH(shortRetryCounter);
        WATCH(longRetryCounter);
    }
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

void QosRecoveryProcedure::txopCompleted()
{
    cwCalculator->resetCw();
}

void QosRecoveryProcedure::edcaTransmissionFailed()
{
    cwCalculator->incrementCw();
}

//
// This SRC shall be reset when a MAC frame of length less than or equal
// to dot11RTSThreshold succeeds for that MPDU of type Data or MMPDU.

// This LRC shall be reset when a MAC frame of length greater than dot11RTSThreshold
// succeeds for that MPDU of type Data or MMPDU.
//
void QosRecoveryProcedure::ackFrameReceived(Packet *packet, const Ptr<const Ieee80211DataHeader>& ackedHeader)
{
    auto id = getRetryKey(ackedHeader);
    auto& retries = packet->getByteLength() >= rtsThreshold ? longRetryCounter : shortRetryCounter;
    retries.erase(id);
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
    // RTS failure can leave an SRC entry even when the protected DATA uses an LRC entry.
    shortRetryCounter.erase(id);
    longRetryCounter.erase(id);
}

//
// The SRC for an MPDU of type Data or MMPDU shall be incremented every
// time transmission of a MAC frame of length less than or equal to dot11RTSThreshold
// fails for that MPDU of type Data or MMPDU.

// The LRC for an MPDU of type Data or MMPDU shall be incremented every time
// transmission of a MAC frame of length greater than dot11RTSThreshold fails for that MPDU
// of type Data or MMPDU.
//
void QosRecoveryProcedure::dataFrameTransmissionFailed(Packet *packet, const Ptr<const Ieee80211DataHeader>& failedHeader)
{
    edcaTransmissionFailed();
    auto& retries = packet->getByteLength() >= rtsThreshold ? longRetryCounter : shortRetryCounter;
    incrementCounter(failedHeader, retries);
}

//
// If RTS transmission fails, also charge the protected DATA's short retry count.
//
void QosRecoveryProcedure::rtsFrameTransmissionFailed(const Ptr<const Ieee80211DataHeader>& protectedHeader)
{
    edcaTransmissionFailed();
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


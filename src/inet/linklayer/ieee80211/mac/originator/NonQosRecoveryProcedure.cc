//
// Copyright (C) 2016 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see http://www.gnu.org/licenses/.
//

#include "inet/linklayer/ieee80211/mac/contract/IRtsPolicy.h"
#include "inet/linklayer/ieee80211/mac/originator/NonQosRecoveryProcedure.h"

namespace inet {
namespace ieee80211 {

Define_Module(NonQosRecoveryProcedure);

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
void NonQosRecoveryProcedure::initialize(int stage)
{
    if (stage == INITSTAGE_LAST) {
        auto rtsPolicy = check_and_cast<IRtsPolicy *>(getModuleByPath(par("rtsPolicyModule")));
        cwCalculator = check_and_cast<ICwCalculator *>(getModuleByPath(par("cwCalculatorModule")));
        rtsThreshold = rtsPolicy->getRtsThreshold();
        shortRetryLimit = par("shortRetryLimit");
        longRetryLimit = par("longRetryLimit");
        emit(contentionWindowChangedSignal, cwCalculator->getCw());
        WATCH_MAP(shortRetryCounter);
        WATCH_MAP(longRetryCounter);
    }
}

void NonQosRecoveryProcedure::incrementStationSrc(StationRetryCounters *stationCounters)
{
    stationCounters->incrementStationShortRetryCount();
    EV_INFO << "Incremented station SRC: stationShortRetryCounter = " << stationCounters->getStationShortRetryCount() << ".\n";
    if (stationCounters->getStationShortRetryCount() == shortRetryLimit) // 9.3.3 Random backoff time
        resetContentionWindow();
    else
        incrementContentionWindow();
}

void NonQosRecoveryProcedure::incrementStationLrc(StationRetryCounters *stationCounters)
{
    stationCounters->incrementStationLongRetryCount();
    EV_INFO << "Incremented station LRC: stationLongRetryCounter = " << stationCounters->getStationLongRetryCount() << ".\n";
    if (stationCounters->getStationLongRetryCount() == longRetryLimit) // 9.3.3 Random backoff time
        resetContentionWindow();
    else
        incrementContentionWindow();
}

void NonQosRecoveryProcedure::incrementCounter(const Ptr<const Ieee80211DataOrMgmtHeader>& header, std::map<SequenceControlField, int>& retryCounter)
{
    auto id = SequenceControlField(header->getSequenceNumber().get(), header->getFragmentNumber());
    if (retryCounter.find(id) != retryCounter.end())
        retryCounter[id]++;
    else
        retryCounter.insert(std::make_pair(id, 1));
}

//
// The SSRC shall be reset to 0 [...] when a frame with a group address in the
// Address1 field is transmitted. The SLRC shall be reset to 0 when [...] a
// frame with a group address in the Address1 field is transmitted.
//
void NonQosRecoveryProcedure::multicastFrameTransmitted(StationRetryCounters *stationCounters)
{
    stationCounters->resetStationShortRetryCount();
    stationCounters->resetStationLongRetryCount();
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
void NonQosRecoveryProcedure::ctsFrameReceived(StationRetryCounters *stationCounters)
{
    stationCounters->resetStationShortRetryCount();
}

//
// This SRC and the SSRC shall be reset when a MAC frame of length less than or equal
// to dot11RTSThreshold succeeds for that MPDU of type Data or MMPDU.

// This LRC and the SLRC shall be reset when a MAC frame of length greater than dot11RTSThreshold
// succeeds for that MPDU of type Data or MMPDU.
//
void NonQosRecoveryProcedure::ackFrameReceived(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& ackedHeader, StationRetryCounters *stationCounters)
{
    auto id = SequenceControlField(ackedHeader->getSequenceNumber().get(), ackedHeader->getFragmentNumber());
    if (packet->getByteLength() >= rtsThreshold) {
        stationCounters->resetStationLongRetryCount();
        auto it = longRetryCounter.find(id);
        if (it != longRetryCounter.end())
            longRetryCounter.erase(it);
    }
    else {
        stationCounters->resetStationShortRetryCount();
        auto it = shortRetryCounter.find(id);
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
void NonQosRecoveryProcedure::retryLimitReached(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& header)
{
    EV_WARN << "Retry limit reached for " << *packet << ".\n";
    auto id = SequenceControlField(header->getSequenceNumber().get(), header->getFragmentNumber());
    if (packet->getByteLength() >= rtsThreshold) {
        auto it = longRetryCounter.find(id);
        if (it != longRetryCounter.end())
            longRetryCounter.erase(it);
    }
    else {
        auto it = shortRetryCounter.find(id);
        if (it != shortRetryCounter.end())
            shortRetryCounter.erase(it);
    }
    emit(retryLimitReachedSignal, packet);
}

//
// The SRC for an MPDU of type Data or MMPDU and the SSRC shall be incremented every
// time transmission of a MAC frame of length less than or equal to dot11RTSThreshold
// fails for that MPDU of type Data or MMPDU.

// The LRC for an MPDU of type Data or MMPDU and the SLRC shall be incremented every time
// transmission of a MAC frame of length greater than dot11RTSThreshold fails for that MPDU
// of type Data or MMPDU.
//
void NonQosRecoveryProcedure::dataOrMgmtFrameTransmissionFailed(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& failedHeader, StationRetryCounters *stationCounters)
{
    if (packet->getByteLength() >= rtsThreshold) {
        incrementStationLrc(stationCounters);
        incrementCounter(failedHeader, longRetryCounter);
    }
    else {
        incrementStationSrc(stationCounters);
        incrementCounter(failedHeader, shortRetryCounter);
    }
}

//
// If the RTS transmission fails, the SRC for the MSDU or MMPDU and the SSRC are incremented.
//
void NonQosRecoveryProcedure::rtsFrameTransmissionFailed(const Ptr<const Ieee80211DataOrMgmtHeader>& protectedHeader, StationRetryCounters *stationCounters)
{
    incrementStationSrc(stationCounters);
    incrementCounter(protectedHeader, shortRetryCounter);
}

//
// Retries for failed transmission attempts shall continue until the SRC for the MPDU of type
// Data or MMPDU is equal to dot11ShortRetryLimit or until the LRC for the MPDU of type Data
// or MMPDU is equal to dot11LongRetryLimit. When either of these limits is reached, retry attempts
// shall cease, and the MPDU of type Data (and any MSDU of which it is a part) or MMPDU shall be discarded.
//
bool NonQosRecoveryProcedure::isRetryLimitReached(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& failedHeader)
{
    if (packet->getByteLength() >= rtsThreshold)
        return getRc(packet, failedHeader, longRetryCounter) >= longRetryLimit;
    else
        return getRc(packet, failedHeader, shortRetryCounter) >= shortRetryLimit;
}


int NonQosRecoveryProcedure::getRetryCount(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& header)
{
    if (packet->getByteLength() >= rtsThreshold)
        return getRc(packet, header, longRetryCounter);
    else
        return getRc(packet, header, shortRetryCounter);
}


int NonQosRecoveryProcedure::getShortRetryCount(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& dataOrMgmtHeader)
{
    return getRc(packet, dataOrMgmtHeader, shortRetryCounter);
}

int NonQosRecoveryProcedure::getLongRetryCount(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& dataOrMgmtHeader)
{
    return getRc(packet, dataOrMgmtHeader, longRetryCounter);
}

void NonQosRecoveryProcedure::incrementContentionWindow()
{
    auto oldCw = cwCalculator->getCw();
    cwCalculator->incrementCw();
    auto newCw = cwCalculator->getCw();
    if (oldCw != newCw)
        emit(contentionWindowChangedSignal, newCw);
}

void NonQosRecoveryProcedure::resetContentionWindow()
{
    auto oldCw = cwCalculator->getCw();
    cwCalculator->resetCw();
    auto newCw = cwCalculator->getCw();
    if (oldCw != newCw)
        emit(contentionWindowChangedSignal, newCw);
}

bool NonQosRecoveryProcedure::isRtsFrameRetryLimitReached(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& protectedHeader)
{
    return getRc(packet, protectedHeader, shortRetryCounter) >= shortRetryLimit;
}

int NonQosRecoveryProcedure::getRc(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& header, std::map<SequenceControlField, int>& retryCounter)
{
    auto count = retryCounter.find(SequenceControlField(header->getSequenceNumber().get(), header->getFragmentNumber()));
    if (count != retryCounter.end())
        return count->second;
    else
        return 0;
}

bool NonQosRecoveryProcedure::isMulticastFrame(const Ptr<const Ieee80211MacHeader>& header)
{
    if (auto oneAddressHeader = dynamicPtrCast<const Ieee80211OneAddressHeader>(header))
        return oneAddressHeader->getReceiverAddress().isMulticast();
    return false;
}

} /* namespace ieee80211 */
} /* namespace inet */

//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/common/ResultFilters.h"

#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {

namespace ieee80211 {

Register_ResultFilter("ieee80211Unicast", Ieee80211UnicastFilter);

void Ieee80211UnicastFilter::receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details)
{
    if (auto packet = dynamic_cast<Packet *>(object)) {
        const auto& header = packet->peekAtFront<Ieee80211MacHeader>();
        if (header != nullptr) {
            const auto& address = header->getReceiverAddress();
            if (!address.isMulticast() && !address.isBroadcast())
                fire(prev, t, object, details);
        }
    }
}

Register_ResultFilter("ieee80211Multicast", Ieee80211MulticastFilter);

void Ieee80211MulticastFilter::receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details)
{
    if (auto packet = dynamic_cast<Packet *>(object)) {
        const auto& header = packet->peekAtFront<Ieee80211MacHeader>();
        if (header != nullptr && header->getReceiverAddress().isMulticast())
            fire(prev, t, object, details);
    }
}

Register_ResultFilter("ieee80211Broadcast", Ieee80211BroadcastFilter);

void Ieee80211BroadcastFilter::receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details)
{
    if (auto packet = dynamic_cast<Packet *>(object)) {
        const auto& header = packet->peekAtFront<Ieee80211MacHeader>();
        if (header != nullptr && header->getReceiverAddress().isBroadcast())
            fire(prev, t, object, details);
    }
}

Register_ResultFilter("ieee80211Retry", Ieee80211RetryFilter);

void Ieee80211RetryFilter::receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details)
{
    if (auto packet = dynamic_cast<Packet *>(object)) {
        const auto& header = packet->peekAtFront<Ieee80211MacHeader>();
        if (header != nullptr && header->getRetry())
            fire(prev, t, object, details);
    }
}

Register_ResultFilter("ieee80211NotRetry", Ieee80211NotRetryFilter);

void Ieee80211NotRetryFilter::receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details)
{
    if (auto packet = dynamic_cast<Packet *>(object)) {
        const auto& header = packet->peekAtFront<Ieee80211MacHeader>();
        if (header != nullptr && !header->getRetry())
            fire(prev, t, object, details);
    }
}

void FrameTransmissionStatusFilter::receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details)
{
    auto transDetails = dynamic_cast<FrameTransmissionDetails *>(details);
    if (transDetails == nullptr)
        transDetails = dynamic_cast<FrameTransmissionDetails *>(object);
    if (transDetails != nullptr && transDetails->getStatus() == status) {
        cObject *forwardObj = object != nullptr ? object : transDetails;
        fire(this, t, forwardObj, details);
    }
}

class FrameTransmissionStatusIsAcknowledgedFilter : public FrameTransmissionStatusFilter { public: FrameTransmissionStatusIsAcknowledgedFilter() : FrameTransmissionStatusFilter(FRAME_TRANSMISSION_STATUS_ACKNOWLEDGED) {} };
Register_ResultFilter("frameTransmissionStatusIsAcknowledged", FrameTransmissionStatusIsAcknowledgedFilter);

class FrameTransmissionStatusIsRetryLimitReachedFilter : public FrameTransmissionStatusFilter { public: FrameTransmissionStatusIsRetryLimitReachedFilter() : FrameTransmissionStatusFilter(FRAME_TRANSMISSION_STATUS_RETRY_LIMIT_REACHED) {} };
Register_ResultFilter("frameTransmissionStatusIsRetryLimitReached", FrameTransmissionStatusIsRetryLimitReachedFilter);

class FrameTransmissionStatusIsDroppedBeforeTransmissionFilter : public FrameTransmissionStatusFilter { public: FrameTransmissionStatusIsDroppedBeforeTransmissionFilter() : FrameTransmissionStatusFilter(FRAME_TRANSMISSION_STATUS_DROPPED_BEFORE_TRANSMISSION) {} };
Register_ResultFilter("frameTransmissionStatusIsDroppedBeforeTransmission", FrameTransmissionStatusIsDroppedBeforeTransmissionFilter);

} // namespace ieee80211

} // namespace inet


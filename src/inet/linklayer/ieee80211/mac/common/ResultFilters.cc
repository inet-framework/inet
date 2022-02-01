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

} // namespace ieee80211

} // namespace inet


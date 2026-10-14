//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/lifetime/EdcaTransmitLifetimeHandler.h"

namespace inet {
namespace ieee80211 {

EdcaTransmitLifetimeHandler::EdcaTransmitLifetimeHandler(simtime_t bkLifetime, simtime_t beLifetime, simtime_t viLifetime, simtime_t voLifetime)
{
    msduLifetime[AC_BK] = bkLifetime;
    msduLifetime[AC_BE] = beLifetime;
    msduLifetime[AC_VI] = viLifetime;
    msduLifetime[AC_VO] = voLifetime;
}

void EdcaTransmitLifetimeHandler::setDeadlineChangedCallback(std::function<void()> callback)
{
    deadlineChanged = callback;
}

void EdcaTransmitLifetimeHandler::addDeadline(Packet *packet, simtime_t deadline)
{
    auto packetId = packet->getId();
    auto inserted = deadlines.emplace(deadline, packetId);
    if (!inserted.second || !frames.emplace(packetId, FrameLifetime{inserted.first, packet}).second)
        throw cRuntimeError("Duplicate DATA lifetime identity");
}

bool EdcaTransmitLifetimeHandler::eraseDeadline(int64_t packetId)
{
    auto it = frames.find(packetId);
    if (it == frames.end())
        return false;
    deadlines.erase(it->second.deadline);
    frames.erase(it);
    return true;
}

void EdcaTransmitLifetimeHandler::registerFrame(Packet *packet, AccessCategory ac)
{
    // IEEE Std 802.11-2024, 10.23.2.12.1: start at MAC acceptance or the latest A-MSDU addition.
    addDeadline(packet, simTime() + msduLifetime[ac]);
    if (deadlineChanged)
        deadlineChanged();
}

void EdcaTransmitLifetimeHandler::forgetFrame(int64_t packetId)
{
    if (eraseDeadline(packetId) && deadlineChanged)
        deadlineChanged();
}

void EdcaTransmitLifetimeHandler::prepareFrameReplacement(const std::vector<int64_t>& sourceIds)
{
    for (auto id : sourceIds) {
        if (!eraseDeadline(id))
            throw cRuntimeError("Frame replacement source has no DATA lifetime entry");
    }
    // Retain timer order until the synchronous builder registers its replacement.
}

void EdcaTransmitLifetimeHandler::registerFragments(const std::vector<Packet *>& fragments, simtime_t deadline)
{
    // IEEE Std 802.11-2024, 10.23.2.12.1: undelivered fragments retain the MSDU timer.
    for (auto fragment : fragments)
        addDeadline(fragment, deadline);
    if (deadlineChanged)
        deadlineChanged();
}

simtime_t EdcaTransmitLifetimeHandler::getDeadline(const Packet *packet) const
{
    auto it = frames.find(packet->getId());
    return it == frames.end() ? SIMTIME_MAX : it->second.deadline->first;
}

bool EdcaTransmitLifetimeHandler::isExpired(const Packet *packet, simtime_t time) const
{
    return getDeadline(packet) <= time;
}

simtime_t EdcaTransmitLifetimeHandler::getEarliestDeadline(const std::set<int64_t>& referencedFrames) const
{
    for (const auto& entry : deadlines) {
        // An accepted exchange can retain expired DATA until its safe boundary.
        if (entry.first > simTime() || referencedFrames.count(entry.second) == 0)
            return entry.first;
    }
    return SIMTIME_MAX;
}

std::vector<int64_t> EdcaTransmitLifetimeHandler::getExpiredFrameIds(const std::set<int64_t>& referencedFrames) const
{
    std::vector<int64_t> expired;
    for (const auto& entry : deadlines) {
        if (entry.first > simTime())
            break;
        if (referencedFrames.count(entry.second) == 0)
            expired.push_back(entry.second);
    }
    return expired;
}

Packet *EdcaTransmitLifetimeHandler::findFrame(int64_t packetId) const
{
    auto it = frames.find(packetId);
    return it == frames.end() ? nullptr : it->second.packet;
}

} /* namespace ieee80211 */
} /* namespace inet */

